#include "AshenPurgeRitualObjective.h"

#include "AshenTrainingEnemy.h"

#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AAshenPurgeRitualObjective::
AAshenPurgeRitualObjective()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bAlwaysRelevant = true;
	bNetLoadOnClient = true;

	SetReplicateMovement(false);

	SceneRoot =
		CreateDefaultSubobject<USceneComponent>(
			TEXT("SceneRoot")
		);

	SetRootComponent(SceneRoot);

	CompletionZone =
		CreateDefaultSubobject<UBoxComponent>(
			TEXT("CompletionZone")
		);

	CompletionZone->SetupAttachment(SceneRoot);

	CompletionZone->SetBoxExtent(
		FVector(250.0f, 250.0f, 180.0f)
	);

	CompletionZone->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly
	);

	CompletionZone->SetCollisionResponseToAllChannels(
		ECR_Ignore
	);

	CompletionZone->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Overlap
	);

	CompletionZone->SetGenerateOverlapEvents(true);

	static ConstructorHelpers::FObjectFinder<USoundBase>
		VictorySoundFinder(
			TEXT(
				"/Game/Audio/SFX/S_Ashen_Victory."
				"S_Ashen_Victory"
			)
		);

	if (VictorySoundFinder.Succeeded())
	{
		VictorySound =
			VictorySoundFinder.Object;
	}
}

void AAshenPurgeRitualObjective::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	FindCaptainIfNeeded();

	GetWorldTimerManager().SetTimer(
		CompletionCheckTimerHandle,
		this,
		&AAshenPurgeRitualObjective::
		EvaluateCompletion,
		CompletionCheckInterval,
		true,
		CompletionCheckInterval
	);
}

void AAshenPurgeRitualObjective::EndPlay(
	const EEndPlayReason::Type EndPlayReason
)
{
	GetWorldTimerManager().ClearTimer(
		CompletionCheckTimerHandle
	);

	Super::EndPlay(EndPlayReason);
}

void AAshenPurgeRitualObjective::
GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
	Super::GetLifetimeReplicatedProps(
		OutLifetimeProps
	);

	DOREPLIFETIME(
		AAshenPurgeRitualObjective,
		bRitualCompleted
	);
}

void AAshenPurgeRitualObjective::
EvaluateCompletion()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(
		AshenKeep_Objective_Evaluate
	);

	if (!HasAuthority() ||
		bRitualCompleted)
	{
		return;
	}

	if (!IsValid(Captain))
	{
		FindCaptainIfNeeded();
	}

	if (!IsValid(Captain))
	{
		if (!bLoggedMissingCaptain)
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT(
					"Ashen Keep: Purge Ritual could not find the Hunter Captain."
				)
			);

			bLoggedMissingCaptain = true;
		}

		return;
	}

	/*
	 * Победа наступает сразу после смерти
	 * капитана. Нахождение в зоне больше
	 * не требуется.
	 */
	if (Captain->IsDead())
	{
		CompleteRitual();
	}
}

void AAshenPurgeRitualObjective::
FindCaptainIfNeeded()
{
	if (IsValid(Captain))
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	AAshenTrainingEnemy* BestCandidate =
		nullptr;

	float BestDistanceSquared =
		FMath::Square(CaptainAutoFindRadius);

	for (
		TActorIterator<AAshenTrainingEnemy>
		Iterator(World);
		Iterator;
		++Iterator
		)
	{
		AAshenTrainingEnemy* Candidate =
			*Iterator;

		if (!IsValid(Candidate))
		{
			continue;
		}

		const FString ActorName =
			Candidate->GetName();

		const FString ClassName =
			Candidate->GetClass()->GetName();

		const bool bLooksLikeCaptain =
			Candidate->ActorHasTag(
				TEXT("Boss")
			) ||
			ActorName.Contains(
				TEXT("HunterCaptain")
			) ||
			ClassName.Contains(
				TEXT("HunterCaptain")
			);

		if (!bLooksLikeCaptain)
		{
			continue;
		}

		const float DistanceSquared =
			FVector::DistSquared(
				GetActorLocation(),
				Candidate->GetActorLocation()
			);

		if (DistanceSquared <
			BestDistanceSquared)
		{
			BestDistanceSquared =
				DistanceSquared;

			BestCandidate = Candidate;
		}
	}

	Captain = BestCandidate;

	if (IsValid(Captain))
	{
		UE_LOG(
			LogTemp,
			Display,
			TEXT(
				"Ashen Keep: Purge Ritual connected to Captain %s."
			),
			*Captain->GetName()
		);
	}
}

void AAshenPurgeRitualObjective::
CompleteRitual()
{
	if (!HasAuthority() ||
		bRitualCompleted)
	{
		return;
	}

	bRitualCompleted = true;

	GetWorldTimerManager().ClearTimer(
		CompletionCheckTimerHandle
	);

	ForceNetUpdate();

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Ashen Keep: Purge Ritual completed."
		)
	);

	MulticastShowVictory();
}

void AAshenPurgeRitualObjective::
MulticastShowVictory_Implementation()
{
	ShowVictoryLocally();
}

void AAshenPurgeRitualObjective::
OnRep_RitualCompleted()
{
	if (bRitualCompleted)
	{
		ShowVictoryLocally();
	}
}

void AAshenPurgeRitualObjective::
ShowVictoryLocally()
{
	if (bVictoryShownLocally ||
		GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (!VictoryWidgetClass)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Ashen Keep: Victory Widget Class is not assigned."
			)
		);

		return;
	}

	APlayerController* LocalController =
		UGameplayStatics::GetPlayerController(
			this,
			0
		);

	if (!LocalController ||
		!LocalController->IsLocalController())
	{
		return;
	}

	if (VictorySound)
	{
		UGameplayStatics::PlaySound2D(
			this,
			VictorySound,
			VictorySoundVolume
		);
	}

	UUserWidget* VictoryWidget =
		CreateWidget<UUserWidget>(
			LocalController,
			VictoryWidgetClass
		);

	if (!VictoryWidget)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Ashen Keep: Failed to create Victory Widget."
			)
		);

		return;
	}

	const bool bAddedToScreen =
		VictoryWidget->AddToPlayerScreen(1000);

	if (!bAddedToScreen)
	{
		VictoryWidget->AddToViewport(1000);
	}

	LocalController->SetShowMouseCursor(true);

	FInputModeUIOnly InputMode;

	InputMode.SetWidgetToFocus(
		VictoryWidget->TakeWidget()
	);

	LocalController->SetInputMode(InputMode);

	bVictoryShownLocally = true;
}