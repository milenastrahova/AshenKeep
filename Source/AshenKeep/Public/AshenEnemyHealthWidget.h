#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AshenEnemyHealthWidget.generated.h"

class UAshenAttributeComponent;
class UProgressBar;
class AAshenTrainingEnemy;

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
};