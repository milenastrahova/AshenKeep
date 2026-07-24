#include "AshenAttributeComponent.h"

#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

UAshenAttributeComponent::UAshenAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UAshenAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* OwnerActor = GetOwner();

	if (OwnerActor && OwnerActor->HasAuthority())
	{
		Health = MaxHealth;
		Stamina = MaxStamina;
		Mana = MaxMana;
	}
}

void UAshenAttributeComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAshenAttributeComponent, MaxHealth);
	DOREPLIFETIME(UAshenAttributeComponent, Health);

	DOREPLIFETIME(UAshenAttributeComponent, MaxStamina);
	DOREPLIFETIME(UAshenAttributeComponent, Stamina);

	DOREPLIFETIME(UAshenAttributeComponent, MaxMana);
	DOREPLIFETIME(UAshenAttributeComponent, Mana);
}

float UAshenAttributeComponent::ApplyDamage(float DamageAmount)
{
	AActor* OwnerActor = GetOwner();

	if (!OwnerActor ||
		!OwnerActor->HasAuthority() ||
		DamageAmount <= 0.0f ||
		!IsAlive())
	{
		return 0.0f;
	}

	const float OldHealth = Health;

	Health = FMath::Clamp(
		Health - DamageAmount,
		0.0f,
		MaxHealth
	);

	BroadcastHealthChanged(OldHealth);

	const float AppliedDamage = OldHealth - Health;

	if (OldHealth > 0.0f && Health <= 0.0f)
	{
		OnDeath.Broadcast();
	}

	return AppliedDamage;
}

float UAshenAttributeComponent::RestoreHealth(float Amount)
{
	AActor* OwnerActor = GetOwner();

	if (!OwnerActor ||
		!OwnerActor->HasAuthority() ||
		Amount <= 0.0f ||
		!IsAlive())
	{
		return 0.0f;
	}

	const float OldHealth = Health;

	Health = FMath::Clamp(
		Health + Amount,
		0.0f,
		MaxHealth
	);

	BroadcastHealthChanged(OldHealth);

	return Health - OldHealth;
}

bool UAshenAttributeComponent::ConsumeStamina(float Amount)
{
	AActor* OwnerActor = GetOwner();

	if (!OwnerActor ||
		!OwnerActor->HasAuthority() ||
		Amount <= 0.0f ||
		!HasEnoughStamina(Amount))
	{
		return false;
	}

	const float OldStamina = Stamina;

	Stamina = FMath::Clamp(
		Stamina - Amount,
		0.0f,
		MaxStamina
	);

	BroadcastStaminaChanged(OldStamina);

	return true;
}

float UAshenAttributeComponent::RestoreStamina(float Amount)
{
	AActor* OwnerActor = GetOwner();

	if (!OwnerActor ||
		!OwnerActor->HasAuthority() ||
		Amount <= 0.0f)
	{
		return 0.0f;
	}

	const float OldStamina = Stamina;

	Stamina = FMath::Clamp(
		Stamina + Amount,
		0.0f,
		MaxStamina
	);

	BroadcastStaminaChanged(OldStamina);

	return Stamina - OldStamina;
}

bool UAshenAttributeComponent::ConsumeMana(float Amount)
{
	AActor* OwnerActor = GetOwner();

	if (!OwnerActor ||
		!OwnerActor->HasAuthority() ||
		Amount <= 0.0f ||
		!HasEnoughMana(Amount))
	{
		return false;
	}

	const float OldMana = Mana;

	Mana = FMath::Clamp(
		Mana - Amount,
		0.0f,
		MaxMana
	);

	BroadcastManaChanged(OldMana);

	return true;
}

float UAshenAttributeComponent::RestoreMana(float Amount)
{
	AActor* OwnerActor = GetOwner();

	if (!OwnerActor ||
		!OwnerActor->HasAuthority() ||
		Amount <= 0.0f)
	{
		return 0.0f;
	}

	const float OldMana = Mana;

	Mana = FMath::Clamp(
		Mana + Amount,
		0.0f,
		MaxMana
	);

	BroadcastManaChanged(OldMana);

	return Mana - OldMana;
}

bool UAshenAttributeComponent::IsAlive() const
{
	return Health > 0.0f;
}

bool UAshenAttributeComponent::HasEnoughStamina(float Amount) const
{
	return Amount >= 0.0f && Stamina >= Amount;
}

bool UAshenAttributeComponent::HasEnoughMana(float Amount) const
{
	return Amount >= 0.0f && Mana >= Amount;
}

void UAshenAttributeComponent::OnRep_Health(float OldHealth)
{
	BroadcastHealthChanged(OldHealth);

	if (OldHealth > 0.0f && Health <= 0.0f)
	{
		OnDeath.Broadcast();
	}
}

void UAshenAttributeComponent::OnRep_Stamina(float OldStamina)
{
	BroadcastStaminaChanged(OldStamina);
}

void UAshenAttributeComponent::OnRep_Mana(float OldMana)
{
	BroadcastManaChanged(OldMana);
}

void UAshenAttributeComponent::BroadcastHealthChanged(float OldHealth)
{
	OnHealthChanged.Broadcast(
		Health,
		MaxHealth,
		Health - OldHealth
	);
}

void UAshenAttributeComponent::BroadcastStaminaChanged(float OldStamina)
{
	OnStaminaChanged.Broadcast(
		Stamina,
		MaxStamina,
		Stamina - OldStamina
	);
}

void UAshenAttributeComponent::BroadcastManaChanged(float OldMana)
{
	OnManaChanged.Broadcast(
		Mana,
		MaxMana,
		Mana - OldMana
	);
}
