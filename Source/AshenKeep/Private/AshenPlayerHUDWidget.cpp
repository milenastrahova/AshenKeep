#include "AshenPlayerHUDWidget.h"

#include "AshenAttributeComponent.h"
#include "AshenPlayerCharacter.h"
#include "Components/ProgressBar.h"

void UAshenPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToAttributeComponent();
}

void UAshenPlayerHUDWidget::NativeDestruct()
{
	UnbindFromAttributeComponent();

	Super::NativeDestruct();
}

void UAshenPlayerHUDWidget::BindToAttributeComponent()
{
	UnbindFromAttributeComponent();

	AAshenPlayerCharacter* PlayerCharacter =
		Cast<AAshenPlayerCharacter>(GetOwningPlayerPawn());

	if (!PlayerCharacter)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Ashen HUD could not find AshenPlayerCharacter.")
		);

		return;
	}

	AttributeComponent =
		PlayerCharacter->GetAttributeComponent();

	if (!AttributeComponent)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Ashen HUD could not find AttributeComponent.")
		);

		return;
	}

	AttributeComponent->OnHealthChanged.AddUniqueDynamic(
		this,
		&UAshenPlayerHUDWidget::HandleHealthChanged
	);

	AttributeComponent->OnStaminaChanged.AddUniqueDynamic(
		this,
		&UAshenPlayerHUDWidget::HandleStaminaChanged
	);

	AttributeComponent->OnManaChanged.AddUniqueDynamic(
		this,
		&UAshenPlayerHUDWidget::HandleManaChanged
	);

	// Устанавливаем начальные значения сразу после создания HUD.
	HandleHealthChanged(
		AttributeComponent->GetHealth(),
		AttributeComponent->GetMaxHealth(),
		0.0f
	);

	HandleStaminaChanged(
		AttributeComponent->GetStamina(),
		AttributeComponent->GetMaxStamina(),
		0.0f
	);

	HandleManaChanged(
		AttributeComponent->GetMana(),
		AttributeComponent->GetMaxMana(),
		0.0f
	);
}

void UAshenPlayerHUDWidget::UnbindFromAttributeComponent()
{
	if (!AttributeComponent)
	{
		return;
	}

	AttributeComponent->OnHealthChanged.RemoveDynamic(
		this,
		&UAshenPlayerHUDWidget::HandleHealthChanged
	);

	AttributeComponent->OnStaminaChanged.RemoveDynamic(
		this,
		&UAshenPlayerHUDWidget::HandleStaminaChanged
	);

	AttributeComponent->OnManaChanged.RemoveDynamic(
		this,
		&UAshenPlayerHUDWidget::HandleManaChanged
	);

	AttributeComponent = nullptr;
}

void UAshenPlayerHUDWidget::HandleHealthChanged(
	float NewValue,
	float MaxValue,
	float Delta
)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(
			CalculatePercent(NewValue, MaxValue)
		);
	}
}

void UAshenPlayerHUDWidget::HandleStaminaChanged(
	float NewValue,
	float MaxValue,
	float Delta
)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(
			CalculatePercent(NewValue, MaxValue)
		);
	}
}

void UAshenPlayerHUDWidget::HandleManaChanged(
	float NewValue,
	float MaxValue,
	float Delta
)
{
	if (ManaBar)
	{
		ManaBar->SetPercent(
			CalculatePercent(NewValue, MaxValue)
		);
	}
}

float UAshenPlayerHUDWidget::CalculatePercent(
	float CurrentValue,
	float MaxValue
)
{
	if (MaxValue <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(
		CurrentValue / MaxValue,
		0.0f,
		1.0f
	);
}