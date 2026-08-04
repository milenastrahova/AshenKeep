#include "AshenPlayerCharacter.h"

#include "AshenAttributeComponent.h"
#include "AshenLockOnComponent.h"
#include "AshenPlayerHUDWidget.h"
#include "AshenTrainingEnemy.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Camera/CameraComponent.h"
#include "CollisionShape.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

AAshenPlayerCharacter::AAshenPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f;

	bReplicates = true;
	SetReplicateMovement(true);

	AttributeComponent =
		CreateDefaultSubobject<UAshenAttributeComponent>(
			TEXT("AttributeComponent")
		);

	LockOnTargetingComponent =
		CreateDefaultSubobject<UAshenLockOnComponent>(
			TEXT("LockOnTargetingComponent")
		);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate =
		FRotator(0.0f, 720.0f, 0.0f);

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxAcceleration = 1800.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1400.0f;
	GetCharacterMovement()->GroundFriction = 6.0f;

	CameraBoom =
		CreateDefaultSubobject<USpringArmComponent>(
			TEXT("CameraBoom")
		);

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 330.0f;
	CameraBoom->SocketOffset =
		FVector(0.0f, 55.0f, 60.0f);

	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = false;
	CameraBoom->bEnableCameraRotationLag = false;

	FollowCamera =
		CreateDefaultSubobject<UCameraComponent>(
			TEXT("FollowCamera")
		);

	FollowCamera->SetupAttachment(
		CameraBoom,
		USpringArmComponent::SocketName
	);

	FollowCamera->bUsePawnControlRotation = false;

	static ConstructorHelpers::FObjectFinder<USoundBase>
		AttackSwingSoundFinder(
			TEXT(
				"/Game/Audio/SFX/S_Ashen_SwordSwing."
				"S_Ashen_SwordSwing"
			)
		);

	if (AttackSwingSoundFinder.Succeeded())
	{
		AttackSwingSound =
			AttackSwingSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase>
		AttackHitSoundFinder(
			TEXT(
				"/Game/Audio/SFX/S_Ashen_Hit."
				"S_Ashen_Hit"
			)
		);

	if (AttackHitSoundFinder.Succeeded())
	{
		AttackHitSound =
			AttackHitSoundFinder.Object;
	}
}

void AAshenPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AttributeComponent)
	{
		AttributeComponent->OnDeath.AddDynamic(
			this,
			&AAshenPlayerCharacter::HandleDeath
		);
	}

	if (GetCapsuleComponent())
	{
		OriginalPawnCollisionResponse =
			GetCapsuleComponent()->
			GetCollisionResponseToChannel(
				ECC_Pawn
			);
	}

	ApplyMovementSpeed();
	ApplyMistStepState();
	CreatePlayerHUD();

	if (bUseSimpleAnimationSystem)
	{
		UpdateSimpleAnimation(0.0f);
	}
}

void AAshenPlayerCharacter::Tick(
	float DeltaSeconds
)
{
	Super::Tick(DeltaSeconds);

	UpdateSimpleAnimation(DeltaSeconds);
}

void AAshenPlayerCharacter::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(
		AAshenPlayerCharacter,
		bIsSprinting
	);

	DOREPLIFETIME(
		AAshenPlayerCharacter,
		bIsMistStepping
	);

	DOREPLIFETIME(
		AAshenPlayerCharacter,
		bIsDead
	);
}

void AAshenPlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	const APlayerController* PlayerController =
		Cast<APlayerController>(Controller);

	if (PlayerController && DefaultMappingContext)
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
			ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem
			>(
				PlayerController->GetLocalPlayer()
			);

		if (InputSubsystem)
		{
			InputSubsystem->AddMappingContext(
				DefaultMappingContext,
				0
			);
		}
	}

	CreatePlayerHUD();
}

void AAshenPlayerCharacter::SetupPlayerInputComponent(
	UInputComponent* PlayerInputComponent
)
{
	Super::SetupPlayerInputComponent(
		PlayerInputComponent
	);

	UEnhancedInputComponent* EnhancedInput =
		Cast<UEnhancedInputComponent>(
			PlayerInputComponent
		);

	if (!EnhancedInput)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"AshenPlayerCharacter requires Enhanced Input."
			)
		);

		return;
	}

	if (MoveAction)
	{
		EnhancedInput->BindAction(
			MoveAction,
			ETriggerEvent::Triggered,
			this,
			&AAshenPlayerCharacter::Move
		);

		EnhancedInput->BindAction(
			MoveAction,
			ETriggerEvent::Completed,
			this,
			&AAshenPlayerCharacter::StopMove
		);

		EnhancedInput->BindAction(
			MoveAction,
			ETriggerEvent::Canceled,
			this,
			&AAshenPlayerCharacter::StopMove
		);
	}

	if (LookAction)
	{
		EnhancedInput->BindAction(
			LookAction,
			ETriggerEvent::Triggered,
			this,
			&AAshenPlayerCharacter::Look
		);
	}

	if (JumpAction)
	{
		EnhancedInput->BindAction(
			JumpAction,
			ETriggerEvent::Started,
			this,
			&ACharacter::Jump
		);

		EnhancedInput->BindAction(
			JumpAction,
			ETriggerEvent::Completed,
			this,
			&ACharacter::StopJumping
		);
	}

	if (SprintAction)
	{
		EnhancedInput->BindAction(
			SprintAction,
			ETriggerEvent::Started,
			this,
			&AAshenPlayerCharacter::StartSprint
		);

		EnhancedInput->BindAction(
			SprintAction,
			ETriggerEvent::Completed,
			this,
			&AAshenPlayerCharacter::StopSprint
		);

		EnhancedInput->BindAction(
			SprintAction,
			ETriggerEvent::Canceled,
			this,
			&AAshenPlayerCharacter::StopSprint
		);
	}

	if (DodgeAction)
	{
		EnhancedInput->BindAction(
			DodgeAction,
			ETriggerEvent::Started,
			this,
			&AAshenPlayerCharacter::Dodge
		);
	}

	if (AttackAction)
	{
		EnhancedInput->BindAction(
			AttackAction,
			ETriggerEvent::Started,
			this,
			&AAshenPlayerCharacter::Attack
		);
	}

	if (LockOnAction)
	{
		EnhancedInput->BindAction(
			LockOnAction,
			ETriggerEvent::Started,
			this,
			&AAshenPlayerCharacter::ToggleLockOn
		);
	}
}

void AAshenPlayerCharacter::Move(
	const FInputActionValue& Value
)
{
	if (bIsDead)
	{
		return;
	}

	CachedMovementInput =
		Value.Get<FVector2D>();

	if (!Controller)
	{
		return;
	}

	const FRotator ControlRotation =
		Controller->GetControlRotation();

	const FRotator YawRotation(
		0.0f,
		ControlRotation.Yaw,
		0.0f
	);

	const FVector ForwardDirection =
		FRotationMatrix(YawRotation)
		.GetUnitAxis(EAxis::X);

	const FVector RightDirection =
		FRotationMatrix(YawRotation)
		.GetUnitAxis(EAxis::Y);

	AddMovementInput(
		ForwardDirection,
		CachedMovementInput.Y
	);

	AddMovementInput(
		RightDirection,
		CachedMovementInput.X
	);
}

void AAshenPlayerCharacter::StopMove(
	const FInputActionValue& Value
)
{
	CachedMovementInput =
		FVector2D::ZeroVector;
}

void AAshenPlayerCharacter::Look(
	const FInputActionValue& Value
)
{
	if (bIsDead)
	{
		return;
	}

	const FVector2D LookInput =
		Value.Get<FVector2D>();

	AddControllerYawInput(
		LookInput.X * LookSensitivityX
	);

	AddControllerPitchInput(
		-LookInput.Y * LookSensitivityY
	);
}

void AAshenPlayerCharacter::ToggleLockOn(
	const FInputActionValue& Value
)
{
	if (bIsDead ||
		!LockOnTargetingComponent)
	{
		return;
	}

	LockOnTargetingComponent->ToggleLockOn();
}

void AAshenPlayerCharacter::StartSprint(
	const FInputActionValue& Value
)
{
	if (bIsDead ||
		bIsMistStepping ||
		!AttributeComponent ||
		!AttributeComponent->IsAlive() ||
		!AttributeComponent->HasEnoughStamina(1.0f))
	{
		return;
	}

	if (HasAuthority())
	{
		SetSprinting(true);
	}
	else
	{
		ServerSetSprinting(true);
	}
}

void AAshenPlayerCharacter::StopSprint(
	const FInputActionValue& Value
)
{
	if (HasAuthority())
	{
		SetSprinting(false);
	}
	else
	{
		ServerSetSprinting(false);
	}
}

void AAshenPlayerCharacter::ServerSetSprinting_Implementation(
	bool bNewSprinting
)
{
	if (bIsDead || bIsMistStepping)
	{
		SetSprinting(false);
		return;
	}

	if (bNewSprinting)
	{
		const bool bCanSprint =
			AttributeComponent &&
			AttributeComponent->IsAlive() &&
			AttributeComponent->HasEnoughStamina(1.0f);

		SetSprinting(bCanSprint);
		return;
	}

	SetSprinting(false);
}

void AAshenPlayerCharacter::SetSprinting(
	bool bNewSprinting
)
{
	if (!HasAuthority() ||
		bIsSprinting == bNewSprinting)
	{
		return;
	}

	if (bIsDead || bIsMistStepping)
	{
		bNewSprinting = false;
	}

	bIsSprinting = bNewSprinting;

	ApplyMovementSpeed();
	StopStaminaTimers();

	if (bIsSprinting)
	{
		GetWorldTimerManager().SetTimer(
			SprintStaminaTimerHandle,
			this,
			&AAshenPlayerCharacter::UpdateSprintStamina,
			StaminaUpdateInterval,
			true,
			StaminaUpdateInterval
		);
	}
	else if (!bIsDead)
	{
		StartStaminaRegeneration();
	}

	ForceNetUpdate();
}

void AAshenPlayerCharacter::OnRep_IsSprinting()
{
	ApplyMovementSpeed();
}

void AAshenPlayerCharacter::ApplyMovementSpeed()
{
	if (!GetCharacterMovement())
	{
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed =
		bIsSprinting
		? SprintSpeed
		: WalkSpeed;
}

void AAshenPlayerCharacter::UpdateSprintStamina()
{
	if (!HasAuthority() ||
		bIsDead ||
		bIsMistStepping ||
		!bIsSprinting ||
		!AttributeComponent ||
		!AttributeComponent->IsAlive())
	{
		SetSprinting(false);
		return;
	}

	if (GetVelocity().SizeSquared2D() <= 1.0f)
	{
		return;
	}

	const float StaminaCost =
		SprintStaminaCostPerSecond *
		StaminaUpdateInterval;

	const bool bConsumed =
		AttributeComponent->ConsumeStamina(
			StaminaCost
		);

	if (!bConsumed ||
		AttributeComponent->GetStamina() <=
		KINDA_SMALL_NUMBER)
	{
		SetSprinting(false);
	}
}

void AAshenPlayerCharacter::StartStaminaRegeneration()
{
	if (!HasAuthority() ||
		bIsDead ||
		!AttributeComponent ||
		AttributeComponent->GetStamina() >=
		AttributeComponent->GetMaxStamina())
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		StaminaRegenerationTimerHandle,
		this,
		&AAshenPlayerCharacter::RegenerateStamina,
		StaminaUpdateInterval,
		true,
		StaminaRegenerationDelay
	);
}

void AAshenPlayerCharacter::RegenerateStamina()
{
	if (!HasAuthority() ||
		bIsDead ||
		bIsSprinting ||
		!AttributeComponent ||
		!AttributeComponent->IsAlive())
	{
		GetWorldTimerManager().ClearTimer(
			StaminaRegenerationTimerHandle
		);

		return;
	}

	const float RegenerationAmount =
		StaminaRegenerationPerSecond *
		StaminaUpdateInterval;

	AttributeComponent->RestoreStamina(
		RegenerationAmount
	);

	if (AttributeComponent->GetStamina() >=
		AttributeComponent->GetMaxStamina())
	{
		GetWorldTimerManager().ClearTimer(
			StaminaRegenerationTimerHandle
		);
	}
}

void AAshenPlayerCharacter::StopStaminaTimers()
{
	GetWorldTimerManager().ClearTimer(
		SprintStaminaTimerHandle
	);

	GetWorldTimerManager().ClearTimer(
		StaminaRegenerationTimerHandle
	);
}

void AAshenPlayerCharacter::Dodge(
	const FInputActionValue& Value
)
{
	if (bIsDead ||
		bIsMistStepping ||
		!AttributeComponent ||
		!AttributeComponent->IsAlive() ||
		!AttributeComponent->HasEnoughStamina(
			DodgeStaminaCost
		) ||
		!GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	const FVector DodgeDirection =
		GetDesiredDodgeDirection();

	if (HasAuthority())
	{
		PerformDodge(DodgeDirection);
	}
	else
	{
		ServerDodge(DodgeDirection);
	}
}

void AAshenPlayerCharacter::ServerDodge_Implementation(
	FVector DodgeDirection
)
{
	PerformDodge(DodgeDirection);
}

void AAshenPlayerCharacter::PerformDodge(
	const FVector& DodgeDirection
)
{
	if (!HasAuthority() ||
		bIsDead ||
		bIsMistStepping ||
		!bCanDodge ||
		!AttributeComponent ||
		!AttributeComponent->IsAlive() ||
		!AttributeComponent->HasEnoughStamina(
			DodgeStaminaCost
		) ||
		!GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	FVector SafeDirection =
		DodgeDirection.GetSafeNormal2D();

	if (SafeDirection.IsNearlyZero())
	{
		SafeDirection =
			GetCameraForwardDirection();
	}

	const bool bConsumed =
		AttributeComponent->ConsumeStamina(
			DodgeStaminaCost
		);

	if (!bConsumed)
	{
		return;
	}

	if (bIsSprinting)
	{
		SetSprinting(false);
	}
	else
	{
		StopStaminaTimers();
		StartStaminaRegeneration();
	}

	bCanDodge = false;
	bIsMistStepping = true;

	ApplyMistStepState();
	ForceNetUpdate();

	const FVector StartLocation =
		GetActorLocation();

	MulticastPlayMistStepCue(
		StartLocation,
		SafeDirection
	);

	const FVector LaunchVelocity =
		SafeDirection * DodgeStrength +
		FVector::UpVector * DodgeVerticalBoost;

	LaunchCharacter(
		LaunchVelocity,
		true,
		true
	);

	GetWorldTimerManager().SetTimer(
		MistStepTimerHandle,
		this,
		&AAshenPlayerCharacter::EndMistStep,
		MistStepDuration,
		false
	);

	GetWorldTimerManager().SetTimer(
		DodgeCooldownTimerHandle,
		this,
		&AAshenPlayerCharacter::ResetDodgeCooldown,
		DodgeCooldown,
		false
	);
}

void AAshenPlayerCharacter::EndMistStep()
{
	if (!HasAuthority() ||
		!bIsMistStepping)
	{
		return;
	}

	bIsMistStepping = false;

	ApplyMistStepState();
	ForceNetUpdate();
}

void AAshenPlayerCharacter::ApplyMistStepState()
{
	if (USkeletalMeshComponent* PlayerMesh =
		GetMesh())
	{
		PlayerMesh->SetHiddenInGame(
			bIsMistStepping,
			true
		);
	}

	if (UCapsuleComponent* Capsule =
		GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(
			ECC_Pawn,
			bIsMistStepping
			? ECR_Ignore
			: OriginalPawnCollisionResponse
		);
	}
}

void AAshenPlayerCharacter::OnRep_IsMistStepping()
{
	ApplyMistStepState();
}

void AAshenPlayerCharacter::
MulticastPlayMistStepCue_Implementation(
	FVector_NetQuantize StartLocation,
	FVector_NetQuantizeNormal Direction
)
{
	BP_OnMistStep(
		StartLocation,
		Direction
	);
}

void AAshenPlayerCharacter::ResetDodgeCooldown()
{
	bCanDodge = true;
}

void AAshenPlayerCharacter::Attack(
	const FInputActionValue& Value
)
{
	if (bIsDead ||
		bIsMistStepping ||
		!AttributeComponent ||
		!AttributeComponent->IsAlive() ||
		!AttributeComponent->HasEnoughStamina(
			AttackStaminaCost
		))
	{
		return;
	}

	if (HasAuthority())
	{
		PerformAttack();
	}
	else
	{
		ServerAttack();
	}
}

void AAshenPlayerCharacter::ServerAttack_Implementation()
{
	PerformAttack();
}

void AAshenPlayerCharacter::PerformAttack()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(
		AshenKeep_Player_PerformAttack
	);

	if (!HasAuthority() ||
		bIsDead ||
		bIsMistStepping ||
		!bCanAttack ||
		!AttributeComponent ||
		!AttributeComponent->IsAlive() ||
		!AttributeComponent->HasEnoughStamina(
			AttackStaminaCost
		))
	{
		return;
	}

	const bool bConsumed =
		AttributeComponent->ConsumeStamina(
			AttackStaminaCost
		);

	if (!bConsumed)
	{
		return;
	}

	if (bIsSprinting)
	{
		SetSprinting(false);
	}
	else
	{
		StopStaminaTimers();
		StartStaminaRegeneration();
	}

	bCanAttack = false;

	GetWorldTimerManager().SetTimer(
		AttackCooldownTimerHandle,
		this,
		&AAshenPlayerCharacter::ResetAttackCooldown,
		AttackCooldown,
		false
	);

	MulticastPlayAttackAnimationCue();

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	const FVector AttackDirection =
		GetCameraForwardDirection();

	SetActorRotation(
		AttackDirection.Rotation()
	);

	const FVector Start =
		GetActorLocation() +
		FVector(0.0f, 0.0f, 50.0f) +
		AttackDirection * 40.0f;

	const FVector End =
		Start +
		AttackDirection * AttackRange;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(AshenMeleeAttack),
		false,
		this
	);

	TArray<FHitResult> HitResults;

	const bool bHitAnything =
		World->SweepMultiByObjectType(
			HitResults,
			Start,
			End,
			FQuat::Identity,
			ObjectQueryParams,
			FCollisionShape::MakeSphere(
				AttackRadius
			),
			QueryParams
		);

	if (bDrawAttackDebug)
	{
		const FColor DebugColor =
			bHitAnything
			? FColor::Red
			: FColor::Green;

		DrawDebugLine(
			World,
			Start,
			End,
			DebugColor,
			false,
			0.75f,
			0,
			2.0f
		);

		DrawDebugSphere(
			World,
			End,
			AttackRadius,
			20,
			DebugColor,
			false,
			0.75f,
			0,
			2.0f
		);
	}

	TSet<TWeakObjectPtr<AActor>> DamagedActors;

	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor =
			HitResult.GetActor();

		if (!IsValid(HitActor) ||
			HitActor == this)
		{
			continue;
		}

		const TWeakObjectPtr<AActor> ActorKey(
			HitActor
		);

		if (DamagedActors.Contains(ActorKey))
		{
			continue;
		}

		UAshenAttributeComponent*
			TargetAttributeComponent =
			HitActor->FindComponentByClass<
			UAshenAttributeComponent
			>();

		if (!TargetAttributeComponent ||
			!TargetAttributeComponent->IsAlive())
		{
			continue;
		}

		const bool bTargetWasAlive =
			TargetAttributeComponent->IsAlive();

		const float AppliedDamage =
			TargetAttributeComponent->ApplyDamage(
				AttackDamage
			);

		if (AppliedDamage <= 0.0f)
		{
			continue;
		}

		DamagedActors.Add(ActorKey);

		MulticastPlayAttackHitSound(
			HitActor->GetActorLocation()
		);

		const bool bKilledHunter =
			bTargetWasAlive &&
			!TargetAttributeComponent->IsAlive() &&
			Cast<AAshenTrainingEnemy>(HitActor) != nullptr;

		if (bKilledHunter)
		{
			ApplyVampiricKillReward();
		}
	}
}

void AAshenPlayerCharacter::ResetAttackCooldown()
{
	bCanAttack = true;
}

void AAshenPlayerCharacter::
MulticastPlayAttackAnimationCue_Implementation()
{
	PlayAttackAnimationLocally();

	if (AttackSwingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			AttackSwingSound,
			GetActorLocation(),
			AttackSwingVolume
		);
	}
}

void AAshenPlayerCharacter::
MulticastPlayAttackHitSound_Implementation(
	FVector_NetQuantize HitLocation
)
{
	if (!AttackHitSound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		this,
		AttackHitSound,
		HitLocation,
		AttackHitVolume
	);
}

void AAshenPlayerCharacter::UpdateSimpleAnimation(
	float DeltaSeconds
)
{
	if (!bUseSimpleAnimationSystem ||
		bIsDead ||
		bAttackAnimationPlaying ||
		!IsValid(WalkAnimation))
	{
		return;
	}

	/*
	 * Never switch from walk to a separate idle asset. The same compatible
	 * locomotion sequence stays active at all times, and its play rate eases
	 * between movement speed and a very slow idle sway.
	 */
	if (CurrentSimpleAnimation != WalkAnimation ||
		!bCurrentAnimationLooping)
	{
		PlaySimpleAnimation(
			WalkAnimation,
			true
		);
	}

	USkeletalMeshComponent* PlayerMesh =
		GetMesh();

	if (!PlayerMesh)
	{
		return;
	}

	UAnimSingleNodeInstance* SingleNodeInstance =
		PlayerMesh->GetSingleNodeInstance();

	if (!SingleNodeInstance)
	{
		return;
	}

	const bool bMoving =
		GetVelocity().Size2D() >
		MoveAnimationSpeedThreshold;

	const float TargetPlayRate =
		bMoving
		? MovingLocomotionPlayRate
		: IdleLocomotionPlayRate;

	if (DeltaSeconds <= 0.0f)
	{
		CurrentLocomotionPlayRate =
			TargetPlayRate;
	}
	else
	{
		CurrentLocomotionPlayRate =
			FMath::FInterpTo(
				CurrentLocomotionPlayRate,
				TargetPlayRate,
				DeltaSeconds,
				LocomotionPlayRateInterpSpeed
			);
	}

	SingleNodeInstance->SetPlayRate(
		CurrentLocomotionPlayRate
	);

	if (!SingleNodeInstance->IsPlaying())
	{
		SingleNodeInstance->SetPlaying(true);
	}
}

void AAshenPlayerCharacter::PlaySimpleAnimation(
	UAnimationAsset* Animation,
	bool bLooping
)
{
	if (!bUseSimpleAnimationSystem ||
		!IsValid(Animation))
	{
		return;
	}

	USkeletalMeshComponent* PlayerMesh =
		GetMesh();

	if (!PlayerMesh)
	{
		return;
	}

	if (CurrentSimpleAnimation == Animation &&
		bCurrentAnimationLooping == bLooping &&
		PlayerMesh->IsPlaying())
	{
		return;
	}

	PlayerMesh->SetAnimationMode(
		EAnimationMode::AnimationSingleNode,
		true
	);

	PlayerMesh->PlayAnimation(
		Animation,
		bLooping
	);

	CurrentSimpleAnimation = Animation;
	bCurrentAnimationLooping = bLooping;

	if (Animation == WalkAnimation)
	{
		if (UAnimSingleNodeInstance* SingleNodeInstance =
			PlayerMesh->GetSingleNodeInstance())
		{
			SingleNodeInstance->SetPlayRate(
				CurrentLocomotionPlayRate
			);
		}
	}
}

void AAshenPlayerCharacter::
PlayAttackAnimationLocally()
{
	if (!bUseSimpleAnimationSystem ||
		!IsValid(AttackAnimation) ||
		bIsDead)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(
		SimpleAttackAnimationTimerHandle
	);

	bAttackAnimationPlaying = true;

	/*
	 * Force the same attack sequence to restart when the player
	 * attacks again before the previous animation has fully ended.
	 */
	CurrentSimpleAnimation = nullptr;

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
		&AAshenPlayerCharacter::
			FinishAttackAnimationLocally,
		AnimationDuration,
		false
	);
}

void AAshenPlayerCharacter::
FinishAttackAnimationLocally()
{
	bAttackAnimationPlaying = false;
	CurrentSimpleAnimation = nullptr;
	UpdateSimpleAnimation(
		GetWorld()
		? GetWorld()->GetDeltaSeconds()
		: 0.05f
	);
}

void AAshenPlayerCharacter::ApplyVampiricKillReward()
{
	if (!HasAuthority() ||
		bIsDead ||
		!AttributeComponent)
	{
		return;
	}

	const float PreviousHealth =
		AttributeComponent->GetHealth();

	const float PreviousBlood =
		AttributeComponent->GetMana();

	AttributeComponent->RestoreHealth(
		VampiricHealthOnKill
	);

	AttributeComponent->RestoreMana(
		BloodEssenceOnKill
	);

	const float HealthRestored =
		FMath::Max(
			0.0f,
			AttributeComponent->GetHealth() -
			PreviousHealth
		);

	const float BloodRestored =
		FMath::Max(
			0.0f,
			AttributeComponent->GetMana() -
			PreviousBlood
		);

	if (HealthRestored > 0.0f ||
		BloodRestored > 0.0f)
	{
		MulticastPlayVampiricRecoveryCue(
			HealthRestored,
			BloodRestored
		);
	}
}

void AAshenPlayerCharacter::
MulticastPlayVampiricRecoveryCue_Implementation(
	float HealthRestored,
	float BloodRestored
)
{
	BP_OnVampiricRecovery(
		HealthRestored,
		BloodRestored
	);
}

void AAshenPlayerCharacter::HandleDeath()
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	bIsDead = true;

	if (LockOnTargetingComponent)
	{
		LockOnTargetingComponent->ClearLockOn();
	}

	GetWorldTimerManager().ClearTimer(
		MistStepTimerHandle
	);

	if (bIsMistStepping)
	{
		bIsMistStepping = false;
		ApplyMistStepState();
	}

	if (bIsSprinting)
	{
		SetSprinting(false);
	}

	StopStaminaTimers();

	GetWorldTimerManager().ClearTimer(
		DodgeCooldownTimerHandle
	);

	GetWorldTimerManager().ClearTimer(
		AttackCooldownTimerHandle
	);

	GetWorldTimerManager().ClearTimer(
		SimpleAttackAnimationTimerHandle
	);

	bAttackAnimationPlaying = false;

	ApplyDeathState();
	ForceNetUpdate();
}

void AAshenPlayerCharacter::OnRep_IsDead()
{
	if (bIsDead)
	{
		if (LockOnTargetingComponent)
		{
			LockOnTargetingComponent->ClearLockOn();
		}

		bIsMistStepping = false;
		ApplyMistStepState();
		ApplyDeathState();
	}
}

void AAshenPlayerCharacter::ApplyDeathState()
{
	GetWorldTimerManager().ClearTimer(
		SimpleAttackAnimationTimerHandle
	);

	bAttackAnimationPlaying = false;
	CurrentSimpleAnimation = nullptr;

	CachedMovementInput =
		FVector2D::ZeroVector;

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

	if (APlayerController* PlayerController =
		Cast<APlayerController>(Controller))
	{
		PlayerController->SetIgnoreMoveInput(true);
	}

	USkeletalMeshComponent* PlayerMesh =
		GetMesh();

	if (!PlayerMesh)
	{
		return;
	}

	PlayerMesh->SetHiddenInGame(false, true);

	if (bUseSimpleAnimationSystem &&
		IsValid(DeathAnimation))
	{
		PlayerMesh->SetSimulatePhysics(false);

		PlayerMesh->SetCollisionEnabled(
			ECollisionEnabled::NoCollision
		);

		PlaySimpleAnimation(
			DeathAnimation,
			false
		);

		return;
	}

	PlayerMesh->SetCollisionProfileName(
		TEXT("Ragdoll")
	);

	PlayerMesh->SetCollisionEnabled(
		ECollisionEnabled::QueryAndPhysics
	);

	PlayerMesh->SetAllBodiesSimulatePhysics(true);
	PlayerMesh->SetSimulatePhysics(true);
	PlayerMesh->WakeAllRigidBodies();

	if (HasAuthority() && DeathImpulse > 0.0f)
	{
		const FVector ImpulseDirection =
			-GetActorForwardVector() +
			FVector::UpVector * 0.3f;

		PlayerMesh->AddImpulse(
			ImpulseDirection.GetSafeNormal() *
			DeathImpulse,
			NAME_None,
			true
		);
	}
}

FVector AAshenPlayerCharacter::
GetCameraForwardDirection() const
{
	if (Controller)
	{
		const FRotator ControlRotation =
			Controller->GetControlRotation();

		const FRotator YawRotation(
			0.0f,
			ControlRotation.Yaw,
			0.0f
		);

		return FRotationMatrix(YawRotation)
			.GetUnitAxis(EAxis::X)
			.GetSafeNormal2D();
	}

	return GetActorForwardVector()
		.GetSafeNormal2D();
}

FVector AAshenPlayerCharacter::
GetDesiredDodgeDirection() const
{
	if (!Controller)
	{
		return GetActorForwardVector()
			.GetSafeNormal2D();
	}

	const FRotator ControlRotation =
		Controller->GetControlRotation();

	const FRotator YawRotation(
		0.0f,
		ControlRotation.Yaw,
		0.0f
	);

	const FVector ForwardDirection =
		FRotationMatrix(YawRotation)
		.GetUnitAxis(EAxis::X);

	const FVector RightDirection =
		FRotationMatrix(YawRotation)
		.GetUnitAxis(EAxis::Y);

	FVector DodgeDirection =
		ForwardDirection * CachedMovementInput.Y +
		RightDirection * CachedMovementInput.X;

	if (DodgeDirection.IsNearlyZero())
	{
		DodgeDirection =
			ForwardDirection;
	}

	return DodgeDirection.GetSafeNormal2D();
}

void AAshenPlayerCharacter::CreatePlayerHUD()
{
	if (!IsLocallyControlled() ||
		HUDWidgetInstance ||
		!HUDWidgetClass)
	{
		return;
	}

	APlayerController* PlayerController =
		Cast<APlayerController>(Controller);

	if (!PlayerController)
	{
		return;
	}

	HUDWidgetInstance =
		CreateWidget<UAshenPlayerHUDWidget>(
			PlayerController,
			HUDWidgetClass
		);

	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->
		InitializeWithAttributes(
			AttributeComponent
		);

		HUDWidgetInstance->AddToPlayerScreen();
	}
}