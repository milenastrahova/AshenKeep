#include "AshenEnemyHealthWidget.h"

#include "AshenAttributeComponent.h"
#include "AshenTrainingEnemy.h"
#include "Components/ProgressBar.h"

void UAshenEnemyHealthWidget::SetObservedEnemy(
	AAshenTrainingEnemy* NewEnemy
)
{
	ObservedEnemy = NewEnemy;

	RefreshHealthBar();
}

void UAshenEnemyHealthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshHealthBar();
}

void UAshenEnemyHealthWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime
)
{
	Super::NativeTick(
		MyGeometry,
		InDeltaTime
	);

	RefreshHealthBar();
}

void UAshenEnemyHealthWidget::RefreshHealthBar()
{
	AAshenTrainingEnemy* Enemy =
		ObservedEnemy.Get();

	if (!IsValid(Enemy) || !HealthBar)
	{
		SetVisibility(
			ESlateVisibility::Collapsed
		);

		return;
	}

	UAshenAttributeComponent* Attributes =
		Enemy->GetAttributeComponent();

	if (!Attributes || Enemy->IsDead())
	{
		SetVisibility(
			ESlateVisibility::Collapsed
		);

		return;
	}

	const float MaximumHealth =
		FMath::Max(
			Attributes->GetMaxHealth(),
			KINDA_SMALL_NUMBER
		);

	const float HealthPercent =
		FMath::Clamp(
			Attributes->GetHealth() /
			MaximumHealth,
			0.0f,
			1.0f
		);

	HealthBar->SetPercent(
		HealthPercent
	);

	SetVisibility(
		ESlateVisibility::HitTestInvisible
	);
}