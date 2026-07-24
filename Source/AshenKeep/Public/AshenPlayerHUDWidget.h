#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AshenPlayerHUDWidget.generated.h"

class UAshenAttributeComponent;
class UProgressBar;

UCLASS()
class ASHENKEEP_API UAshenPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(
		meta = (BindWidgetOptional),
		BlueprintReadOnly,
		Category = "Ashen Keep|HUD"
	)
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(
		meta = (BindWidgetOptional),
		BlueprintReadOnly,
		Category = "Ashen Keep|HUD"
	)
	TObjectPtr<UProgressBar> StaminaBar;

	UPROPERTY(
		meta = (BindWidgetOptional),
		BlueprintReadOnly,
		Category = "Ashen Keep|HUD"
	)
	TObjectPtr<UProgressBar> ManaBar;

private:
	void BindToAttributeComponent();
	void UnbindFromAttributeComponent();

	UFUNCTION()
	void HandleHealthChanged(
		float NewValue,
		float MaxValue,
		float Delta
	);

	UFUNCTION()
	void HandleStaminaChanged(
		float NewValue,
		float MaxValue,
		float Delta
	);

	UFUNCTION()
	void HandleManaChanged(
		float NewValue,
		float MaxValue,
		float Delta
	);

	static float CalculatePercent(
		float CurrentValue,
		float MaxValue
	);

	UPROPERTY(Transient)
	TObjectPtr<UAshenAttributeComponent> AttributeComponent;
};