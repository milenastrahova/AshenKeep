#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AshenEnemyHealthWidget.generated.h"

class UAshenAttributeComponent;
class UProgressBar;
class AAshenTrainingEnemy;

/**
 * Enemy health bar driven by attribute delegates with a low-frequency
 * fallback refresh. The fallback protects WidgetComponent lifecycle cases
 * where a delegate binding can be recreated after the first damage event.
 */
UCLASS(Abstract)
class ASHENKEEP_API UAshenEnemyHealthWidget
	: public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "Ashen Keep|UI"
	)
	void SetObservedEnemy(
		AAshenTrainingEnemy* NewEnemy
	);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime
	) override;

	UPROPERTY(
		meta = (BindWidgetOptional),
		BlueprintReadOnly,
		Category = "Ashen Keep|UI"
	)
	TObjectPtr<UProgressBar> HealthBar;

private:
	void BindToAttributes(
		UAshenAttributeComponent* NewAttributes
	);

	void UnbindFromAttributes();
	void RefreshHealthBar();

	void UpdateHealthBarFromValues(
		float CurrentHealth,
		float MaximumHealth
	);

	UFUNCTION()
	void HandleHealthChanged(
		float NewValue,
		float MaxValue,
		float Delta
	);

	UFUNCTION()
	void HandleObservedEnemyDeath();

	TWeakObjectPtr<AAshenTrainingEnemy>
		ObservedEnemy;

	UPROPERTY(Transient)
	TObjectPtr<UAshenAttributeComponent>
		ObservedAttributes;

	float RefreshAccumulator = 0.0f;

	static constexpr float RefreshInterval =
		0.05f;
};