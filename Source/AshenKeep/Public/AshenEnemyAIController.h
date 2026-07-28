#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TimerManager.h"
#include "AshenEnemyAIController.generated.h"

class UPawnSensingComponent;
class AAshenPlayerCharacter;
class AAshenTrainingEnemy;

UCLASS()
class ASHENKEEP_API AAshenEnemyAIController
	: public AAIController
{
	GENERATED_BODY()

public:
	AAshenEnemyAIController();

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(
		APawn* InPawn
	) override;

	virtual void OnUnPossess() override;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|AI"
	)
	TObjectPtr<UPawnSensingComponent> PawnSensing;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|AI",
		meta = (ClampMin = "0.05")
	)
	float ThinkInterval = 0.15f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|AI",
		meta = (ClampMin = "0.0")
	)
	float LoseTargetDistance = 2200.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|AI",
		meta = (ClampMin = "0.0")
	)
	float MoveAcceptanceRadius = 110.0f;

private:
	UFUNCTION()
	void HandleSeePawn(APawn* SeenPawn);

	void UpdateAI();
	void ClearTarget();

	TWeakObjectPtr<AAshenPlayerCharacter> TargetPlayer;

	FTimerHandle ThinkTimerHandle;
};