#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AshenDamageVolume.generated.h"

class UBoxComponent;
class UPrimitiveComponent;

UCLASS()
class ASHENKEEP_API AAshenDamageVolume : public AActor
{
	GENERATED_BODY()

public:
	AAshenDamageVolume();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Damage"
	)
	TObjectPtr<UBoxComponent> DamageBox;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Ashen Keep|Damage",
		meta = (ClampMin = "0.0")
	)
	float DamageAmount = 25.0f;

private:
	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};