#include "AshenEnemyHealthWidget.h"

#include "AshenAttributeComponent.h"
#include "AshenTrainingEnemy.h"
#include "Components/ProgressBar.h"

void UAshenEnemyHealthWidget::SetObservedEnemy(
	AAshenTrainingEnemy* NewEnemy
)
{
	if (ObservedEnemy.Get() == NewEnemy)
	{
		RefreshHealthBar();
		return;
	}

	UnbindFromAttributes();

	ObservedEnemy = NewEnemy;

	BindToAttributes(
		IsValid(NewEnemy)
			? NewEnemy->GetAttributeComponent()
			: nullptr
	);

	RefreshHealthBar();
}

void UAshenEnemyHealthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AAshenTrainingEnemy* Enemy =
		ObservedEnemy.Get())
	{
		BindToAttributes(
			Enemy->GetAttributeComponent()
		);
	}

	RefreshHealthBar();
}

void UAshenEnemyHealthWidget::NativeDestruct()
{
	UnbindFromAttributes();

	Super::NativeDestruct();
}

void UAshenEnemyHealthWidget::BindToAttributes(
	UAshenAttributeComponent* NewAttributes
)
{
	if (ObservedAttributes == NewAttributes)
	{
		return;
	}

	UnbindFromAttributes();

	ObservedAttributes = NewAttributes;

	if (!ObservedAttributes)
	{
		return;
	}

	ObservedAttributes->OnHealthChanged.AddUniqueDynamic(
		this,
		&UAshenEnemyHealthWidget::
		HandleHealthChanged
	);

	ObservedAttributes->OnDeath.AddUniqueDynamic(
		this,
		&UAshenEnemyHealthWidget::
		HandleObservedEnemyDeath
	);
}

void UAshenEnemyHealthWidget::
	UnbindFromAttributes()
{
	if (!ObservedAttributes)
	{
		return;
	}

	ObservedAttributes->OnHealthChanged.RemoveDynamic(
		this,
		&UAshenEnemyHealthWidget::
		HandleHealthChanged
	);

	ObservedAttributes->OnDeath.RemoveDynamic(
		this,
		&UAshenEnemyHealthWidget::
		HandleObservedEnemyDeath
	);

	ObservedAttributes = nullptr;
}

void UAshenEnemyHealthWidget::
	HandleHealthChanged(
		float NewValue,
		float MaxValue,
		float Delta
	)
{
	RefreshHealthBar();
}

void UAshenEnemyHealthWidget::
	HandleObservedEnemyDeath()
{
	SetVisibility(
		ESlateVisibility::Collapsed
	);
}

void UAshenEnemyHealthWidget::RefreshHealthBar()
{
	AAshenTrainingEnemy* Enemy =
		ObservedEnemy.Get();

	if (!IsValid(Enemy) ||
		!ObservedAttributes ||
		!HealthBar ||
		Enemy->IsDead())
	{
		SetVisibility(
			ESlateVisibility::Collapsed
		);

		return;
	}

	const float MaximumHealth =
		FMath::Max(
			ObservedAttributes->GetMaxHealth(),
			KINDA_SMALL_NUMBER
		);

	const float HealthPercent =
		FMath::Clamp(
			ObservedAttributes->GetHealth() /
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