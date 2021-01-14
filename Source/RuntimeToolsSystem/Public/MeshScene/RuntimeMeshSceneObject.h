// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Components/PrimitiveComponent.h"
#include "Templates/PimplPtr.h"
#include "DynamicMesh3.h"
#include "DynamicMeshAABBTree3.h"
#include "DynamicPMCActor.h"
#include "DynamicSDMCActor.h"
#include "RuntimeMeshSceneObject.generated.h"

struct FMeshDescription;

/**
 * URuntimeMeshSceneObject is a "Scene Object" in the "Scene". Do not create these yourself.
 * Use the functions in URuntimeMeshSceneSubsystem to create and manage SceneObjects.
 * 
 * Conceptually, URuntimeMeshSceneObject is a triangle mesh object that can be selected,
 * transformed, and edited using mesh editing tools. 
 * 
 * Under the hood, URuntimeMeshSceneObject will spawn a ADynamicSDMCActor to actually implement
 * most of that functionality. But, the premise is that the higher level Scene is not aware
 * of those details.
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API URuntimeMeshSceneObject : public UObject
{
	GENERATED_BODY()

public:
	URuntimeMeshSceneObject();

	void Initialize(UWorld* TargetWorld, const FMeshDescription* InitialMeshDescription);
	void Initialize(UWorld* TargetWorld, const FDynamicMesh3* InitialMesh);

	// set the 3D transform of this SceneObject
	void SetTransform(FTransform Transform);

	// get the Actor that represents this SceneObject
	ADynamicMeshBaseActor* GetActor();

	// get the mesh component that represents this SceneObject
	UMeshComponent* GetMeshComponent();


	//
	// Material functions
	//

	UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
	void CopyMaterialsFromComponent();

	UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
	void SetAllMaterials(UMaterialInterface* SetToMaterial);

	UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
	void SetToHighlightMaterial(UMaterialInterface* Material);

	UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
	void ClearHighlightMaterial();


	//
	// Spatial Query functions
	//

	UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
	bool IntersectRay(FVector RayOrigin, FVector RayDirection, FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle, FVector& TriBaryCoords, float MaxDistance = 0);


protected:
	// URuntimeMeshSceneObject's representation in UE Level is a ADynamicSDMCActor
	UPROPERTY()
	ADynamicSDMCActor* SimpleDynamicMeshActor = nullptr;

protected:

	TUniquePtr<FDynamicMesh3> SourceMesh;
	TUniquePtr<FDynamicMeshAABBTree3> MeshAABBTree;

	void UpdateSourceMesh(const FMeshDescription* MeshDescription);

	void OnExternalDynamicMeshComponentUpdate();

	TArray<UMaterialInterface*> Materials;
	void UpdateComponentMaterials(bool bForceRefresh);
};



