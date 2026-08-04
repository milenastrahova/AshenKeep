#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "AshenAttributeComponent.h"
#include "AshenPlayerHUDWidget.h"
#include "AshenPlayerCharacter.generated.h"

class UAnimationAsset;
class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;
class UInputComponent;
class UAshenLockOnComponent;
class USoundBase;

UCLASS()
class ASHENKEEP_API AAshenPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAshenPlayerCharacter();

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(
		UInputComponent* PlayerInputComponent
	) override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Attributes"
	)
	UAshenAttributeComponent* GetAttributeComponent() const
	{
		return AttributeComponent;
	}

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Death"
	)
	bool IsDead() const
	{
		return bIsDead;
	}

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Mist Step"
	)
	bool IsMistStepping() const
	{
		return bIsMistStepping;
	}

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Lock On"
	)
	UAshenLockOnComponent* GetLockOnComponent() const
	{
		return LockOnTargetingComponent;
	}

protected:
	virtual void BeginPlay() override;
	virtual void NotifyControllerChanged() override;

	void Move(const FInputActionValue& Value);
	void StopMove(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);

	void Dodge(const FInputActionValue& Value);
	void Attack(const FInputActionValue& Value);
	void ToggleLockOn(const FInputActionValue& Value);

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|UI"
	)
	TSubclassOf<UAshenPlayerHUDWidget> HUDWidgetClass;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Input"
	)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Input"
	)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Input"
	)
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Input"
	)
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Input"
	)
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Input"
	)
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Input"
	)
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Input"
	)
	TObjectPtr<UInputAction> LockOnAction;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Camera",
		meta = (ClampMin = "0.05")
	)
	float LookSensitivityX = 0.45f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Camera",
		meta = (ClampMin = "0.05")
	)
	float LookSensitivityY = 0.35f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Movement",
		meta = (ClampMin = "0.0")
	)
	float WalkSpeed = 450.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Movement",
		meta = (ClampMin = "0.0")
	)
	float SprintSpeed = 700.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Movement",
		meta = (ClampMin = "0.0")
	)
	float SprintStaminaCostPerSecond = 20.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Movement",
		meta = (ClampMin = "0.0")
	)
	float StaminaRegenerationPerSecond = 15.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Movement",
		meta = (ClampMin = "0.0")
	)
	float StaminaRegenerationDelay = 1.5f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Mist Step",
		meta = (ClampMin = "0.0")
	)
	float DodgeStrength = 1150.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Mist Step",
		meta = (ClampMin = "0.0")
	)
	float DodgeVerticalBoost = 35.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Mist Step",
		meta = (ClampMin = "0.0")
	)
	float DodgeStaminaCost = 25.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Mist Step",
		meta = (ClampMin = "0.1")
	)
	float DodgeCooldown = 0.75f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Mist Step",
		meta = (ClampMin = "0.05")
	)
	float MistStepDuration = 0.22f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Combat",
		meta = (ClampMin = "0.0")
	)
	float AttackDamage = 35.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Combat",
		meta = (ClampMin = "0.0")
	)
	float AttackRange = 240.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Combat",
		meta = (ClampMin = "1.0")
	)
	float AttackRadius = 100.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Combat",
		meta = (ClampMin = "0.0")
	)
	float AttackStaminaCost = 10.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Combat",
		meta = (ClampMin = "0.1")
	)
	float AttackCooldown = 0.45f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Combat"
	)
	bool bDrawAttackDebug = false;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Vampire",
		meta = (ClampMin = "0.0")
	)
	float VampiricHealthOnKill = 25.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Vampire",
		meta = (ClampMin = "0.0")
	)
	float BloodEssenceOnKill = 35.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Death",
		meta = (ClampMin = "0.0")
	)
	float DeathImpulse = 180.0f;


	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Audio"
	)
	TObjectPtr<USoundBase> AttackSwingSound;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Audio"
	)
	TObjectPtr<USoundBase> AttackHitSound;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Audio",
		meta = (ClampMin = "0.0")
	)
	float AttackSwingVolume = 0.72f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Audio",
		meta = (ClampMin = "0.0")
	)
	float AttackHitVolume = 0.82f;

	/*
	 * Paragon demo Animation Blueprints depend on obsolete sample
	 * PlayerCharacter classes. Ashen Keep uses compatible animation
	 * sequences directly through AnimationSingleNode instead.
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

	/*
	 * Countess uses one compatible locomotion sequence for both moving and
	 * standing. Only the play rate changes, so stopping never snaps to a
	 * different pose or animation asset.
	 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Animation",
		meta = (ClampMin = "0.0", ClampMax = "1.0")
	)
	float IdleLocomotionPlayRate = 0.08f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Animation",
		meta = (ClampMin = "0.1")
	)
	float MovingLocomotionPlayRate = 1.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Animation",
		meta = (ClampMin = "0.1")
	)
	float LocomotionPlayRateInterpSpeed = 7.0f;

	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Ashen Keep|Vampire",
		meta = (DisplayName = "On Vampiric Recovery")
	)
	void BP_OnVampiricRecovery(
		float HealthRestored,
		float BloodRestored
	);

	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Ashen Keep|Mist Step",
		meta = (DisplayName = "On Mist Step")
	)
	void BP_OnMistStep(
		FVector StartLocation,
		FVector Direction
	);

private:
	void CreatePlayerHUD();

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprinting);

	UFUNCTION(Server, Reliable)
	void ServerDodge(FVector DodgeDirection);

	UFUNCTION(Server, Reliable)
	void ServerAttack();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayAttackAnimationCue();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayAttackHitSound(
		FVector_NetQuantize HitLocation
	);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayVampiricRecoveryCue(
		float HealthRestored,
		float BloodRestored
	);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayMistStepCue(
		FVector_NetQuantize StartLocation,
		FVector_NetQuantizeNormal Direction
	);

	UFUNCTION()
	void OnRep_IsSprinting();

	UFUNCTION()
	void OnRep_IsMistStepping();

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void OnRep_IsDead();

	void SetSprinting(bool bNewSprinting);
	void ApplyMovementSpeed();

	void UpdateSprintStamina();
	void RegenerateStamina();
	void StartStaminaRegeneration();
	void StopStaminaTimers();

	void PerformDodge(const FVector& DodgeDirection);
	void EndMistStep();
	void ApplyMistStepState();
	void ResetDodgeCooldown();

	void PerformAttack();
	void ResetAttackCooldown();

	void UpdateSimpleAnimation(float DeltaSeconds);
	void PlaySimpleAnimation(
		UAnimationAsset* Animation,
		bool bLooping
	);
	void PlayAttackAnimationLocally();
	void FinishAttackAnimationLocally();

	void ApplyVampiricKillReward();
	void ApplyDeathState();

	FVector GetCameraForwardDirection() const;
	FVector GetDesiredDodgeDirection() const;

	UPROPERTY(Transient)
	TObjectPtr<UAshenPlayerHUDWidget> HUDWidgetInstance;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Camera",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Camera",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Attributes",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UAshenAttributeComponent> AttributeComponent;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Lock On",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UAshenLockOnComponent> LockOnTargetingComponent;

	UPROPERTY(
		ReplicatedUsing = OnRep_IsSprinting,
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Movement",
		meta = (AllowPrivateAccess = "true")
	)
	bool bIsSprinting = false;

	UPROPERTY(
		ReplicatedUsing = OnRep_IsMistStepping,
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Mist Step",
		meta = (AllowPrivateAccess = "true")
	)
	bool bIsMistStepping = false;

	UPROPERTY(
		ReplicatedUsing = OnRep_IsDead,
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Death",
		meta = (AllowPrivateAccess = "true")
	)
	bool bIsDead = false;

	FVector2D CachedMovementInput = FVector2D::ZeroVector;

	float StaminaUpdateInterval = 0.1f;

	FTimerHandle SprintStaminaTimerHandle;
	FTimerHandle StaminaRegenerationTimerHandle;

	bool bCanDodge = true;

	FTimerHandle DodgeCooldownTimerHandle;
	FTimerHandle MistStepTimerHandle;

	bool bCanAttack = true;
	bool bAttackAnimationPlaying = false;
	bool bCurrentAnimationLooping = false;
	float CurrentLocomotionPlayRate = 0.08f;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CurrentSimpleAnimation;

	FTimerHandle AttackCooldownTimerHandle;
	FTimerHandle SimpleAttackAnimationTimerHandle;

	// Важно: обычный ECollisionResponse,
	// а не TEnumAsByte.
	ECollisionResponse OriginalPawnCollisionResponse = ECR_Block;
};