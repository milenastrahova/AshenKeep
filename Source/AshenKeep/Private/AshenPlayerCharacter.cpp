#include "AshenPlayerCharacter.h"

#include "AshenAttributeComponent.h"
#include "AshenPlayerHUDWidget.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
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
		FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->MaxWalkSpeed =
		WalkSpeed;

	CameraBoom =
		CreateDefaultSubobject<USpringArmComponent>(
			TEXT("CameraBoom")
		);

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

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
}

void AAshenPlayerCharacter::Move(
	const FInputActionValue& Value
)
{
	const FVector2D MovementInput =
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
		MovementInput.Y
	);

	AddMovementInput(
		RightDirection,
		MovementInput.X
	);
}

void AAshenPlayerCharacter::Look(
	const FInputActionValue& Value
)
{
	const FVector2D LookInput =
		Value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void AAshenPlayerCharacter::StartSprint(
	const FInputActionValue& Value
)
{
	if (!AttributeComponent ||
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
	else
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
	GetCharacterMovement()->MaxWalkSpeed =
		bIsSprinting
		? SprintSpeed
		: WalkSpeed;
}

void AAshenPlayerCharacter::UpdateSprintStamina()
{
	if (!HasAuthority() ||
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
	if (!AttributeComponent ||
		!AttributeComponent->IsAlive() ||
		!AttributeComponent->HasEnoughStamina(
			DodgeStaminaCost
		) ||
		!GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	FVector DodgeDirection =
		GetLastMovementInputVector();

	DodgeDirection.Z = 0.0f;

	if (!DodgeDirection.Normalize())
	{
		DodgeDirection =
			GetActorForwardVector();

		DodgeDirection.Z = 0.0f;
		DodgeDirection.Normalize();
	}

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

	FVector SafeDirection = DodgeDirection;
	SafeDirection.Z = 0.0f;

	if (!SafeDirection.Normalize())
	{
		SafeDirection =
			GetActorForwardVector();

		SafeDirection.Z = 0.0f;
		SafeDirection.Normalize();
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