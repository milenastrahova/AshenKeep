#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AshenTargetingMathLibrary.generated.h"

/**
 * Reusable targeting math shared by AI perception and player lock-on.
 * The functions are deterministic and covered by automation tests.
 */
UCLASS()
class ASHENKEEP_API UAshenTargetingMathLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Math|Targeting"
	)
	static float CalculateDirectionDot2D(
		const FVector& Forward,
		const FVector& DirectionToTarget
	);

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Math|Targeting"
	)
	static bool IsInsideVisionCone2D(
		const FVector& Forward,
		const FVector& DirectionToTarget,
		float HalfAngleDegrees
	);

	UFUNCTION(
		BlueprintPure,
		Category = "Ashen Keep|Math|Targeting"
	)
	static float CalculateTargetScore(
		float DirectionDot,
		float Distance,
		float MaximumDistance,
		float AngleWeight = 2.0f
	);
};