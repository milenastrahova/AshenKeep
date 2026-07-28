#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "AshenTrainingEnemy.generated.h"

class UAshenAttributeComponent;

UCLASS()
class ASHENKEEP_API AAshenTrainingEnemy
	: public ACharacter
{
	GENERATED_BODY()

public:
	AAshenTrainingEnemy();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Attributes"
	)
	UAshenAttributeComponent*
		GetAttributeComponent() const
	{
		return AttributeComponent;
	}

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Combat"
	)
	bool IsDead() const
	{
		return bIsDead;
	}

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Combat"
	)
	float GetAttackRange() const
	{
		return AttackRange;
	}

	UFUNCTION(
		BlueprintCallable,
		Category = "Ashen Keep|Combat"
	)
	bool TryAttack(AActor* TargetActor);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Attributes"
	)
	TObjectPtr<UAshenAttributeComponent>
		AttributeComponent;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|AI",
		meta = (ClampMin = "0.0")
	)
	float ChaseSpeed = 320.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Combat",
		meta = (ClampMin = "0.0")
	)
	float AttackDamage = 20.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Combat",
		meta = (ClampMin = "0.0")
	)
	float AttackRange = 165.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Combat",
		meta = (ClampMin = "1.0")
	)
	float AttackRadius = 60.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Combat",
		meta = (ClampMin = "0.1")
	)
	float AttackCooldown = 1.2f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Combat"
	)
	bool bDrawAttackDebug = true;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Death",
		meta = (ClampMin = "0.0")
	)
	float DeathImpulse = 250.0f;

	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Ashen Keep|Combat",
		meta = (DisplayName = "On Enemy Attack")
	)
	void BP_OnAttack();

private:
	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void OnRep_IsDead();

	UFUNCTION(
		NetMulticast,
		Unreliable
	)
	void MulticastPlayAttackCue();

	void ApplyDeathState();
	void ResetAttackCooldown();

	UPROPERTY(
		ReplicatedUsing = OnRep_IsDead,
		VisibleInstanceOnly,
		Category = "Ashen Keep|Death"
	)
	bool bIsDead = false;

	bool bCanAttack = true;

	FTimerHandle AttackCooldownTimerHandle;
};