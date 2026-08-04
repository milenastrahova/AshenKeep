#include "Misc/AutomationTest.h"

#include "AshenAttributeComponent.h"
#include "AshenBloodBurstComponent.h"
#include "AshenDamageVolume.h"
#include "AshenEnemyAIController.h"
#include "AshenLockOnComponent.h"
#include "AshenPlayerCharacter.h"
#include "AshenPurgeRitualObjective.h"
#include "AshenTargetingMathLibrary.h"
#include "AshenTrainingEnemy.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace AshenAutomation
{
	bool IsNearlyEqual(
		float A,
		float B,
		float Tolerance = KINDA_SMALL_NUMBER
	)
	{
		return FMath::IsNearlyEqual(
			A,
			B,
			Tolerance
		);
	}

	bool HasFunctionFlags(
		const UClass* Class,
		const FName FunctionName,
		EFunctionFlags RequiredFlags
	)
	{
		if (!Class)
		{
			return false;
		}

		const UFunction* Function =
			Class->FindFunctionByName(
				FunctionName
			);

		return Function &&
			Function->HasAllFunctionFlags(
				RequiredFlags
			);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAshenAttributeDefaultsTest,
	"AshenKeep.Attributes.DefaultValues",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter
)

bool FAshenAttributeDefaultsTest::RunTest(
	const FString& Parameters
)
{
	const UAshenAttributeComponent* Attributes =
		GetDefault<UAshenAttributeComponent>();

	TestNotNull(
		TEXT("Attribute component CDO exists"),
		Attributes
	);

	if (!Attributes)
	{
		return false;
	}

	TestTrue(
		TEXT("Attribute component replication is enabled"),
		Attributes->GetIsReplicated()
	);

	TestTrue(
		TEXT("Attribute component does not tick"),
		!Attributes->
			PrimaryComponentTick.bCanEverTick
	);

	TestTrue(
		TEXT("Maximum health is positive"),
		Attributes->GetMaxHealth() > 0.0f
	);

	TestTrue(
		TEXT("Maximum stamina is positive"),
		Attributes->GetMaxStamina() > 0.0f
	);

	TestTrue(
		TEXT("Maximum Blood Essence is positive"),
		Attributes->GetMaxMana() > 0.0f
	);

	TestTrue(
		TEXT("Health starts inside the valid range"),
		Attributes->GetHealth() >= 0.0f &&
		Attributes->GetHealth() <=
			Attributes->GetMaxHealth()
	);

	TestTrue(
		TEXT("Stamina starts inside the valid range"),
		Attributes->GetStamina() >= 0.0f &&
		Attributes->GetStamina() <=
			Attributes->GetMaxStamina()
	);

	TestTrue(
		TEXT("Blood Essence starts inside the valid range"),
		Attributes->GetMana() >= 0.0f &&
		Attributes->GetMana() <=
			Attributes->GetMaxMana()
	);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAshenAttributeMathTest,
	"AshenKeep.Attributes.DeterministicMath",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter
)

bool FAshenAttributeMathTest::RunTest(
	const FString& Parameters
)
{
	const float DamagedValue =
		UAshenAttributeComponent::
		CalculateValueAfterDamage(
			100.0f,
			100.0f,
			35.0f
		);

	TestTrue(
		TEXT("Damage subtracts from the current value"),
		AshenAutomation::IsNearlyEqual(
			DamagedValue,
			65.0f
		)
	);

	const float LethalDamageValue =
		UAshenAttributeComponent::
		CalculateValueAfterDamage(
			25.0f,
			100.0f,
			500.0f
		);

	TestTrue(
		TEXT("Damage clamps at zero"),
		AshenAutomation::IsNearlyEqual(
			LethalDamageValue,
			0.0f
		)
	);

	const float NegativeDamageValue =
		UAshenAttributeComponent::
		CalculateValueAfterDamage(
			70.0f,
			100.0f,
			-10.0f
		);

	TestTrue(
		TEXT("Negative damage does not heal"),
		AshenAutomation::IsNearlyEqual(
			NegativeDamageValue,
			70.0f
		)
	);

	const float RestoredValue =
		UAshenAttributeComponent::
		CalculateValueAfterRestore(
			65.0f,
			100.0f,
			20.0f
		);

	TestTrue(
		TEXT("Restore adds to the current value"),
		AshenAutomation::IsNearlyEqual(
			RestoredValue,
			85.0f
		)
	);

	const float OverRestoredValue =
		UAshenAttributeComponent::
		CalculateValueAfterRestore(
			90.0f,
			100.0f,
			50.0f
		);

	TestTrue(
		TEXT("Restore clamps at the maximum"),
		AshenAutomation::IsNearlyEqual(
			OverRestoredValue,
			100.0f
		)
	);

	TestTrue(
		TEXT("Exact resource cost is allowed"),
		UAshenAttributeComponent::
		CanConsumeResource(
			40.0f,
			40.0f
		)
	);

	TestFalse(
		TEXT("Resource cost above current value is rejected"),
		UAshenAttributeComponent::
		CanConsumeResource(
			39.0f,
			40.0f
		)
	);

	TestFalse(
		TEXT("Negative resource cost is rejected"),
		UAshenAttributeComponent::
		CanConsumeResource(
			100.0f,
			-1.0f
		)
	);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAshenTargetingVisionConeTest,
	"AshenKeep.Math.Targeting.VisionCone",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter
)

bool FAshenTargetingVisionConeTest::RunTest(
	const FString& Parameters
)
{
	const FVector Forward =
		FVector::ForwardVector;

	TestTrue(
		TEXT("A target directly ahead is inside the cone"),
		UAshenTargetingMathLibrary::
		IsInsideVisionCone2D(
			Forward,
			FVector::ForwardVector,
			60.0f
		)
	);

	TestTrue(
		TEXT("A target at 45 degrees is inside a 60 degree half-angle"),
		UAshenTargetingMathLibrary::
		IsInsideVisionCone2D(
			Forward,
			FVector(1.0f, 1.0f, 0.0f),
			60.0f
		)
	);

	TestFalse(
		TEXT("A side target is outside a 60 degree half-angle"),
		UAshenTargetingMathLibrary::
		IsInsideVisionCone2D(
			Forward,
			FVector::RightVector,
			60.0f
		)
	);

	TestFalse(
		TEXT("A target behind the actor is outside the cone"),
		UAshenTargetingMathLibrary::
		IsInsideVisionCone2D(
			Forward,
			-FVector::ForwardVector,
			80.0f
		)
	);

	const float AheadDot =
		UAshenTargetingMathLibrary::
		CalculateDirectionDot2D(
			Forward,
			FVector::ForwardVector
		);

	TestTrue(
		TEXT("Forward dot product is one"),
		AshenAutomation::IsNearlyEqual(
			AheadDot,
			1.0f
		)
	);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAshenTargetingScoreTest,
	"AshenKeep.Math.Targeting.Score",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter
)

bool FAshenTargetingScoreTest::RunTest(
	const FString& Parameters
)
{
	const float CenteredScore =
		UAshenTargetingMathLibrary::
		CalculateTargetScore(
			1.0f,
			600.0f,
			1200.0f,
			2.0f
		);

	const float OffCenterScore =
		UAshenTargetingMathLibrary::
		CalculateTargetScore(
			0.5f,
			600.0f,
			1200.0f,
			2.0f
		);

	const float FarScore =
		UAshenTargetingMathLibrary::
		CalculateTargetScore(
			1.0f,
			1100.0f,
			1200.0f,
			2.0f
		);

	TestTrue(
		TEXT("A centered target scores better than an equally distant off-center target"),
		CenteredScore < OffCenterScore
	);

	TestTrue(
		TEXT("A closer centered target scores better than a farther centered target"),
		CenteredScore < FarScore
	);

	const float InvalidMaximumScore =
		UAshenTargetingMathLibrary::
		CalculateTargetScore(
			1.0f,
			100.0f,
			0.0f,
			2.0f
		);

	TestTrue(
		TEXT("An invalid maximum distance produces a sentinel score"),
		InvalidMaximumScore ==
			TNumericLimits<float>::Max()
	);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAshenReplicationConfigurationTest,
	"AshenKeep.Network.ReplicationConfiguration",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter
)

bool FAshenReplicationConfigurationTest::RunTest(
	const FString& Parameters
)
{
	const AAshenPlayerCharacter* PlayerDefaults =
		GetDefault<AAshenPlayerCharacter>();

	const AAshenTrainingEnemy* EnemyDefaults =
		GetDefault<AAshenTrainingEnemy>();

	const AAshenPurgeRitualObjective*
		ObjectiveDefaults =
		GetDefault<
			AAshenPurgeRitualObjective
		>();

	const UAshenBloodBurstComponent*
		BloodBurstDefaults =
		GetDefault<
			UAshenBloodBurstComponent
		>();

	TestNotNull(
		TEXT("Player class default object exists"),
		PlayerDefaults
	);

	TestNotNull(
		TEXT("Enemy class default object exists"),
		EnemyDefaults
	);

	TestNotNull(
		TEXT("Objective class default object exists"),
		ObjectiveDefaults
	);

	TestNotNull(
		TEXT("Blood Burst component default object exists"),
		BloodBurstDefaults
	);

	if (!PlayerDefaults ||
		!EnemyDefaults ||
		!ObjectiveDefaults ||
		!BloodBurstDefaults)
	{
		return false;
	}

	TestTrue(
		TEXT("Player replication is enabled"),
		PlayerDefaults->GetIsReplicated()
	);

	TestTrue(
		TEXT("Player movement replication is enabled"),
		PlayerDefaults->IsReplicatingMovement()
	);

	TestNotNull(
		TEXT("Player owns an attribute component"),
		PlayerDefaults->GetAttributeComponent()
	);

	TestNotNull(
		TEXT("Player owns a lock-on component"),
		PlayerDefaults->GetLockOnComponent()
	);

	TestTrue(
		TEXT("Enemy replication is enabled"),
		EnemyDefaults->GetIsReplicated()
	);

	TestTrue(
		TEXT("Enemy movement replication is enabled"),
		EnemyDefaults->IsReplicatingMovement()
	);

	TestNotNull(
		TEXT("Enemy owns an attribute component"),
		EnemyDefaults->GetAttributeComponent()
	);

	TestTrue(
		TEXT("Purge ritual objective replication is enabled"),
		ObjectiveDefaults->GetIsReplicated()
	);

	TestTrue(
		TEXT("Blood Burst component replication is enabled"),
		BloodBurstDefaults->GetIsReplicated()
	);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAshenNetworkFunctionFlagsTest,
	"AshenKeep.Network.RPCFunctionFlags",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter
)

bool FAshenNetworkFunctionFlagsTest::RunTest(
	const FString& Parameters
)
{
	TestTrue(
		TEXT("ServerAttack is a reliable Server RPC"),
		AshenAutomation::HasFunctionFlags(
			AAshenPlayerCharacter::StaticClass(),
			TEXT("ServerAttack"),
			FUNC_Net |
			FUNC_NetServer |
			FUNC_NetReliable
		)
	);

	TestTrue(
		TEXT("ServerDodge is a reliable Server RPC"),
		AshenAutomation::HasFunctionFlags(
			AAshenPlayerCharacter::StaticClass(),
			TEXT("ServerDodge"),
			FUNC_Net |
			FUNC_NetServer |
			FUNC_NetReliable
		)
	);

	TestTrue(
		TEXT("Blood Burst activation is a reliable Server RPC"),
		AshenAutomation::HasFunctionFlags(
			UAshenBloodBurstComponent::StaticClass(),
			TEXT("ServerActivateBloodBurst"),
			FUNC_Net |
			FUNC_NetServer |
			FUNC_NetReliable
		)
	);

	TestTrue(
		TEXT("Blood Burst cue is an unreliable multicast"),
		AshenAutomation::HasFunctionFlags(
			UAshenBloodBurstComponent::StaticClass(),
			TEXT("MulticastBloodBurstCue"),
			FUNC_Net |
			FUNC_NetMulticast
		) &&
		!AshenAutomation::HasFunctionFlags(
			UAshenBloodBurstComponent::StaticClass(),
			TEXT("MulticastBloodBurstCue"),
			FUNC_NetReliable
		)
	);

	TestTrue(
		TEXT("Victory presentation is a reliable multicast"),
		AshenAutomation::HasFunctionFlags(
			AAshenPurgeRitualObjective::StaticClass(),
			TEXT("MulticastShowVictory"),
			FUNC_Net |
			FUNC_NetMulticast |
			FUNC_NetReliable
		)
	);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAshenPerformanceConfigurationTest,
	"AshenKeep.Performance.TickConfiguration",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter
)

bool FAshenPerformanceConfigurationTest::RunTest(
	const FString& Parameters
)
{
	const AAshenEnemyAIController* AIDefaults =
		GetDefault<AAshenEnemyAIController>();

	const AAshenDamageVolume* DamageVolumeDefaults =
		GetDefault<AAshenDamageVolume>();

	const AAshenPurgeRitualObjective*
		ObjectiveDefaults =
		GetDefault<
			AAshenPurgeRitualObjective
		>();

	const UAshenLockOnComponent* LockOnDefaults =
		GetDefault<UAshenLockOnComponent>();

	TestTrue(
		TEXT("Enemy AI uses a timer instead of actor Tick"),
		AIDefaults &&
		!AIDefaults->
			PrimaryActorTick.bCanEverTick
	);

	TestTrue(
		TEXT("Damage volume does not use actor Tick"),
		DamageVolumeDefaults &&
		!DamageVolumeDefaults->
			PrimaryActorTick.bCanEverTick
	);

	TestTrue(
		TEXT("Purge objective does not use actor Tick"),
		ObjectiveDefaults &&
		!ObjectiveDefaults->
			PrimaryActorTick.bCanEverTick
	);

	TestTrue(
		TEXT("Lock-on component starts with Tick disabled"),
		LockOnDefaults &&
		!LockOnDefaults->
			PrimaryComponentTick.
			bStartWithTickEnabled
	);

	return true;
}

#endif