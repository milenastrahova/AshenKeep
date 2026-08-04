#include "AshenEnemyAIController.h"

#include "AshenAttributeComponent.h"
#include "AshenPlayerCharacter.h"
#include "AshenTrainingEnemy.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

AAshenEnemyAIController::
AAshenEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAshenEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	if (APawn* ControlledPawn = GetPawn())
	{
		HomeLocation =
			ControlledPawn->GetActorLocation();
	}

	StartThinkLoop();
}

void AAshenEnemyAIController::OnPossess(
	APawn* InPawn
)
{
	Super::OnPossess(InPawn);

	if (!HasAuthority() || !InPawn)
	{
		return;
	}

	HomeLocation =
		InPawn->GetActorLocation();

	StartThinkLoop();
}

void AAshenEnemyAIController::StartThinkLoop()
{
	GetWorldTimerManager().ClearTimer(
		ThinkTimerHandle
	);

	GetWorldTimerManager().SetTimer(
		ThinkTimerHandle,
		this,
		&AAshenEnemyAIController::UpdateAI,
		ThinkInterval,
		true,
		0.15f
	);
}

void AAshenEnemyAIController::UpdateAI()
{
	if (!HasAuthority())
	{
		return;
	}

	AAshenTrainingEnemy* Enemy =
		GetControlledEnemy();

	if (!IsValid(Enemy) ||
		Enemy->IsDead())
	{
		ClearTarget();
		return;
	}

	UAshenAttributeComponent* EnemyAttributes =
		Enemy->GetAttributeComponent();

	if (!EnemyAttributes ||
		!EnemyAttributes->IsAlive())
	{
		ClearTarget();
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		ClearTarget();
		return;
	}

	AAshenPlayerCharacter* Target =
		CurrentTarget.Get();

	bool bTargetVisible = false;

	if (IsPlayerValid(Target) &&
		IsInsideChaseArea(Target))
	{
		const float VerticalDifference =
			FMath::Abs(
				Enemy->GetActorLocation().Z -
				Target->GetActorLocation().Z
			);

		if (VerticalDifference <=
			ChaseVerticalTolerance)
		{
			bTargetVisible =
				HasClearSightToPlayer(Target);

			if (bTargetVisible)
			{
				LastSeenLocation =
					Target->GetActorLocation();

				LastSeenTime =
					World->GetTimeSeconds();
			}
		}
		else
		{
			CurrentTarget.Reset();
			Target = nullptr;
		}
	}
	else
	{
		CurrentTarget.Reset();
		Target = nullptr;
	}

	/*
	 * Новая цель выбирается только тогда,
	 * когда враг действительно способен
	 * её увидеть на своём этаже.
	 */
	if (!Target)
	{
		Target = FindBestVisiblePlayer();

		if (Target)
		{
			CurrentTarget = Target;

			LastSeenLocation =
				Target->GetActorLocation();

			LastSeenTime =
				World->GetTimeSeconds();

			bTargetVisible = true;

			UE_LOG(
				LogTemp,
				Log,
				TEXT(
					"%s detected %s."
				),
				*GetName(),
				*Target->GetName()
			);
		}
	}

	if (!Target)
	{
		StopMovement();
		return;
	}

	/*
	 * Если игрок спрятался, враг короткое
	 * время идёт к последней точке.
	 */
	if (!bTargetVisible)
	{
		const float TimeWithoutSight =
			World->GetTimeSeconds() -
			LastSeenTime;

		if (TimeWithoutSight >
			LoseSightGracePeriod)
		{
			ClearTarget();
			return;
		}

		MoveToLocation(
			LastSeenLocation,
			MoveAcceptanceRadius
		);

		return;
	}

	const float DistanceToTarget =
		FVector::Dist2D(
			Enemy->GetActorLocation(),
			Target->GetActorLocation()
		);

	if (DistanceToTarget <=
		Enemy->GetAttackRange())
	{
		StopMovement();

		Enemy->TryAttack(Target);
		return;
	}

	MoveToActor(
		Target,
		MoveAcceptanceRadius
	);
}

AAshenPlayerCharacter*
AAshenEnemyAIController::
FindBestVisiblePlayer() const
{
	const AAshenTrainingEnemy* Enemy =
		GetControlledEnemy();

	UWorld* World = GetWorld();

	if (!Enemy || !World)
	{
		return nullptr;
	}

	AAshenPlayerCharacter* BestPlayer =
		nullptr;

	float BestDistanceSquared =
		FMath::Square(SightRadius);

	for (
		TActorIterator<AAshenPlayerCharacter>
		Iterator(World);
		Iterator;
		++Iterator
		)
	{
		AAshenPlayerCharacter* Candidate =
			*Iterator;

		if (!CanAcquirePlayer(Candidate))
		{
			continue;
		}

		const float DistanceSquared =
			FVector::DistSquared2D(
				Enemy->GetActorLocation(),
				Candidate->GetActorLocation()
			);

		if (DistanceSquared <
			BestDistanceSquared)
		{
			BestDistanceSquared =
				DistanceSquared;

			BestPlayer = Candidate;
		}
	}

	return BestPlayer;
}

bool AAshenEnemyAIController::CanAcquirePlayer(
	AAshenPlayerCharacter* Player
) const
{
	const AAshenTrainingEnemy* Enemy =
		GetControlledEnemy();

	if (!Enemy ||
		!IsPlayerValid(Player) ||
		!IsInsideChaseArea(Player))
	{
		return false;
	}

	const FVector EnemyLocation =
		Enemy->GetActorLocation();

	const FVector PlayerLocation =
		Player->GetActorLocation();

	const float VerticalDifference =
		FMath::Abs(
			EnemyLocation.Z -
			PlayerLocation.Z
		);

	/*
	 * Не позволяем охотникам обнаруживать
	 * игрока сквозь пол между этажами.
	 */
	if (VerticalDifference >
		AcquisitionVerticalTolerance)
	{
		return false;
	}

	const float DistanceSquared =
		FVector::DistSquared2D(
			EnemyLocation,
			PlayerLocation
		);

	if (DistanceSquared >
		FMath::Square(SightRadius))
	{
		return false;
	}

	if (!HasClearSightToPlayer(Player))
	{
		return false;
	}

	/*
	 * На близкой дистанции враг реагирует
	 * независимо от поворота капсулы.
	 */
	if (DistanceSquared <=
		FMath::Square(
			CloseAwarenessRadius
		))
	{
		return true;
	}

	FVector DirectionToPlayer =
		PlayerLocation -
		EnemyLocation;

	DirectionToPlayer.Z = 0.0f;
	DirectionToPlayer.Normalize();

	FVector EnemyForward =
		Enemy->GetActorForwardVector();

	EnemyForward.Z = 0.0f;
	EnemyForward.Normalize();

	const float MinimumViewDot =
		FMath::Cos(
			FMath::DegreesToRadians(
				PeripheralVisionHalfAngleDegrees
			)
		);

	const float ViewDot =
		FVector::DotProduct(
			EnemyForward,
			DirectionToPlayer
		);

	const bool bInsideVisionCone =
		ViewDot >= MinimumViewDot;

	if (bDrawVisionDebug && GetWorld())
	{
		DrawDebugLine(
			GetWorld(),
			EnemyLocation +
			FVector(0.0f, 0.0f, 60.0f),
			PlayerLocation +
			FVector(0.0f, 0.0f, 60.0f),
			bInsideVisionCone
			? FColor::Green
			: FColor::Blue,
			false,
			ThinkInterval,
			0,
			1.5f
		);
	}

	return bInsideVisionCone;
}

bool AAshenEnemyAIController::
HasClearSightToPlayer(
	AAshenPlayerCharacter* Player
) const
{
	if (!IsPlayerValid(Player))
	{
		return false;
	}

	/*
	 * Проверяет стены, двери и другую
	 * блокирующую геометрию.
	 */
	return LineOfSightTo(
		Player,
		FVector::ZeroVector,
		true
	);
}

bool AAshenEnemyAIController::
IsInsideChaseArea(
	AAshenPlayerCharacter* Player
) const
{
	if (!Player)
	{
		return false;
	}

	const float DistanceFromHomeSquared =
		FVector::DistSquared2D(
			HomeLocation,
			Player->GetActorLocation()
		);

	return DistanceFromHomeSquared <=
		FMath::Square(
			MaxChaseDistanceFromHome
		);
}

bool AAshenEnemyAIController::IsPlayerValid(
	AAshenPlayerCharacter* Player
) const
{
	if (!IsValid(Player) ||
		Player->IsDead())
	{
		return false;
	}

	const UAshenAttributeComponent*
		PlayerAttributes =
		Player->GetAttributeComponent();

	return PlayerAttributes &&
		PlayerAttributes->IsAlive();
}

AAshenTrainingEnemy*
AAshenEnemyAIController::
GetControlledEnemy() const
{
	return Cast<AAshenTrainingEnemy>(
		GetPawn()
	);
}

void AAshenEnemyAIController::ClearTarget()
{
	CurrentTarget.Reset();

	LastSeenLocation =
		FVector::ZeroVector;

	LastSeenTime = -10000.0f;

	StopMovement();
}