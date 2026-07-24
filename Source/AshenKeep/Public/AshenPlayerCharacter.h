#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "AshenAttributeComponent.h"
#include "AshenPlayerHUDWidget.h"
#include "AshenPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;
class UInputComponent;

UCLASS()
class ASHENKEEP_API AAshenPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAshenPlayerCharacter();

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

protected:
	virtual void BeginPlay() override;
	virtual void NotifyControllerChanged() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);

	void Dodge(const FInputActionValue& Value);

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
		Category = "Ashen Keep|Dodge",
		meta = (ClampMin = "0.0")
	)
	float DodgeStrength = 900.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Dodge",
		meta = (ClampMin = "0.0")
	)
	float DodgeVerticalBoost = 90.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Dodge",
		meta = (ClampMin = "0.0")
	)
	float DodgeStaminaCost = 25.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Dodge",
		meta = (ClampMin = "0.1")
	)
	float DodgeCooldown = 0.75f;

private:
	void CreatePlayerHUD();

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprinting);

	UFUNCTION(Server, Reliable)
	void ServerDodge(FVector DodgeDirection);

	UFUNCTION()
	void OnRep_IsSprinting();

	void SetSprinting(bool bNewSprinting);
	void ApplyMovementSpeed();

	void UpdateSprintStamina();
	void RegenerateStamina();
	void StartStaminaRegeneration();
	void StopStaminaTimers();

	void PerformDodge(const FVector& DodgeDirection);
	void ResetDodgeCooldown();

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
		ReplicatedUsing = OnRep_IsSprinting,
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Movement",
		meta = (AllowPrivateAccess = "true")
	)
	bool bIsSprinting = false;

	float StaminaUpdateInterval = 0.1f;

	FTimerHandle SprintStaminaTimerHandle;
	FTimerHandle StaminaRegenerationTimerHandle;

	bool bCanDodge = true;

	FTimerHandle DodgeCooldownTimerHandle;
};