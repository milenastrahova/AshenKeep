#include "AshenEnemyAIController.h"

#include "AshenPlayerCharacter.h"
#include "AshenTrainingEnemy.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/PawnSensingComponent.h"

AAshenEnemyAIController::AAshenEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = false;

	PawnSensing =
		CreateDefaultSubobject<UPawnSensingComponent>(
			TEXT("PawnSensing")
		);

	if (PawnSensing)
	{
		PawnSensing->SightRadius = 1600.0f;
		PawnSensing->bOnlySensePlayers = true;
		PawnSensing->bSeePawns = true;
		PawnSensing->bHearNoises = false;

		// В конструкторе записываем значение напрямую.
		// Не вызываем здесь функции, работающие с таймером.
		PawnSensing->SensingInterval = 0.2f;
	}
}

void AAshenEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	if (PawnSensing)
	{
		// Здесь мир уже создан, поэтому вызов безопасен.
		PawnSensing->SetPeripheralVisionAngle(
			70.0f
		);

		PawnSensing->OnSeePawn.AddDynamic(
			this,
			&AAshenEnemyAIController::HandleSeePawn
		);
	}
}

void AAshenEnemyAIController::OnPossess(
	APawn* InPawn
)
{
	Super::OnPossess(InPawn);

	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		ThinkTimerHandle,
		this,
		&AAshenEnemyAIController::UpdateAI,
		ThinkInterval,
		true,
		0.1f
	);
}

void AAshenEnemyAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(
		ThinkTimerHandle
	);

	ClearTarget();

	Super::OnUnPossess();
}

void AAshenEnemyAIController::HandleSeePawn(
	APawn* SeenPawn
)
{
	AAshenPlayerCharacter* SeenPlayer =
		Cast<AAshenPlayerCharacter>(
			SeenPawn
		);

	if (!IsValid(SeenPlayer) ||
		SeenPlayer->IsDead())
	{
		return;
	}

	TargetPlayer = SeenPlayer;
}

void AAshenEnemyAIController::UpdateAI()
{
	AAshenTrainingEnemy* Enemy =
		Cast<AAshenTrainingEnemy>(
			GetPawn()
		);

	if (!IsValid(Enemy) ||
		Enemy->IsDead())
	{
		ClearTarget();
		return;
	}

	AAshenPlayerCharacter* Player =
		TargetPlayer.Get();

	if (!IsValid(Player) ||
		Player->IsDead())
	{
		ClearTarget();
		return;
	}

	const float DistanceToPlayer =
		FVector::Dist2D(
			Enemy->GetActorLocation(),
			Player->GetActorLocation()
		);

	if (DistanceToPlayer > LoseTargetDistance)
	{
		ClearTarget();
		return;
	}

	SetFocus(Player);

	const bool bCanAttack =
		DistanceToPlayer <=
		Enemy->GetAttackRange() &&
		LineOfSightTo(Player);

	if (bCanAttack)
	{
		StopMovement();
		Enemy->TryAttack(Player);
		return;
	}

	MoveToActor(
		Player,
		MoveAcceptanceRadius,
		true,
		true,
		false,
		nullptr,
		true
	);
}

void AAshenEnemyAIController::ClearTarget()
{
	TargetPlayer.Reset();

	StopMovement();
	ClearFocus(
		EAIFocusPriority::Gameplay
	);
}