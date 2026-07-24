#include "AshenDamageVolume.h"

#include "AshenAttributeComponent.h"
#include "Components/BoxComponent.h"

AAshenDamageVolume::AAshenDamageVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	DamageBox = CreateDefaultSubobject<UBoxComponent>(
		TEXT("DamageBox")
	);

	SetRootComponent(DamageBox);

	DamageBox->SetBoxExtent(
		FVector(100.0f, 100.0f, 50.0f)
	);

	DamageBox->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly
	);

	DamageBox->SetCollisionResponseToAllChannels(
		ECR_Ignore
	);

	DamageBox->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Overlap
	);

	DamageBox->SetGenerateOverlapEvents(true);
}

void AAshenDamageVolume::BeginPlay()
{
	Super::BeginPlay();

	DamageBox->OnComponentBeginOverlap.AddDynamic(
		this,
		&AAshenDamageVolume::HandleBeginOverlap
	);

	DamageBox->OnComponentEndOverlap.AddDynamic(
		this,
		&AAshenDamageVolume::HandleEndOverlap
	);
}

void AAshenDamageVolume::EndPlay(
	const EEndPlayReason::Type EndPlayReason
)
{
	StopDamageTimer();
	OverlappingActors.Reset();

	Super::EndPlay(EndPlayReason);
}

void AAshenDamageVolume::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!HasAuthority() || !IsValid(OtherActor))
	{
		return;
	}

	UAshenAttributeComponent* AttributeComponent =
		OtherActor->FindComponentByClass<UAshenAttributeComponent>();

	if (!AttributeComponent)
	{
		return;
	}

	const int32 PreviousActorCount = OverlappingActors.Num();

	OverlappingActors.Add(OtherActor);

	// Не наносим повторный мгновенный урон,
	// если тот же Actor уже был зарегистрирован.
	if (OverlappingActors.Num() == PreviousActorCount)
	{
		return;
	}

	// Первый урон сразу при входе.
	ApplyDamageToActor(OtherActor);

	if (PreviousActorCount == 0)
	{
		StartDamageTimer();
	}
}

void AAshenDamageVolume::HandleEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex
)
{
	if (!HasAuthority() || !IsValid(OtherActor))
	{
		return;
	}

	OverlappingActors.Remove(OtherActor);

	if (OverlappingActors.Num() == 0)
	{
		StopDamageTimer();
	}
}

void AAshenDamageVolume::StartDamageTimer()
{
	if (DamageInterval <= 0.0f ||
		GetWorldTimerManager().IsTimerActive(DamageTimerHandle))
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		DamageTimerHandle,
		this,
		&AAshenDamageVolume::ApplyDamageToOverlappingActors,
		DamageInterval,
		true,
		DamageInterval
	);
}

void AAshenDamageVolume::StopDamageTimer()
{
	GetWorldTimerManager().ClearTimer(DamageTimerHandle);
}

void AAshenDamageVolume::ApplyDamageToOverlappingActors()
{
	if (!HasAuthority())
	{
		return;
	}

	for (auto Iterator = OverlappingActors.CreateIterator();
		Iterator;
		++Iterator)
	{
		AActor* TargetActor = Iterator->Get();

		if (!IsValid(TargetActor))
		{
			Iterator.RemoveCurrent();
			continue;
		}

		ApplyDamageToActor(TargetActor);
	}

	if (OverlappingActors.Num() == 0)
	{
		StopDamageTimer();
	}
}

void AAshenDamageVolume::ApplyDamageToActor(
	AActor* TargetActor
)
{
	if (!IsValid(TargetActor))
	{
		return;
	}

	UAshenAttributeComponent* AttributeComponent =
		TargetActor->FindComponentByClass<UAshenAttributeComponent>();

	if (!AttributeComponent ||
		!AttributeComponent->IsAlive())
	{
		return;
	}

	const float AppliedDamage =
		AttributeComponent->ApplyDamage(DamageAmount);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("%s received %.1f periodic damage from %s"),
		*TargetActor->GetName(),
		AppliedDamage,
		*GetName()
	);
}