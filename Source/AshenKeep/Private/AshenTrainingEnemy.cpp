#include "AshenTrainingEnemy.h"

#include "AshenAttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

AAshenTrainingEnemy::AAshenTrainingEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	AttributeComponent =
		CreateDefaultSubobject<UAshenAttributeComponent>(
			TEXT("AttributeComponent")
		);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	GetCharacterMovement()->RotationRate =
		FRotator(0.0f, 500.0f, 0.0f);
}

void AAshenTrainingEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (AttributeComponent)
	{
		AttributeComponent->OnDeath.AddDynamic(
			this,
			&AAshenTrainingEnemy::HandleDeath
		);
	}
}

void AAshenTrainingEnemy::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
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

void AAshenTrainingEnemy::HandleDeath()
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	bIsDead = true;

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
	if (UCharacterMovementComponent* MovementComponent =
		GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
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

	EnemyMesh->SetAllBodiesSimulatePhysics(true);
	EnemyMesh->SetSimulatePhysics(true);
	EnemyMesh->WakeAllRigidBodies();

	if (HasAuthority() && DeathImpulse > 0.0f)
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