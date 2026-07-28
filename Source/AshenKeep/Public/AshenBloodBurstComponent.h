#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AshenBloodBurstComponent.generated.h"

class AAshenTrainingEnemy;
class UAshenAttributeComponent;

UCLASS(
	ClassGroup = (AshenKeep),
	meta = (BlueprintSpawnableComponent)
)
class ASHENKEEP_API UAshenBloodBurstComponent
	: public UActorComponent
{
	GENERATED_BODY()

public:
	UAshenBloodBurstComponent();

	UFUNCTION(
		BlueprintCallable,
		Category = "Ashen Keep|Vampire"
	)
	void ActivateBloodBurst();

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Vampire"
	)
	bool CanActivateBloodBurst() const;

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Vampire"
	)
	bool IsBloodBurstOnCooldown() const
	{
		return !bCanUseBloodBurst;
	}

protected:
	virtual void BeginPlay() override;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Blood Burst",
		meta = (ClampMin = "0.0")
	)
	float BloodCost = 40.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Blood Burst",
		meta = (ClampMin = "0.0")
	)
	float Damage = 45.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Blood Burst",
		meta = (ClampMin = "1.0")
	)
	float Radius = 450.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Blood Burst",
		meta = (ClampMin = "0.1")
	)
	float Cooldown = 3.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Blood Burst"
	)
	bool bDrawDebug = true;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Blood Burst",
		meta = (ClampMin = "0.0")
	)
	float KillHealthReward = 10.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Blood Burst",
		meta = (ClampMin = "0.0")
	)
	float KillBloodReward = 10.0f;

	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Ashen Keep|Blood Burst",
		meta = (DisplayName = "On Blood Burst")
	)
	void BP_OnBloodBurst(
		FVector BurstLocation,
		float BurstRadius,
		int32 HuntersHit
	);

private:
	UFUNCTION(Server, Reliable)
	void ServerActivateBloodBurst();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastBloodBurstCue(
		FVector_NetQuantize BurstLocation,
		float BurstRadius,
		int32 HuntersHit
	);

	void PerformBloodBurst();
	void ResetBloodBurstCooldown();

	UAshenAttributeComponent* GetOwnerAttributes() const;

	void ApplyKillReward(
		UAshenAttributeComponent* OwnerAttributes
	);

	bool bCanUseBloodBurst = true;

	FTimerHandle BloodBurstCooldownTimerHandle;
};