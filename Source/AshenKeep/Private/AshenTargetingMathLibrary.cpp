#include "AshenTargetingMathLibrary.h"

float UAshenTargetingMathLibrary::
	CalculateDirectionDot2D(
		const FVector& Forward,
		const FVector& DirectionToTarget
	)
{
	FVector SafeForward = Forward;
	SafeForward.Z = 0.0f;

	FVector SafeDirection = DirectionToTarget;
	SafeDirection.Z = 0.0f;

	if (!SafeForward.Normalize() ||
		!SafeDirection.Normalize())
	{
		return -1.0f;
	}

	return FMath::Clamp(
		FVector::DotProduct(
			SafeForward,
			SafeDirection
		),
		-1.0f,
		1.0f
	);
}

bool UAshenTargetingMathLibrary::
	IsInsideVisionCone2D(
		const FVector& Forward,
		const FVector& DirectionToTarget,
		float HalfAngleDegrees
	)
{
	const float SafeHalfAngle =
		FMath::Clamp(
			HalfAngleDegrees,
			0.0f,
			180.0f
		);

	const float MinimumDot =
		FMath::Cos(
			FMath::DegreesToRadians(
				SafeHalfAngle
			)
		);

	const float DirectionDot =
		CalculateDirectionDot2D(
			Forward,
			DirectionToTarget
		);

	return DirectionDot >= MinimumDot;
}

float UAshenTargetingMathLibrary::
	CalculateTargetScore(
		float DirectionDot,
		float Distance,
		float MaximumDistance,
		float AngleWeight
	)
{
	if (MaximumDistance <=
		KINDA_SMALL_NUMBER)
	{
		return TNumericLimits<float>::Max();
	}

	const float SafeDot =
		FMath::Clamp(
			DirectionDot,
			-1.0f,
			1.0f
		);

	const float SafeDistance =
		FMath::Max(
			0.0f,
			Distance
		);

	const float SafeAngleWeight =
		FMath::Max(
			0.0f,
			AngleWeight
		);

	const float AngleScore =
		1.0f - SafeDot;

	const float DistanceScore =
		FMath::Clamp(
			SafeDistance /
				MaximumDistance,
			0.0f,
			1.0f
		);

	return AngleScore *
		SafeAngleWeight +
		DistanceScore;
}