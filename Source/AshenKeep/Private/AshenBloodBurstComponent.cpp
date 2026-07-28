#include "AshenBloodBurstComponent.h"

#include "AshenAttributeComponent.h"
#include "AshenTrainingEnemy.h"

#include "CollisionShape.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UAshenBloodBurstComponent::UAshenBloodBurstComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UAshenBloodBurstComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAshenBloodBurstComponent::ActivateBloodBurst()
{
	AActor* OwnerActor = GetOwner();

	if (!IsValid(OwnerActor))
	{
		return;
	}

	if (OwnerActor->HasAuthority())
	{
		PerformBloodBurst();
	}
	else
	{
		ServerActivateBloodBurst();
	}
}

void UAshenBloodBurstComponent::
ServerActivateBloodBurst_Implementation()
{
	PerformBloodBurst();
}

bool UAshenBloodBurstComponent::
CanActivateBloodBurst() const
{
	const AActor* OwnerActor = GetOwner();

	if (!IsValid(OwnerActor) ||
		!bCanUseBloodBurst)
	{
		return false;
	}

	const UAshenAttributeComponent* OwnerAttributes =
		GetOwnerAttributes();

	if (!OwnerAttributes ||
		!OwnerAttributes->IsAlive())
	{
		return false;
	}

	return OwnerAttributes->HasEnoughMana(BloodCost);
}

void UAshenBloodBurstComponent::PerformBloodBurst()
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();

	if (!IsValid(OwnerActor) ||
		!World ||
		!OwnerActor->HasAuthority() ||
		!bCanUseBloodBurst)
	{
		return;
	}

	UAshenAttributeComponent* OwnerAttributes =
		GetOwnerAttributes();

	if (!OwnerAttributes ||
		!OwnerAttributes->IsAlive() ||
		!OwnerAttributes->HasEnoughMana(BloodCost))
	{
		return;
	}

	const bool bConsumedBlood =
		OwnerAttributes->ConsumeMana(BloodCost);

	if (!bConsumedBlood)
	{
		return;
	}

	bCanUseBloodBurst = false;

	World->GetTimerManager().SetTimer(
		BloodBurstCooldownTimerHandle,
		this,
		&UAshenBloodBurstComponent::
		ResetBloodBurstCooldown,
		Cooldown,
		false
	);

	const FVector BurstLocation =
		OwnerActor->GetActorLocation();

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(AshenBloodBurst),
		false,
		OwnerActor
	);

	TArray<FOverlapResult> OverlapResults;

	const bool bFoundAnything =
		World->OverlapMultiByObjectType(
			OverlapResults,
			BurstLocation,
			FQuat::Identity,
			ObjectQueryParams,
			FCollisionShape::MakeSphere(Radius),
			QueryParams
		);

	int32 HuntersHit = 0;

	TSet<TWeakObjectPtr<AAshenTrainingEnemy>>
		DamagedHunters;

	if (bFoundAnything)
	{
		for (const FOverlapResult& OverlapResult
			: OverlapResults)
		{
			AAshenTrainingEnemy* Hunter =
				Cast<AAshenTrainingEnemy>(
					OverlapResult.GetActor()
				);

			if (!IsValid(Hunter))
			{
				continue;
			}

			const TWeakObjectPtr<
				AAshenTrainingEnemy
			> HunterKey(Hunter);

			if (DamagedHunters.Contains(HunterKey))
			{
				continue;
			}

			UAshenAttributeComponent*
				HunterAttributes =
				Hunter->FindComponentByClass<
				UAshenAttributeComponent
				>();

			if (!HunterAttributes ||
				!HunterAttributes->IsAlive())
			{
				continue;
			}

			const bool bWasAlive =
				HunterAttributes->IsAlive();

			const float AppliedDamage =
				HunterAttributes->ApplyDamage(Damage);

			if (AppliedDamage <= 0.0f)
			{
				continue;
			}

			DamagedHunters.Add(HunterKey);
			++HuntersHit;

			const bool bKilledHunter =
				bWasAlive &&
				!HunterAttributes->IsAlive();

			if (bKilledHunter)
			{
				ApplyKillReward(OwnerAttributes);
			}

			UE_LOG(
				LogTemp,
				Log,
				TEXT(
					"Blood Burst hit %s for %.1f damage."
				),
				*Hunter->GetName(),
				AppliedDamage
			);
		}
	}

	if (bDrawDebug)
	{
		DrawDebugSphere(
			World,
			BurstLocation,
			Radius,
			32,
			HuntersHit > 0
			? FColor::Red
			: FColor::Purple,
			false,
			1.0f,
			0,
			3.0f
		);
	}

	MulticastBloodBurstCue(
		BurstLocation,
		Radius,
		HuntersHit
	);

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Blood Burst activated. Hunters hit: %d."
		),
		HuntersHit
	);
}

void UAshenBloodBurstComponent::
ApplyKillReward(
	UAshenAttributeComponent* OwnerAttributes
)
{
	if (!OwnerAttributes)
	{
		return;
	}

	OwnerAttributes->RestoreHealth(
		KillHealthReward
	);

	OwnerAttributes->RestoreMana(
		KillBloodReward
	);
}

void UAshenBloodBurstComponent::
MulticastBloodBurstCue_Implementation(
	FVector_NetQuantize BurstLocation,
	float BurstRadius,
	int32 HuntersHit
)
{
	BP_OnBloodBurst(
		BurstLocation,
		BurstRadius,
		HuntersHit
	);
}

void UAshenBloodBurstComponent::
ResetBloodBurstCooldown()
{
	bCanUseBloodBurst = true;
}

UAshenAttributeComponent*
UAshenBloodBurstComponent::GetOwnerAttributes() const
{
	const AActor* OwnerActor = GetOwner();

	if (!IsValid(OwnerActor))
	{
		return nullptr;
	}

	return OwnerActor->FindComponentByClass<
		UAshenAttributeComponent
	>();
}