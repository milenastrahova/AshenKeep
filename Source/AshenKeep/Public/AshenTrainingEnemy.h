#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AshenTrainingEnemy.generated.h"

class UAshenAttributeComponent;

UCLASS()
class ASHENKEEP_API AAshenTrainingEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AAshenTrainingEnemy();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Attributes"
	)
	UAshenAttributeComponent* GetAttributeComponent() const
	{
		return AttributeComponent;
	}

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Combat"
	)
	bool IsDead() const
	{
		return bIsDead;
	}

protected:
	virtual void BeginPlay() override;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Attributes"
	)
	TObjectPtr<UAshenAttributeComponent> AttributeComponent;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Death",
		meta = (ClampMin = "0.0")
	)
	float DeathImpulse = 250.0f;

private:
	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void OnRep_IsDead();

	void ApplyDeathState();

	UPROPERTY(
		ReplicatedUsing = OnRep_IsDead,
		VisibleInstanceOnly,
		Category = "Ashen Keep|Death"
	)
	bool bIsDead = false;
};