#include "AshenTrainingEnemy.h"

#include "AshenAttributeComponent.h"
#include "AshenEnemyAIController.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

AAshenTrainingEnemy::AAshenTrainingEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	AIControllerClass =
		AAshenEnemyAIController::StaticClass();

	AutoPossessAI =
		EAutoPossessAI::PlacedInWorldOrSpawned;

	AttributeComponent =
		CreateDefaultSubobject<
		UAshenAttributeComponent
		>(
			TEXT("AttributeComponent")
		);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->
		bOrientRotationToMovement = true;

	GetCharacterMovement()->RotationRate =
		FRotator(0.0f, 600.0f, 0.0f);
}

void AAshenTrainingEnemy::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed =
		ChaseSpeed;

	if (AttributeComponent)
	{
		AttributeComponent->OnDeath.AddDynamic(
			this,
			&AAshenTrainingEnemy::HandleDeath
		);
	}
}

void AAshenTrainingEnemy::
GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>&
	OutLifetimeProps
) const
{
	Super::GetLifetimeReplicatedProps(
		OutLifetimeProps
	);

	DOREPLIFETIME(
		AAshenTrainingEnemy,
		bIsDead
	);
}

bool AAshenTrainingEnemy::TryAttack(
	AActor* TargetActor
)
{
	if (!HasAuthority() ||
		bIsDead ||
		!bCanAttack ||
		!IsValid(TargetActor) ||
		TargetActor == this)
	{
		return false;
	}

	UAshenAttributeComponent*
		TargetAttributeComponent =
		TargetActor->FindComponentByClass<
		UAshenAttributeComponent
		>();

	if (!TargetAttributeComponent ||
		!TargetAttributeComponent->IsAlive())
	{
		return false;
	}

	const float MaximumDistance =
		AttackRange + AttackRadius;

	const float DistanceSquared =
		FVector::DistSquared2D(
			GetActorLocation(),
			TargetActor->GetActorLocation()
		);

	if (DistanceSquared >
		FMath::Square(MaximumDistance))
	{
		return false;
	}

	FVector AttackDirection =
		TargetActor->GetActorLocation() -
		GetActorLocation();

	AttackDirection.Z = 0.0f;

	if (!AttackDirection.Normalize())
	{
		return false;
	}

	const FRotator AttackRotation =
		AttackDirection.Rotation();

	SetActorRotation(
		FRotator(
			0.0f,
			AttackRotation.Yaw,
			0.0f
		)
	);

	bCanAttack = false;

	GetWorldTimerManager().SetTimer(
		AttackCooldownTimerHandle,
		this,
		&AAshenTrainingEnemy::
		ResetAttackCooldown,
		AttackCooldown,
		false
	);

	MulticastPlayAttackCue();

	const float AppliedDamage =
		TargetAttributeComponent->ApplyDamage(
			AttackDamage
		);

	if (bDrawAttackDebug)
	{
		const FVector DebugLocation =
			GetActorLocation() +
			FVector(0.0f, 0.0f, 55.0f) +
			AttackDirection * AttackRange;

		DrawDebugSphere(
			GetWorld(),
			DebugLocation,
			AttackRadius,
			18,
			AppliedDamage > 0.0f
			? FColor::Red
			: FColor::Green,
			false,
			0.5f,
			0,
			2.0f
		);
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"%s attacked %s for %.1f damage."
		),
		*GetName(),
		*TargetActor->GetName(),
		AppliedDamage
	);

	return AppliedDamage > 0.0f;
}

void AAshenTrainingEnemy::
ResetAttackCooldown()
{
	bCanAttack = true;
}

void AAshenTrainingEnemy::
MulticastPlayAttackCue_Implementation()
{
	BP_OnAttack();
}

void AAshenTrainingEnemy::HandleDeath()
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	bIsDead = true;
	bCanAttack = false;

	GetWorldTimerManager().ClearTimer(
		AttackCooldownTimerHandle
	);

	ApplyDeathState();
	ForceNetUpdate();
}

void AAshenTrainingEnemy::OnRep_IsDead()
{
	if (bIsDead)
	{
		ApplyDeathState();
	}
}

void AAshenTrainingEnemy::ApplyDeathState()
{
	if (AAIController* EnemyController =
		Cast<AAIController>(Controller))
	{
		EnemyController->StopMovement();

		EnemyController->ClearFocus(
			EAIFocusPriority::Gameplay
		);
	}

	if (UCharacterMovementComponent*
		MovementComponent =
		GetCharacterMovement())
	{
		MovementComponent->
			StopMovementImmediately();

		MovementComponent->
			DisableMovement();
	}

	if (UCapsuleComponent* Capsule =
		GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(
			ECollisionEnabled::NoCollision
		);
	}

	USkeletalMeshComponent* EnemyMesh =
		GetMesh();

	if (!EnemyMesh)
	{
		return;
	}

	EnemyMesh->SetCollisionProfileName(
		TEXT("Ragdoll")
	);

	EnemyMesh->SetCollisionEnabled(
		ECollisionEnabled::QueryAndPhysics
	);

	EnemyMesh->SetAllBodiesSimulatePhysics(
		true
	);

	EnemyMesh->SetSimulatePhysics(true);
	EnemyMesh->WakeAllRigidBodies();

	if (HasAuthority() &&
		DeathImpulse > 0.0f)
	{
		const FVector ImpulseDirection =
			-GetActorForwardVector() +
			FVector::UpVector * 0.35f;

		EnemyMesh->AddImpulse(
			ImpulseDirection.GetSafeNormal() *
			DeathImpulse,
			NAME_None,
			true
		);
	}
}