#include "AshenPlayerCharacter.h"

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
#include "AshenAttributeComponent.h"
#include "AshenPlayerHUDWidget.h"

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
	GetCharacterMovement()->MaxWalkSpeed = 450.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(
		TEXT("CameraBoom")
	);
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(
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
	CreatePlayerHUD();
}

void AAshenPlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	const APlayerController* PlayerController =
		Cast<APlayerController>(Controller);

	if (PlayerController && DefaultMappingContext)
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
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
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput =
		Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (!EnhancedInput)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("AshenPlayerCharacter requires Enhanced Input.")
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
}

void AAshenPlayerCharacter::Move(
	const FInputActionValue& Value
)
{
	const FVector2D MovementInput = Value.Get<FVector2D>();

	if (!Controller)
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(
		0.0f,
		ControlRotation.Yaw,
		0.0f
	);

	const FVector ForwardDirection =
		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	const FVector RightDirection =
		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

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
	const FVector2D LookInput = Value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
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