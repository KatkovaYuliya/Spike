// Bartender Ender
// Naughty Panda @ 2022

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityActor.h"
#include "GeometryToolsStorage.generated.h"

class ABakedDynamicMeshActor;
class ABakedStaticMeshActor;

/**
 * Editor Utility Actor that stores Baked Dynamic Mesh Actors.
 */
UCLASS(Abstract)
class GEOMETRYTOOLS_API AGeometryToolsStorage : public AEditorUtilityActor
{
	GENERATED_BODY()

public:
	AGeometryToolsStorage(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Add Dynamic Mesh Actor to stored actors.
	UFUNCTION(BlueprintCallable, Category = "Tools")
	void AddActor(ABakedDynamicMeshActor* Actor);

	// Release Static Mesh Actor from stored actors.
	UFUNCTION(BlueprintCallable, Category = "Tools")
	ABakedDynamicMeshActor* ReleaseActor(ABakedStaticMeshActor* Actor);

	// Update storage with world actors.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Settings")
	void UpdateStorage();

private:
	// Check if Dynamic Actor has associated baked Static Actor.
	UE_NODISCARD FORCEINLINE bool HasBakedStaticActor(ABakedDynamicMeshActor* DynamicActor, TArray<AActor*>& StaticActors) const;

private:
	// World location offset to store dynamic meshes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Meta = (AllowPrivateAccess = "true"))
	FVector StoredWorldOffset{0., 0., -5000.};

	// Array of stored actors.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings", Meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<ABakedDynamicMeshActor>> StoredActors;

	// Prefix to append stored actor's label.
	FString StoredLabelPrefix{TEXT("STORED_")};
};
