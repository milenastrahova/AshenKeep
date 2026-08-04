#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AshenEnemyAIController.generated.h"

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

private:
	void StartThinkLoop();
	void UpdateAI();
	void ClearTarget();

	AAshenPlayerCharacter*
		FindBestVisiblePlayer() const;

	bool IsPlayerValid(
		AAshenPlayerCharacter* Player
	) const;

	bool CanAcquirePlayer(
		AAshenPlayerCharacter* Player
	) const;

	bool HasClearSightToPlayer(
		AAshenPlayerCharacter* Player
	) const;

	bool IsInsideChaseArea(
		AAshenPlayerCharacter* Player
	) const;

	AAshenTrainingEnemy*
		GetControlledEnemy() const;

	/*
	 * Максимальная дистанция, на которой
	 * враг способен впервые заметить игрока.
	 */
	UPROPERTY(
		EditDefaultsOnly,
		Category = "Ashen Keep|AI|Sight",
		meta = (ClampMin = "100.0")
	)
	float SightRadius = 1300.0f;

	/*
	 * На близкой дистанции враг замечает
	 * игрока независимо от направления,
	 * но только при прямой видимости.
	 */
	UPROPERTY(
		EditDefaultsOnly,
		Category = "Ashen Keep|AI|Sight",
		meta = (ClampMin = "0.0")
	)
	float CloseAwarenessRadius = 750.0f;

	/*
	 * Половина угла зрения.
	 * 80 означает общий конус 160 градусов.
	 */
	UPROPERTY(
		EditDefaultsOnly,
		Category = "Ashen Keep|AI|Sight",
		meta = (
			ClampMin = "1.0",
			ClampMax = "180.0"
			)
	)
	float PeripheralVisionHalfAngleDegrees =
		80.0f;

	/*
	 * При первом обнаружении враг не должен
	 * замечать игрока на другом этаже.
	 */
	UPROPERTY(
		EditDefaultsOnly,
		Category = "Ashen Keep|AI|Sight",
		meta = (ClampMin = "0.0")
	)
	float AcquisitionVerticalTolerance =
		260.0f;

	/*
	 * После обнаружения враг может немного
	 * сопровождать игрока по лестнице.
	 */
	UPROPERTY(
		EditDefaultsOnly,
		Category = "Ashen Keep|AI|Sight",
		meta = (ClampMin = "0.0")
	)
	float ChaseVerticalTolerance =
		700.0f;

	/*
	 * Враг не преследует игрока через
	 * абсолютно всё подземелье.
	 */
	UPROPERTY(
		EditDefaultsOnly,
		Category = "Ashen Keep|AI|Chase",
		meta = (ClampMin = "100.0")
	)
	float MaxChaseDistanceFromHome =
		2000.0f;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Ashen Keep|AI|Memory",
		meta = (ClampMin = "0.0")
	)
	float LoseSightGracePeriod = 1.8f;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Ashen Keep|AI",
		meta = (ClampMin = "0.05")
	)
	float ThinkInterval = 0.15f;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Ashen Keep|AI",
		meta = (ClampMin = "0.0")
	)
	float MoveAcceptanceRadius = 80.0f;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Ashen Keep|AI|Debug"
	)
	bool bDrawVisionDebug = false;

	TWeakObjectPtr<AAshenPlayerCharacter>
		CurrentTarget;

	FVector HomeLocation =
		FVector::ZeroVector;

	FVector LastSeenLocation =
		FVector::ZeroVector;

	float LastSeenTime = -10000.0f;

	FTimerHandle ThinkTimerHandle;
};