// Bartender Ender
// Naughty Panda @ 2022

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "BakedStaticMeshActor.generated.h"

/**
 * Base class to represent generated and stored geometry tool.
 */
UCLASS()
class GEOMETRYTOOLS_API ABakedStaticMeshActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	ABakedStaticMeshActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	// Unique key that links Dynamic and Static Mesh Actors.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
	FString GeneratorKey;
};
