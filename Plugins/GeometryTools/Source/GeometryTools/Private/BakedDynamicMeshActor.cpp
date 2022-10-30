// Bartender Ender
// Naughty Panda @ 2022

#include "BakedDynamicMeshActor.h"
#include "Algo/ForEach.h"
#include "GeometryScript/MeshQueryFunctions.h"
#include "GeometryScript/ListUtilityFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshRepairFunctions.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshPolygroupFunctions.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "GeometryScript/MeshModelingFunctions.h"
#include "GeometryScript/CreateNewAssetUtilityFunctions.h"
#include "GeometryScript/CollisionFunctions.h"
#include "Kismet/KismetGuidLibrary.h"

#define LOCTEXT_NAMESPACE "GeometryTools_ABakedDynamicMeshActor"

DEFINE_LOG_CATEGORY_STATIC(LogGeometryTools, Display, Display);

ABakedDynamicMeshActor::ABakedDynamicMeshActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	BakedStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BakedStaticMeshComponent"));
	BakedStaticMeshComponent->SetupAttachment(DynamicMeshComponent);

	bIsEditable = true;
	bEnableNanite = false;
}

void ABakedDynamicMeshActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SwitchComponentsVisibility();
}

void ABakedDynamicMeshActor::SwitchComponentsVisibility() const
{
	DynamicMeshComponent->SetVisibility(bIsEditable);
	BakedStaticMeshComponent->SetVisibility(!bIsEditable);
	BakedStaticMeshComponent->SetStaticMesh(TargetStaticMesh);
}

UDynamicMesh* ABakedDynamicMeshActor::ClearBadTriangles(UDynamicMesh* TargetMesh, float SafeNormalTolerance, bool bDeleteBadTriangles, bool bDebugLog)
{
	TArray<int32> TriangleIndexArray, BadTriangleArray;
	FGeometryScriptIndexList TriangleIndexList;
	bool bHasTriangleIDGaps = false;

	UGeometryScriptLibrary_MeshQueryFunctions::GetAllTriangleIDs(TargetMesh, TriangleIndexList, bHasTriangleIDGaps);
	UGeometryScriptLibrary_ListUtilityFunctions::ConvertIndexListToArray(TriangleIndexList, TriangleIndexArray);

	auto FindBadTriangles = [&](const auto& TriangleID) -> void
	{
		// Get vertexes.
		bool bIsValidTriangle = false;
		FVector Vertex1, Vertex2, Vertex3;
		UGeometryScriptLibrary_MeshQueryFunctions::GetTrianglePositions(TargetMesh, TriangleID, bIsValidTriangle, Vertex1, Vertex2, Vertex3);

		// Compare vertexes.
		const float SquaredVectorLength = FVector::CrossProduct(Vertex3 - Vertex1, Vertex2 - Vertex1).SquaredLength();

		if (SquaredVectorLength < SafeNormalTolerance)
		{
			BadTriangleArray.Add(TriangleID);
		}
	};

	Algo::ForEach(TriangleIndexArray, FindBadTriangles);

	if (!BadTriangleArray.IsEmpty())
	{
		if (bDebugLog)
		{
			UE_LOG(LogGeometryTools, Warning, TEXT("TriangleIndexArray num: %i; Bad triangles: %i"), TriangleIndexArray.Num(), BadTriangleArray.Num());
		}

		if (bDeleteBadTriangles)
		{
			TriangleIndexList.Reset(EGeometryScriptIndexType::Triangle);
			UGeometryScriptLibrary_ListUtilityFunctions::ConvertArrayToIndexList(BadTriangleArray, TriangleIndexList, EGeometryScriptIndexType::Triangle);

			int32 DeletedTrianglesNum = 0;
			UGeometryScriptLibrary_MeshBasicEditFunctions::DeleteTrianglesFromMesh(TargetMesh, TriangleIndexList, DeletedTrianglesNum, false);
			UGeometryScriptLibrary_MeshRepairFunctions::CompactMesh(TargetMesh);
		}
	}

	return TargetMesh;
}

UDynamicMesh* ABakedDynamicMeshActor::Bevel(UDynamicMesh* TargetMesh, float BevelDistance, float CreaseAngle)
{
	const FGeometryScriptGroupLayer GroupLayer;
	UGeometryScriptLibrary_MeshPolygroupFunctions::ComputePolygroupsFromAngleThreshold(TargetMesh, GroupLayer, CreaseAngle);

	const FGeometryScriptMeshBevelOptions BevelOptions{BevelDistance, true};
	UGeometryScriptLibrary_MeshModelingFunctions::ApplyMeshPolygroupBevel(TargetMesh, BevelOptions);

	const FGeometryScriptSplitNormalsOptions NormalOptions{true, CreaseAngle};
	const FGeometryScriptCalculateNormalsOptions CalculateOptions;
	UGeometryScriptLibrary_MeshNormalsFunctions::ComputeSplitNormals(TargetMesh, NormalOptions, CalculateOptions);

	return TargetMesh;
}

void ABakedDynamicMeshActor::GenerateNewStaticMesh()
{
	GeneratorKey = UKismetGuidLibrary::NewGuid().ToString();

	// Generate new asset path and name.
	TEnumAsByte<EGeometryScriptOutcomePins> Outcome;
	FString UniqueAssetPathAndName, UniqueAssetName;
	const FString BasePath(TEXT("/Game/Generated/Meshes"));
	const FString AssetFolderPath(FolderName.IsEmpty() ? BasePath : BasePath + "/" + FolderName);
	const FString NewAssetName(AssetName.StartsWith(TEXT("SM_")) ? AssetName : TEXT("SM_") + AssetName);

	UGeometryScriptLibrary_CreateNewAssetFunctions::CreateUniqueNewAssetPathName(AssetFolderPath, NewAssetName, UniqueAssetPathAndName, UniqueAssetName, FGeometryScriptUniqueAssetNameOptions(), Outcome);

	if (Outcome == EGeometryScriptOutcomePins::Failure)
	{
		UE_LOG(LogGeometryTools, Error, TEXT("GenerateNewStaticMesh: Failed to generate asset path!"));
		return;
	}

	// Create new static mesh.
	UDynamicMesh* DynamicMesh = DynamicMeshComponent->GetDynamicMesh();
	const FGeometryScriptCreateNewStaticMeshAssetOptions AssetOptions{
		false,
		false,
		static_cast<bool>(bEnableNanite),
		NaniteProxyTrianglePercent,
		true,
		ECollisionTraceFlag::CTF_UseComplexAsSimple
	};

	UStaticMesh* CreatedMesh = UGeometryScriptLibrary_CreateNewAssetFunctions::CreateNewStaticMeshAssetFromMesh(DynamicMesh, UniqueAssetPathAndName, AssetOptions, Outcome);

	if (Outcome == EGeometryScriptOutcomePins::Failure)
	{
		UE_LOG(LogGeometryTools, Error, TEXT("GenerateNewStaticMesh: Failed to create new asset!"));
		return;
	}

	// Setup target mesh and collision.
	TargetStaticMesh = CreatedMesh;
	UGeometryScriptLibrary_CollisionFunctions::SetStaticMeshCollisionFromComponent(TargetStaticMesh, DynamicMeshComponent);

	// Copy materials.
	CopyMaterialsToTargetMesh();
}

void ABakedDynamicMeshActor::BakeToStaticMesh()
{
	UDynamicMesh* DynamicMesh = DynamicMeshComponent->GetDynamicMesh();

	// Copy dynamic mesh to static mesh.
	const TArray<UMaterialInterface*> Materials;
	const TArray<FName> MaterialSlotNames;

	const FGeometryScriptNaniteOptions NaniteOptions{
		static_cast<bool>(bEnableNanite),
		NaniteProxyTrianglePercent,
		0.f
	};

	const FGeometryScriptCopyMeshToAssetOptions AssetOptions{
		false,
		false,
		false,
		false,
		Materials,
		MaterialSlotNames,
		true,
		NaniteOptions,
		true,
		false
	};

	TEnumAsByte<EGeometryScriptOutcomePins> Outcome;

	UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshToStaticMesh(DynamicMesh, TargetStaticMesh, AssetOptions, FGeometryScriptMeshWriteLOD(), Outcome);

	if (Outcome == EGeometryScriptOutcomePins::Failure)
	{
		UE_LOG(LogGeometryTools, Error, TEXT("BakeToStaticMesh: Failed to copy mesh!"));
		return;
	}

	// Copy materials.
	CopyMaterialsToTargetMesh();

	// Setup collision.
	UGeometryScriptLibrary_CollisionFunctions::SetStaticMeshCollisionFromComponent(TargetStaticMesh, DynamicMeshComponent);
}

void ABakedDynamicMeshActor::CopyMaterialsToTargetMesh()
{
	int32 MaterialIndex = 0;

	auto CopyMaterialsToStaticMesh = [&](const auto& NewMaterial) -> void
	{
		TargetStaticMesh->SetMaterial(MaterialIndex++, NewMaterial);
	};

	TArray<UMaterialInterface*> DynamicMeshMaterials = DynamicMeshComponent->GetMaterials();
	Algo::ForEach(DynamicMeshMaterials, CopyMaterialsToStaticMesh);
}

#undef LOCTEXT_NAMESPACE
