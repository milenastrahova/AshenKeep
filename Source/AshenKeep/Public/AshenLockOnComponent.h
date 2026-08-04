#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AshenLockOnComponent.generated.h"

class ACharacter;
class APlayerController;
class AAshenTrainingEnemy;

UCLASS(
	ClassGroup = (AshenKeep),
	meta = (BlueprintSpawnableComponent)
)
class ASHENKEEP_API UAshenLockOnComponent
	: public UActorComponent
{
	GENERATED_BODY()

public:
	UAshenLockOnComponent();

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	UFUNCTION(
		BlueprintCallable,
		Category = "Ashen Keep|Lock On"
	)
	void ToggleLockOn();

	UFUNCTION(
		BlueprintCallable,
		Category = "Ashen Keep|Lock On"
	)
	void ClearLockOn();

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Lock On"
	)
	bool IsLockedOn() const
	{
		return CurrentTarget.IsValid();
	}

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Lock On"
	)
	AAshenTrainingEnemy* GetCurrentTarget() const
	{
		return CurrentTarget.Get();
	}

protected:
	virtual void BeginPlay() override;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Lock On",
		meta = (ClampMin = "100.0")
	)
	float MaxLockDistance = 1200.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Lock On",
		meta = (ClampMin = "10.0", ClampMax = "180.0")
	)
	float MaxLockAngleDegrees = 75.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Lock On",
		meta = (ClampMin = "0.1")
	)
	float CameraRotationSpeed = 9.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Lock On",
		meta = (ClampMin = "0.1")
	)
	float CharacterRotationSpeed = 12.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Lock On"
	)
	float TargetHeightOffset = 65.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Lock On"
	)
	bool bDrawDebug = true;

	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Ashen Keep|Lock On",
		meta = (DisplayName = "On Lock On Changed")
	)
	void BP_OnLockOnChanged(
		AAshenTrainingEnemy* Target,
		bool bLockedOn
	);

private:
	AAshenTrainingEnemy* FindBestTarget() const;

	bool IsTargetUsable(
		AAshenTrainingEnemy* Target
	) const;

	void SetLockOnTarget(
		AAshenTrainingEnemy* NewTarget
	);

	ACharacter* GetOwnerCharacter() const;

	APlayerController* GetOwnerPlayerController() const;

	TWeakObjectPtr<AAshenTrainingEnemy> CurrentTarget;

	bool bPreviousOrientRotationToMovement = true;
};