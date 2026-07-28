#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AshenEnemyHealthWidget.generated.h"

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
	void RefreshHealthBar();

	TWeakObjectPtr<AAshenTrainingEnemy>
		ObservedEnemy;
};