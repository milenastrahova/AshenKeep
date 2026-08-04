#include "AshenEnemyHealthWidget.h"

#include "AshenAttributeComponent.h"
#include "AshenTrainingEnemy.h"
#include "Components/ProgressBar.h"

void UAshenEnemyHealthWidget::SetObservedEnemy(
	AAshenTrainingEnemy* NewEnemy
)
{
	if (ObservedEnemy.Get() != NewEnemy)
	{
		UnbindFromAttributes();
		ObservedEnemy = NewEnemy;
	}

	BindToAttributes(
		IsValid(NewEnemy)
			? NewEnemy->GetAttributeComponent()
			: nullptr
	);

	RefreshAccumulator = 0.0f;
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

	RefreshAccumulator = 0.0f;
	RefreshHealthBar();
}

void UAshenEnemyHealthWidget::NativeDestruct()
{
	UnbindFromAttributes();

	Super::NativeDestruct();
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

	RefreshAccumulator +=
		FMath::Max(0.0f, InDeltaTime);

	if (RefreshAccumulator <
		RefreshInterval)
	{
		return;
	}

	RefreshAccumulator = 0.0f;

	AAshenTrainingEnemy* Enemy =
		ObservedEnemy.Get();

	if (IsValid(Enemy) &&
		ObservedAttributes !=
			Enemy->GetAttributeComponent())
	{
		BindToAttributes(
			Enemy->GetAttributeComponent()
		);
	}

	RefreshHealthBar();
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

	ObservedAttributes->
		OnHealthChanged.AddUniqueDynamic(
			this,
			&UAshenEnemyHealthWidget::
				HandleHealthChanged
		);

	ObservedAttributes->
		OnDeath.AddUniqueDynamic(
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

	ObservedAttributes->
		OnHealthChanged.RemoveDynamic(
			this,
			&UAshenEnemyHealthWidget::
				HandleHealthChanged
		);

	ObservedAttributes->
		OnDeath.RemoveDynamic(
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
	UpdateHealthBarFromValues(
		NewValue,
		MaxValue
	);
}

void UAshenEnemyHealthWidget::
	HandleObservedEnemyDeath()
{
	if (HealthBar)
	{
		HealthBar->SetPercent(0.0f);
	}

	SetVisibility(
		ESlateVisibility::Collapsed
	);
}

void UAshenEnemyHealthWidget::
	UpdateHealthBarFromValues(
		float CurrentHealth,
		float MaximumHealth
	)
{
	if (!HealthBar)
	{
		SetVisibility(
			ESlateVisibility::Collapsed
		);

		return;
	}

	const float SafeMaximum =
		FMath::Max(
			MaximumHealth,
			KINDA_SMALL_NUMBER
		);

	const float HealthPercent =
		FMath::Clamp(
			CurrentHealth /
				SafeMaximum,
			0.0f,
			1.0f
		);

	HealthBar->SetPercent(
		HealthPercent
	);

	SetVisibility(
		HealthPercent > 0.0f
			? ESlateVisibility::
				HitTestInvisible
			: ESlateVisibility::
				Collapsed
	);
}

void UAshenEnemyHealthWidget::
	RefreshHealthBar()
{
	AAshenTrainingEnemy* Enemy =
		ObservedEnemy.Get();

	if (!IsValid(Enemy) ||
		Enemy->IsDead())
	{
		if (HealthBar)
		{
			HealthBar->SetPercent(
				0.0f
			);
		}

		SetVisibility(
			ESlateVisibility::Collapsed
		);

		return;
	}

	UAshenAttributeComponent* Attributes =
		Enemy->GetAttributeComponent();

	if (!Attributes)
	{
		SetVisibility(
			ESlateVisibility::Collapsed
		);

		return;
	}

	if (ObservedAttributes != Attributes)
	{
		BindToAttributes(Attributes);
	}

	UpdateHealthBarFromValues(
		Attributes->GetHealth(),
		Attributes->GetMaxHealth()
	);
}