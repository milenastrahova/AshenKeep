#include "AshenLockOnComponent.h"

#include "AshenAttributeComponent.h"
#include "AshenTrainingEnemy.h"

#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UAshenLockOnComponent::UAshenLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UAshenLockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(false);
}

void UAshenLockOnComponent::ToggleLockOn()
{
	ACharacter* OwnerCharacter =
		GetOwnerCharacter();

	if (!OwnerCharacter ||
		!OwnerCharacter->IsLocallyControlled())
	{
		return;
	}

	if (CurrentTarget.IsValid())
	{
		ClearLockOn();
		return;
	}

	AAshenTrainingEnemy* NewTarget =
		FindBestTarget();

	if (NewTarget)
	{
		SetLockOnTarget(NewTarget);
	}
}

void UAshenLockOnComponent::ClearLockOn()
{
	const bool bWasLocked =
		CurrentTarget.IsValid() ||
		IsComponentTickEnabled();

	ACharacter* OwnerCharacter =
		GetOwnerCharacter();

	if (OwnerCharacter)
	{
		UCharacterMovementComponent* Movement =
			OwnerCharacter->GetCharacterMovement();

		if (Movement)
		{
			Movement->bOrientRotationToMovement =
				bPreviousOrientRotationToMovement;
		}
	}

	CurrentTarget.Reset();
	SetComponentTickEnabled(false);

	if (bWasLocked)
	{
		BP_OnLockOnChanged(
			nullptr,
			false
		);
	}
}

void UAshenLockOnComponent::SetLockOnTarget(
	AAshenTrainingEnemy* NewTarget
)
{
	if (!IsTargetUsable(NewTarget))
	{
		return;
	}

	ACharacter* OwnerCharacter =
		GetOwnerCharacter();

	if (!OwnerCharacter)
	{
		return;
	}

	UCharacterMovementComponent* Movement =
		OwnerCharacter->GetCharacterMovement();

	if (Movement)
	{
		bPreviousOrientRotationToMovement =
			Movement->bOrientRotationToMovement;

		Movement->bOrientRotationToMovement = false;
	}

	CurrentTarget = NewTarget;

	SetComponentTickEnabled(true);

	BP_OnLockOnChanged(
		NewTarget,
		true
	);
}

AAshenTrainingEnemy*
UAshenLockOnComponent::FindBestTarget() const
{
	const ACharacter* OwnerCharacter =
		GetOwnerCharacter();

	const APlayerController* PlayerController =
		GetOwnerPlayerController();

	if (!OwnerCharacter || !PlayerController)
	{
		return nullptr;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	PlayerController->GetPlayerViewPoint(
		ViewLocation,
		ViewRotation
	);

	const FVector ViewForward =
		ViewRotation.Vector().GetSafeNormal();

	const float MinimumDot =
		FMath::Cos(
			FMath::DegreesToRadians(
				MaxLockAngleDegrees
			)
		);

	TArray<AActor*> FoundEnemies;

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AAshenTrainingEnemy::StaticClass(),
		FoundEnemies
	);

	AAshenTrainingEnemy* BestTarget = nullptr;
	float BestScore = TNumericLimits<float>::Max();

	for (AActor* FoundActor : FoundEnemies)
	{
		AAshenTrainingEnemy* Candidate =
			Cast<AAshenTrainingEnemy>(
				FoundActor
			);

		if (!IsTargetUsable(Candidate))
		{
			continue;
		}

		const FVector TargetLocation =
			Candidate->GetActorLocation() +
			FVector(
				0.0f,
				0.0f,
				TargetHeightOffset
			);

		const FVector DirectionToTarget =
			(
				TargetLocation -
				ViewLocation
				).GetSafeNormal();

		const float DirectionDot =
			FVector::DotProduct(
				ViewForward,
				DirectionToTarget
			);

		if (DirectionDot < MinimumDot)
		{
			continue;
		}

		if (!PlayerController->LineOfSightTo(
			Candidate,
			ViewLocation,
			true
		))
		{
			continue;
		}

		const float Distance =
			FVector::Distance(
				OwnerCharacter->GetActorLocation(),
				Candidate->GetActorLocation()
			);

		const float DistanceScore =
			Distance / MaxLockDistance;

		const float AngleScore =
			1.0f - DirectionDot;

		const float TotalScore =
			AngleScore * 2.0f +
			DistanceScore;

		if (TotalScore < BestScore)
		{
			BestScore = TotalScore;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

bool UAshenLockOnComponent::IsTargetUsable(
	AAshenTrainingEnemy* Target
) const
{
	if (!IsValid(Target))
	{
		return false;
	}

	const ACharacter* OwnerCharacter =
		GetOwnerCharacter();

	if (!OwnerCharacter)
	{
		return false;
	}

	const float DistanceSquared =
		FVector::DistSquared(
			OwnerCharacter->GetActorLocation(),
			Target->GetActorLocation()
		);

	if (DistanceSquared >
		FMath::Square(MaxLockDistance))
	{
		return false;
	}

	const UAshenAttributeComponent* Attributes =
		Target->FindComponentByClass<
		UAshenAttributeComponent
		>();

	return Attributes &&
		Attributes->IsAlive();
}

void UAshenLockOnComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(
		DeltaTime,
		TickType,
		ThisTickFunction
	);

	AAshenTrainingEnemy* Target =
		CurrentTarget.Get();

	if (!IsTargetUsable(Target))
	{
		ClearLockOn();
		return;
	}

	ACharacter* OwnerCharacter =
		GetOwnerCharacter();

	APlayerController* PlayerController =
		GetOwnerPlayerController();

	if (!OwnerCharacter || !PlayerController)
	{
		ClearLockOn();
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	PlayerController->GetPlayerViewPoint(
		ViewLocation,
		ViewRotation
	);

	const FVector TargetLocation =
		Target->GetActorLocation() +
		FVector(
			0.0f,
			0.0f,
			TargetHeightOffset
		);

	FRotator DesiredControlRotation =
		(
			TargetLocation -
			ViewLocation
			).Rotation();

	DesiredControlRotation.Roll = 0.0f;

	DesiredControlRotation.Pitch =
		FMath::ClampAngle(
			DesiredControlRotation.Pitch,
			-45.0f,
			35.0f
		);

	const FRotator NewControlRotation =
		FMath::RInterpTo(
			PlayerController->GetControlRotation(),
			DesiredControlRotation,
			DeltaTime,
			CameraRotationSpeed
		);

	PlayerController->SetControlRotation(
		NewControlRotation
	);

	FVector DirectionToTarget =
		Target->GetActorLocation() -
		OwnerCharacter->GetActorLocation();

	DirectionToTarget.Z = 0.0f;

	if (!DirectionToTarget.IsNearlyZero())
	{
		FRotator DesiredCharacterRotation =
			DirectionToTarget.Rotation();

		DesiredCharacterRotation.Pitch = 0.0f;
		DesiredCharacterRotation.Roll = 0.0f;

		const FRotator NewCharacterRotation =
			FMath::RInterpTo(
				OwnerCharacter->GetActorRotation(),
				DesiredCharacterRotation,
				DeltaTime,
				CharacterRotationSpeed
			);

		OwnerCharacter->SetActorRotation(
			NewCharacterRotation
		);
	}

	if (bDrawDebug && GetWorld())
	{
		DrawDebugSphere(
			GetWorld(),
			TargetLocation,
			45.0f,
			16,
			FColor::Red,
			false,
			0.0f,
			0,
			2.5f
		);
	}
}

ACharacter*
UAshenLockOnComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(
		GetOwner()
	);
}

APlayerController*
UAshenLockOnComponent::GetOwnerPlayerController() const
{
	const ACharacter* OwnerCharacter =
		GetOwnerCharacter();

	if (!OwnerCharacter)
	{
		return nullptr;
	}

	return Cast<APlayerController>(
		OwnerCharacter->GetController()
	);
}