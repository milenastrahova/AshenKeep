#include "AshenPlayerCharacter.h"

#include "AshenAttributeComponent.h"
#include "AshenPlayerHUDWidget.h"
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
#include "Net/UnrealNetwork.h"

AAshenPlayerCharacter::AAshenPlayerCharacter()
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
		FRotator(0.0f, 720.0f, 0.0f);

	GetCharacterMovement()->MaxWalkSpeed =
		WalkSpeed;

	GetCharacterMovement()->MaxAcceleration =
		1800.0f;

	GetCharacterMovement()->BrakingDecelerationWalking =
		1400.0f;

	GetCharacterMovement()->GroundFriction =
		6.0f;

	CameraBoom =
		CreateDefaultSubobject<USpringArmComponent>(
			TEXT("CameraBoom")
		);

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 12.0f;
	CameraBoom->CameraLagMaxDistance = 35.0f;

	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 15.0f;

	FollowCamera =
		CreateDefaultSubobject<UCameraComponent>(
			TEXT("FollowCamera")
		);

	FollowCamera->SetupAttachment(
		CameraBoom,
		USpringArmComponent::SocketName
	);

	FollowCamera->bUsePawnControlRotation = false;
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

	ApplyMovementSpeed();
	CreatePlayerHUD();
}

void AAshenPlayerCharacter::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
	Super::GetLifetimeReplicatedProps(
		OutLifetimeProps
	);

	DOREPLIFETIME(
		AAshenPlayerCharacter,
		bIsSprinting
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
	const FVector2D LookInput =
		Value.Get<FVector2D>();

	AddControllerYawInput(
		LookInput.X * LookSensitivityX
	);

	AddControllerPitchInput(
		LookInput.Y * LookSensitivityY
	);
}

void AAshenPlayerCharacter::StartSprint(
	const FInputActionValue& Value
)
{
	if (bIsDead ||
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
	if (bIsDead)
	{
		SetSprinting(false);
		return;
	}

	if (bNewSprinting)
	{
		const bool bCanSprint =
			AttributeComponent &&
			AttributeComponent->IsAlive() &&
			AttributeComponent->HasEnoughStamina(
				1.0f
			);

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

	if (bIsDead)
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

	const FVector LaunchVelocity =
		SafeDirection * DodgeStrength +
		FVector::UpVector * DodgeVerticalBoost;

	LaunchCharacter(
		LaunchVelocity,
		true,
		true
	);

	GetWorldTimerManager().SetTimer(
		DodgeCooldownTimerHandle,
		this,
		&AAshenPlayerCharacter::ResetDodgeCooldown,
		DodgeCooldown,
		false
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
	if (!HasAuthority() ||
		bIsDead ||
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

	ObjectQueryParams.AddObjectTypesToQuery(
		ECC_Pawn
	);

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

		const float AppliedDamage =
			TargetAttributeComponent->ApplyDamage(
				AttackDamage
			);

		if (AppliedDamage <= 0.0f)
		{
			continue;
		}

		DamagedActors.Add(ActorKey);

		UE_LOG(
			LogTemp,
			Log,
			TEXT(
				"%s attacked %s for %.1f damage."
			),
			*GetName(),
			*HitActor->GetName(),
			AppliedDamage
		);
	}
}

void AAshenPlayerCharacter::ResetAttackCooldown()
{
	bCanAttack = true;
}

void AAshenPlayerCharacter::HandleDeath()
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	bIsDead = true;

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

	ApplyDeathState();
	ForceNetUpdate();
}

void AAshenPlayerCharacter::OnRep_IsDead()
{
	if (bIsDead)
	{
		ApplyDeathState();
	}
}

void AAshenPlayerCharacter::ApplyDeathState()
{
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

FVector AAshenPlayerCharacter::GetCameraForwardDirection() const
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

FVector AAshenPlayerCharacter::GetDesiredDodgeDirection() const
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
		HUDWidgetInstance->AddToPlayerScreen();
	}
}