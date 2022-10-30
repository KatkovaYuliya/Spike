// Bartender Ender
// Naughty Panda @ 2022

#pragma once

#include "CoreMinimal.h"
#include "GeometryActors/GeneratedDynamicMeshActor.h"
#include "BakedDynamicMeshActor.generated.h"

class UStaticMeshComponent;

/**
 * Base class for editable geometry tools.
 */
UCLASS()
class GEOMETRYTOOLS_API ABakedDynamicMeshActor : public AGeneratedDynamicMeshActor
{
	GENERATED_BODY()

public:
	ABakedDynamicMeshActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnConstruction(const FTransform& Transform) override;

	// Makes only one component visible at a time based on IsEditable value.
	UFUNCTION(BlueprintCallable, Category = "Geometry Tolls")
	FORCEINLINE void SwitchComponentsVisibility() const;

	// Find and remove bad triangles.
	UFUNCTION(BlueprintCallable, Category = "Events")
	static UPARAM(DisplayName = "Target Mesh") UDynamicMesh* ClearBadTriangles(UDynamicMesh* TargetMesh, float SafeNormalTolerance = 0.0005f, bool bDeleteBadTriangles = true, bool bDebugLog = true);

	// Blueprint implementable event to clear materials.
	UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category = "Materials")
	void ClearMaterials();

	// Applies bevel to the selected dynamic mesh.
	UFUNCTION(BlueprintCallable, Category = "Geometry Tolls")
	static UPARAM(DisplayName = "Target Mesh") UDynamicMesh* Bevel(UDynamicMesh* TargetMesh, float BevelDistance = 5.f, float CreaseAngle = 25.f);

	// Creates new static mesh asset in a specified subfolder and sets it as current TargetStaticMesh.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Tool Settings")
	void GenerateNewStaticMesh();

	// Bake current dynamic mesh to selected TargetStaticMesh.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Tool Settings")
	void BakeToStaticMesh();

private:
	// Copy materials from Dynamic Mesh to Target Static Mesh.
	FORCEINLINE void CopyMaterialsToTargetMesh();

private:
	// Visualizes Target Static Mesh when this tool is not editable.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> BakedStaticMeshComponent;

	// Holds baked Dynamic Mesh as Static Mesh.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool Settings", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> TargetStaticMesh;

public:
	// Name used to store this asset in content folder.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Settings")
	FString AssetName{TEXT("GeneratedMesh")};

	// Optional folder name with path Game/Generated/Meshes/...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Settings")
	FString FolderName;

	// Unique key that links Dynamic and Static Mesh Actors.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tool Settings")
	FString GeneratorKey;

	// Previous actor's label. Should be valid if actor was stored.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Tool Settings")
	FString PreviousLabel;

	// Nanite quality percentage.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Settings")
	float NaniteProxyTrianglePercent = 100.f;

	// If editable - Dynamic Mesh is visible. If not - Static Mesh is visible.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Settings")
	uint8 bIsEditable : 1;

	// Should Nanite be enabled for this Static Mesh asset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Settings")
	uint8 bEnableNanite : 1;
};
