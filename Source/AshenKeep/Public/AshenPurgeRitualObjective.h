#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AshenPurgeRitualObjective.generated.h"

class USceneComponent;
class UBoxComponent;
class UUserWidget;
class AAshenTrainingEnemy;
class USoundBase;

UCLASS()
class ASHENKEEP_API AAshenPurgeRitualObjective : public AActor
{
	GENERATED_BODY()

public:
	AAshenPurgeRitualObjective();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Objective"
	)
	bool IsRitualCompleted() const
	{
		return bRitualCompleted;
	}

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason
	) override;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Objective"
	)
	TObjectPtr<USceneComponent> SceneRoot;

	/*
	 * Оставляем компонент, чтобы существующий
	 * Blueprint не сломался. Для победы зона
	 * больше не требуется.
	 */
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Objective"
	)
	TObjectPtr<UBoxComponent> CompletionZone;

	UPROPERTY(
		EditInstanceOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Objective"
	)
	TObjectPtr<AAshenTrainingEnemy> Captain;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Objective"
	)
	TSubclassOf<UUserWidget> VictoryWidgetClass;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Objective",
		meta = (ClampMin = "0.1")
	)
	float CompletionCheckInterval = 0.25f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Objective",
		meta = (ClampMin = "100.0")
	)
	float CaptainAutoFindRadius = 3000.0f;


	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Objective|Audio"
	)
	TObjectPtr<USoundBase> VictorySound;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Ashen Keep|Objective|Audio",
		meta = (ClampMin = "0.0")
	)
	float VictorySoundVolume = 0.82f;

private:
	void EvaluateCompletion();
	void FindCaptainIfNeeded();
	void CompleteRitual();
	void ShowVictoryLocally();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShowVictory();

	UFUNCTION()
	void OnRep_RitualCompleted();

	UPROPERTY(
		ReplicatedUsing = OnRep_RitualCompleted,
		VisibleInstanceOnly,
		Category = "Ashen Keep|Objective"
	)
	bool bRitualCompleted = false;

	bool bVictoryShownLocally = false;
	bool bLoggedMissingCaptain = false;

	FTimerHandle CompletionCheckTimerHandle;
};