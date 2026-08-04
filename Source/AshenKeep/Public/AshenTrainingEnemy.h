#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AshenTrainingEnemy.generated.h"

class UAnimationAsset;
class UAshenAttributeComponent;
class UWidgetComponent;
class AAshenPlayerCharacter;
class USoundBase;

UCLASS()
class ASHENKEEP_API AAshenTrainingEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AAshenTrainingEnemy();

	virtual void Tick(float DeltaSeconds) override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;

	UFUNCTION(
		BlueprintCallable,
		Category = "Ashen Keep|Combat"
	)
	void TryAttack(AAshenPlayerCharacter* Target);

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Enemy"
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
		BlueprintPure,
		Category = "Ashen Keep|Attributes"
	)
	UAshenAttributeComponent* GetAttributeComponent() const
	{
		return AttributeComponent;
	}

protected:
	virtual void BeginPlay() override;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Attributes"
	)
	TObjectPtr<UAshenAttributeComponent> AttributeComponent;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|UI"
	)
	TObjectPtr<UWidgetComponent> HealthWidgetComponent;

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
	float AttackDamage = 15.0f;

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
		meta = (ClampMin = "0.0")
	)
	float AttackRadius = 80.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Combat",
		meta = (ClampMin = "0.0")
	)
	float AttackCooldown = 1.25f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Combat|Debug"
	)
	bool bDrawAttackDebug = false;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Death",
		meta = (ClampMin = "0.0")
	)
	float DeathImpulse = 500.0f;


	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Audio"
	)
	TObjectPtr<USoundBase> AttackSound;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Audio"
	)
	TObjectPtr<USoundBase> DeathSound;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Audio",
		meta = (ClampMin = "0.0")
	)
	float AttackSoundVolume = 0.62f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Audio",
		meta = (ClampMin = "0.0")
	)
	float DeathSoundVolume = 0.78f;

	/*
	 * The imported Paragon Animation Blueprints expect their own
	 * demonstration PlayerCharacter classes. Our AI enemies use a
	 * reliable animation-asset player instead.
	 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Animation"
	)
	bool bUseSimpleAnimationSystem = true;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Animation"
	)
	TObjectPtr<UAnimationAsset> IdleAnimation;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Animation"
	)
	TObjectPtr<UAnimationAsset> WalkAnimation;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Animation"
	)
	TObjectPtr<UAnimationAsset> AttackAnimation;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Animation"
	)
	TObjectPtr<UAnimationAsset> DeathAnimation;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Animation",
		meta = (ClampMin = "0.0")
	)
	float MoveAnimationSpeedThreshold = 10.0f;

	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Ashen Keep|Animation"
	)
	void BP_OnAttack();

private:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayAttackCue();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayDeathSound(
		FVector_NetQuantize DeathLocation
	);

	UFUNCTION()
	void ResetAttackCooldown();

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void OnRep_IsDead();

	void ApplyDeathState();

	void UpdateSimpleAnimation();

	void PlaySimpleAnimation(
		UAnimationAsset* Animation,
		bool bLooping
	);

	void PlayAttackAnimationLocally();

	void FinishAttackAnimationLocally();

	UPROPERTY(
		ReplicatedUsing = OnRep_IsDead
	)
	bool bIsDead = false;

	bool bCanAttack = true;
	bool bAttackAnimationPlaying = false;
	bool bCurrentAnimationLooping = false;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CurrentSimpleAnimation;

	FTimerHandle AttackCooldownTimerHandle;
	FTimerHandle SimpleAttackAnimationTimerHandle;
};
