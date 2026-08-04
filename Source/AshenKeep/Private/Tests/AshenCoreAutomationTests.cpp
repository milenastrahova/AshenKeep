#include "Misc/AutomationTest.h"

#include "AshenAttributeComponent.h"
#include "AshenLockOnComponent.h"
#include "AshenPlayerCharacter.h"
#include "AshenPurgeRitualObjective.h"
#include "AshenTrainingEnemy.h"

#if WITH_DEV_AUTOMATION_TESTS

/*
 * Проверяет базовую конфигурацию здоровья,
 * выносливости и ресурса крови.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAshenAttributeDefaultsTest,
	"AshenKeep.Core.Attributes.DefaultValues",
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
		TEXT("Attribute component default object exists"),
		Attributes
	);

	if (!Attributes)
	{
		return false;
	}

	TestTrue(
		TEXT("Maximum health is greater than zero"),
		Attributes->GetMaxHealth() > 0.0f
	);

	TestTrue(
		TEXT("Maximum stamina is greater than zero"),
		Attributes->GetMaxStamina() > 0.0f
	);

	TestTrue(
		TEXT("Maximum blood resource is greater than zero"),
		Attributes->GetMaxMana() > 0.0f
	);

	TestTrue(
		TEXT("Health begins inside the valid range"),
		Attributes->GetHealth() >= 0.0f &&
		Attributes->GetHealth() <=
		Attributes->GetMaxHealth()
	);

	TestTrue(
		TEXT("Stamina begins inside the valid range"),
		Attributes->GetStamina() >= 0.0f &&
		Attributes->GetStamina() <=
		Attributes->GetMaxStamina()
	);

	TestTrue(
		TEXT("Blood resource begins inside the valid range"),
		Attributes->GetMana() >= 0.0f &&
		Attributes->GetMana() <=
		Attributes->GetMaxMana()
	);

	return true;
}

/*
 * Проверяет, что ключевые классы Ashen Keep
 * правильно подготовлены к сетевой игре.
 */
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

	const AAshenPurgeRitualObjective* ObjectiveDefaults =
		GetDefault<AAshenPurgeRitualObjective>();

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

	if (!PlayerDefaults ||
		!EnemyDefaults ||
		!ObjectiveDefaults)
	{
		return false;
	}

	TestTrue(
		TEXT("Player actor replication is enabled"),
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
		TEXT("Enemy actor replication is enabled"),
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

	return true;
}

#endif