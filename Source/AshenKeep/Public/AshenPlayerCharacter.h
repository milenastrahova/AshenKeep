#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
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

private:
	void CreatePlayerHUD();

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
};