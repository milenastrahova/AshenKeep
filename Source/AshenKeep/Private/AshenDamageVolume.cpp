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

	const float AppliedDamage =
		AttributeComponent->ApplyDamage(DamageAmount);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("%s received %.1f damage from %s"),
		*OtherActor->GetName(),
		AppliedDamage,
		*GetName()
	);
}