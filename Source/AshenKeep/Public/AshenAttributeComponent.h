#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AshenAttributeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnAshenAttributeChanged,
	float, NewValue,
	float, MaxValue,
	float, Delta
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnAshenDeath
);

UCLASS(
	ClassGroup = (AshenKeep),
	meta = (BlueprintSpawnableComponent)
)
class ASHENKEEP_API UAshenAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAshenAttributeComponent();

	/**
	 * Pure deterministic helpers used by runtime code and automation tests.
	 * They do not require a World or Actor authority.
	 */
	static float CalculateValueAfterDamage(
		float CurrentValue,
		float MaximumValue,
		float DamageAmount
	);

	static float CalculateValueAfterRestore(
		float CurrentValue,
		float MaximumValue,
		float RestoreAmount
	);

	static bool CanConsumeResource(
		float CurrentValue,
		float Amount
	);

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;

	UFUNCTION(BlueprintCallable, Category = "Ashen Keep|Attributes")
	float ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Ashen Keep|Attributes")
	float RestoreHealth(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Ashen Keep|Attributes")
	bool ConsumeStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Ashen Keep|Attributes")
	float RestoreStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Ashen Keep|Attributes")
	bool ConsumeMana(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Ashen Keep|Attributes")
	float RestoreMana(float Amount);

	UFUNCTION(BlueprintPure, Category = "Ashen Keep|Attributes")
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category = "Ashen Keep|Attributes")
	bool HasEnoughStamina(float Amount) const;

	UFUNCTION(BlueprintPure, Category = "Ashen Keep|Attributes")
	bool HasEnoughMana(float Amount) const;

	UFUNCTION(BlueprintPure, Category = "Ashen Keep|Attributes")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Ashen Keep|Attributes")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Ashen Keep|Attributes")
	float GetStamina() const { return Stamina; }

	UFUNCTION(BlueprintPure, Category = "Ashen Keep|Attributes")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Ashen Keep|Attributes")
	float GetMana() const { return Mana; }

	UFUNCTION(BlueprintPure, Category = "Ashen Keep|Attributes")
	float GetMaxMana() const { return MaxMana; }

	UPROPERTY(BlueprintAssignable, Category = "Ashen Keep|Attributes")
	FOnAshenAttributeChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Ashen Keep|Attributes")
	FOnAshenAttributeChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Ashen Keep|Attributes")
	FOnAshenAttributeChanged OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Ashen Keep|Attributes")
	FOnAshenDeath OnDeath;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Replicated,
		Category = "Ashen Keep|Attributes",
		meta = (ClampMin = "1.0")
	)
	float MaxHealth = 100.0f;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		ReplicatedUsing = OnRep_Health,
		Category = "Ashen Keep|Attributes"
	)
	float Health = 100.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Replicated,
		Category = "Ashen Keep|Attributes",
		meta = (ClampMin = "1.0")
	)
	float MaxStamina = 100.0f;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		ReplicatedUsing = OnRep_Stamina,
		Category = "Ashen Keep|Attributes"
	)
	float Stamina = 100.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Replicated,
		Category = "Ashen Keep|Attributes",
		meta = (ClampMin = "1.0")
	)
	float MaxMana = 100.0f;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		ReplicatedUsing = OnRep_Mana,
		Category = "Ashen Keep|Attributes"
	)
	float Mana = 100.0f;

private:
	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UFUNCTION()
	void OnRep_Stamina(float OldStamina);

	UFUNCTION()
	void OnRep_Mana(float OldMana);

	void BroadcastHealthChanged(float OldHealth);
	void BroadcastStaminaChanged(float OldStamina);
	void BroadcastManaChanged(float OldMana);
};