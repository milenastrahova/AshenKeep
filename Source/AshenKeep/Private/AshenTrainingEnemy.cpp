#include "AshenTrainingEnemy.h"

#include "AshenAttributeComponent.h"
#include "AshenEnemyAIController.h"
#include "AshenEnemyHealthWidget.h"
#include "AshenPlayerCharacter.h"

#include "AIController.h"
#include "Animation/AnimationAsset.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"

AAshenTrainingEnemy::AAshenTrainingEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f;

	bReplicates = true;
	SetReplicateMovement(true);

	AttributeComponent =
		CreateDefaultSubobject<UAshenAttributeComponent>(
			TEXT("AttributeComponent")
		);

	HealthWidgetComponent =
		CreateDefaultSubobject<UWidgetComponent>(
			TEXT("HealthWidgetComponent")
		);

	HealthWidgetComponent->SetupAttachment(
		RootComponent
	);

	HealthWidgetComponent->SetRelativeLocation(
		FVector(0.0f, 0.0f, 120.0f)
	);

	HealthWidgetComponent->SetWidgetSpace(
		EWidgetSpace::Screen
	);

	HealthWidgetComponent->SetDrawSize(
		FVector2D(180.0f, 24.0f)
	);

	HealthWidgetComponent->SetCollisionEnabled(
		ECollisionEnabled::NoCollision
	);

	AIControllerClass =
		AAshenEnemyAIController::StaticClass();

	AutoPossessAI =
		EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCharacterMovement()->
		bOrientRotationToMovement = true;

	GetCharacterMovement()->RotationRate =
		FRotator(0.0f, 540.0f, 0.0f);

	GetCharacterMovement()->MaxWalkSpeed =
		ChaseSpeed;

	static ConstructorHelpers::FObjectFinder<USoundBase>
		AttackSoundFinder(
			TEXT(
				"/Game/Audio/SFX/S_Ashen_EnemyAttack."
				"S_Ashen_EnemyAttack"
			)
		);

	if (AttackSoundFinder.Succeeded())
	{
		AttackSound =
			AttackSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase>
		DeathSoundFinder(
			TEXT(
				"/Game/Audio/SFX/S_Ashen_EnemyDeath."
				"S_Ashen_EnemyDeath"
			)
		);

	if (DeathSoundFinder.Succeeded())
	{
		DeathSound =
			DeathSoundFinder.Object;
	}
}

void AAshenTrainingEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed =
			ChaseSpeed;
	}

	if (AttributeComponent)
	{
		AttributeComponent->OnDeath.AddDynamic(
			this,
			&AAshenTrainingEnemy::HandleDeath
		);
	}

	if (HealthWidgetComponent)
	{
		HealthWidgetComponent->InitWidget();

		UAshenEnemyHealthWidget* HealthWidget =
			Cast<UAshenEnemyHealthWidget>(
				HealthWidgetComponent->
					GetUserWidgetObject()
			);

		if (HealthWidget)
		{
			HealthWidget->SetObservedEnemy(
				this
			);
		}
	}

	if (bUseSimpleAnimationSystem)
	{
		UpdateSimpleAnimation();
	}
}

void AAshenTrainingEnemy::Tick(
	float DeltaSeconds
)
{
	Super::Tick(DeltaSeconds);

	UpdateSimpleAnimation();
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

void AAshenTrainingEnemy::TryAttack(
	AAshenPlayerCharacter* Target
)
{
	if (!HasAuthority() ||
		bIsDead ||
		!bCanAttack ||
		!IsValid(Target) ||
		Target->IsDead())
	{
		return;
	}

	if (Target->IsMistStepping())
	{
		return;
	}

	if (!AttributeComponent ||
		!AttributeComponent->IsAlive())
	{
		return;
	}

	UAshenAttributeComponent* TargetAttributes =
		Target->GetAttributeComponent();

	if (!TargetAttributes ||
		!TargetAttributes->IsAlive())
	{
		return;
	}

	const float DistanceToTarget =
		FVector::Distance(
			GetActorLocation(),
			Target->GetActorLocation()
		);

	if (DistanceToTarget > AttackRange)
	{
		return;
	}

	AAIController* EnemyController =
		Cast<AAIController>(
			GetController()
		);

	if (!EnemyController ||
		!EnemyController->LineOfSightTo(
			Target,
			FVector::ZeroVector,
			true
		))
	{
		return;
	}

	FVector DirectionToTarget =
		Target->GetActorLocation() -
		GetActorLocation();

	DirectionToTarget.Z = 0.0f;

	if (!DirectionToTarget.IsNearlyZero())
	{
		SetActorRotation(
			DirectionToTarget.Rotation()
		);
	}

	bCanAttack = false;

	GetWorldTimerManager().SetTimer(
		AttackCooldownTimerHandle,
		this,
		&AAshenTrainingEnemy::
			ResetAttackCooldown,
		AttackCooldown,
		false
	);

	const float AppliedDamage =
		TargetAttributes->ApplyDamage(
			AttackDamage
		);

	if (AppliedDamage <= 0.0f)
	{
		return;
	}

	MulticastPlayAttackCue();

	if (bDrawAttackDebug && GetWorld())
	{
		DrawDebugSphere(
			GetWorld(),
			Target->GetActorLocation() +
				FVector(0.0f, 0.0f, 50.0f),
			AttackRadius,
			16,
			FColor::Orange,
			false,
			0.6f,
			0,
			2.0f
		);
	}
}

void AAshenTrainingEnemy::
	MulticastPlayAttackCue_Implementation()
{
	PlayAttackAnimationLocally();

	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			AttackSound,
			GetActorLocation(),
			AttackSoundVolume
		);
	}

	/*
	 * Keep the old Blueprint montage hook only when the direct
	 * animation system is disabled. This prevents an incompatible
	 * Manny montage from interrupting the Paragon animation asset.
	 */
	if (!bUseSimpleAnimationSystem)
	{
		BP_OnAttack();
	}
}

void AAshenTrainingEnemy::
	ResetAttackCooldown()
{
	bCanAttack = true;
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

	GetWorldTimerManager().ClearTimer(
		SimpleAttackAnimationTimerHandle
	);

	if (AAIController* EnemyController =
		Cast<AAIController>(
			GetController()
		))
	{
		EnemyController->StopMovement();
	}

	if (HealthWidgetComponent)
	{
		HealthWidgetComponent->
			SetVisibility(false);
	}

	MulticastPlayDeathSound(
		GetActorLocation()
	);

	ApplyDeathState();
	ForceNetUpdate();
}

void AAshenTrainingEnemy::
	MulticastPlayDeathSound_Implementation(
		FVector_NetQuantize DeathLocation
	)
{
	if (!DeathSound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		this,
		DeathSound,
		DeathLocation,
		DeathSoundVolume
	);
}

void AAshenTrainingEnemy::OnRep_IsDead()
{
	if (!bIsDead)
	{
		return;
	}

	if (HealthWidgetComponent)
	{
		HealthWidgetComponent->
			SetVisibility(false);
	}

	ApplyDeathState();
}

void AAshenTrainingEnemy::ApplyDeathState()
{
	GetWorldTimerManager().ClearTimer(
		SimpleAttackAnimationTimerHandle
	);

	bAttackAnimationPlaying = false;

	if (UCharacterMovementComponent* Movement =
		GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
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

	/*
	 * A dedicated death animation is more reliable for imported
	 * Paragon characters than forcing their demo AnimBP or an
	 * incompatible ragdoll setup.
	 */
	if (bUseSimpleAnimationSystem &&
		IsValid(DeathAnimation))
	{
		EnemyMesh->SetSimulatePhysics(false);

		EnemyMesh->SetCollisionEnabled(
			ECollisionEnabled::NoCollision
		);

		PlaySimpleAnimation(
			DeathAnimation,
			false
		);

		return;
	}

	/*
	 * Fallback: use ragdoll when no death animation is assigned.
	 */
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
			FVector::UpVector * 0.25f;

		EnemyMesh->AddImpulse(
			ImpulseDirection.GetSafeNormal() *
				DeathImpulse,
			NAME_None,
			true
		);
	}
}

void AAshenTrainingEnemy::
	UpdateSimpleAnimation()
{
	if (!bUseSimpleAnimationSystem ||
		bIsDead ||
		bAttackAnimationPlaying)
	{
		return;
	}

	const float MovementSpeed =
		GetVelocity().Size2D();

	UAnimationAsset* DesiredAnimation =
		MovementSpeed >
			MoveAnimationSpeedThreshold
			? WalkAnimation
			: IdleAnimation;

	if (!IsValid(DesiredAnimation))
	{
		return;
	}

	PlaySimpleAnimation(
		DesiredAnimation,
		true
	);
}

void AAshenTrainingEnemy::
	PlaySimpleAnimation(
		UAnimationAsset* Animation,
		bool bLooping
	)
{
	if (!bUseSimpleAnimationSystem ||
		!IsValid(Animation))
	{
		return;
	}

	USkeletalMeshComponent* EnemyMesh =
		GetMesh();

	if (!EnemyMesh)
	{
		return;
	}

	if (CurrentSimpleAnimation == Animation &&
		bCurrentAnimationLooping == bLooping &&
		EnemyMesh->IsPlaying())
	{
		return;
	}

	EnemyMesh->SetAnimationMode(
		EAnimationMode::AnimationSingleNode,
		true
	);

	EnemyMesh->PlayAnimation(
		Animation,
		bLooping
	);

	CurrentSimpleAnimation = Animation;
	bCurrentAnimationLooping = bLooping;
}

void AAshenTrainingEnemy::
	PlayAttackAnimationLocally()
{
	if (!bUseSimpleAnimationSystem ||
		!IsValid(AttackAnimation) ||
		bIsDead)
	{
		return;
	}

	bAttackAnimationPlaying = true;

	PlaySimpleAnimation(
		AttackAnimation,
		false
	);

	const float AnimationDuration =
		FMath::Max(
			AttackAnimation->GetPlayLength(),
			0.1f
		);

	GetWorldTimerManager().SetTimer(
		SimpleAttackAnimationTimerHandle,
		this,
		&AAshenTrainingEnemy::
			FinishAttackAnimationLocally,
		AnimationDuration,
		false
	);
}

void AAshenTrainingEnemy::
	FinishAttackAnimationLocally()
{
	bAttackAnimationPlaying = false;
	UpdateSimpleAnimation();
}
