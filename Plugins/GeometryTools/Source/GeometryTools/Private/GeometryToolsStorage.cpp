// Bartender Ender
// Naughty Panda @ 2022

#include "GeometryToolsStorage.h"
#include "BakedDynamicMeshActor.h"
#include "BakedStaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Algo/ForEach.h"
#include "EditorAssetLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogGeometryToolsStorage, Display, Display);

AGeometryToolsStorage::AGeometryToolsStorage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGeometryToolsStorage::AddActor(ABakedDynamicMeshActor* Actor)
{
	// Mark actors dirty.
	UKismetSystemLibrary::TransactObject(this);
	UKismetSystemLibrary::TransactObject(Actor);

	// Set new actor location.
	const FVector NewActorLocation = Actor->GetActorLocation() + StoredWorldOffset;
	Actor->SetActorLocation(NewActorLocation);

	// Modify actor label.
	const FString NewActorLabel(StoredLabelPrefix + Actor->GetActorLabel());
	Actor->SetActorLabel(NewActorLabel);

	// Store actor.
	StoredActors.Add(Actor);

	UE_LOG(LogGeometryToolsStorage, Warning, TEXT("%s stored. StoredActors: %i"), *NewActorLabel, StoredActors.Num());
}

ABakedDynamicMeshActor* AGeometryToolsStorage::ReleaseActor(ABakedStaticMeshActor* Actor)
{
	ABakedDynamicMeshActor* ActorToRelease = nullptr;

	for (auto DynamicActorsIterator = StoredActors.CreateIterator(); DynamicActorsIterator; ++DynamicActorsIterator)
	{
		if ((*DynamicActorsIterator)->GeneratorKey == Actor->GeneratorKey)
		{
			ActorToRelease = *DynamicActorsIterator;

			// Mark actors dirty.
			UKismetSystemLibrary::TransactObject(ActorToRelease);
			UKismetSystemLibrary::TransactObject(this);

			// Remove current Dynamic Actor from storage.
			DynamicActorsIterator.RemoveCurrent();

			// Update Dynamic Actor transform from Static Actor's transform.
			ActorToRelease->SetActorTransform(Actor->GetTransform());

			// Update actor's current label and previous label.
			FString DynamicActorLabel = ActorToRelease->GetActorLabel();

			if (!DynamicActorLabel.RemoveFromStart(StoredLabelPrefix, ESearchCase::CaseSensitive))
			{
				UE_LOG(LogGeometryToolsStorage, Error, TEXT("ReleaseActor: Incorrect label: %s. Actor: &s"), *DynamicActorLabel, *GetNameSafe(ActorToRelease));
			}

			ActorToRelease->SetActorLabel(DynamicActorLabel);
			ActorToRelease->PreviousLabel = Actor->GetActorLabel();

			return ActorToRelease;
		}
	}

	return ActorToRelease;
}

void AGeometryToolsStorage::UpdateStorage()
{
	StoredActors.Reset();

	TArray<AActor*> WorldStaticActors, WorldDynamicActors;
	UGameplayStatics::GetAllActorsOfClass(this, ABakedStaticMeshActor::StaticClass(), WorldStaticActors);
	UGameplayStatics::GetAllActorsOfClass(this, ABakedDynamicMeshActor::StaticClass(), WorldDynamicActors);

	auto FindSupportedActors = [&](AActor* CurrentDynamicActor) -> void
	{
		if (CurrentDynamicActor->GetActorLabel().StartsWith(StoredLabelPrefix, ESearchCase::CaseSensitive))
		{
			ABakedDynamicMeshActor* DynamicActor = Cast<ABakedDynamicMeshActor>(CurrentDynamicActor);

			// Store Dynamic Actor if it has baked Static version, otherwise destroy it.
			HasBakedStaticActor(DynamicActor, WorldStaticActors) ? StoredActors.Add(DynamicActor) : DynamicActor->Destroy();
		}
	};

	Algo::ForEach(WorldDynamicActors, FindSupportedActors);

	UE_LOG(LogGeometryToolsStorage, Warning, TEXT("Geometry tools storage updated. Found %i dynamic mesh actors."), StoredActors.Num());

	UEditorAssetLibrary::CheckoutLoadedAsset(this);
}

bool AGeometryToolsStorage::HasBakedStaticActor(ABakedDynamicMeshActor* DynamicActor, TArray<AActor*>& StaticActors) const
{
	const FString DynamicActorKey(DynamicActor->GeneratorKey);

	for (auto StaticActorsIterator = StaticActors.CreateIterator(); StaticActorsIterator; ++StaticActorsIterator)
	{
		if (const ABakedStaticMeshActor* CurrentStaticActor = Cast<ABakedStaticMeshActor>(*StaticActorsIterator))
		{
			if (CurrentStaticActor->GeneratorKey == DynamicActorKey)
			{
				// Remove current Static Actor from an array.
				StaticActorsIterator.RemoveCurrent();
				return true;
			}
		}
	}

	return false;
}
