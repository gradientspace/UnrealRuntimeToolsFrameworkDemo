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

UCLASS()
class RUNTIMETOOLSSYSTEM_API URuntimeMeshSceneObject : public UObject
{
	GENERATED_BODY()

public:

	URuntimeMeshSceneObject();

	void Initialize(UWorld* TargetWorld, const FMeshDescription* InitialMeshDescription);

	void Initialize(ADynamicPMCActor* Actor, UGeneratedMesh* NewMesh);

	void SetTransform(FTransform Transform);


	ADynamicMeshBaseActor* GetActor();
	UMeshComponent* GetMeshComponent();

	UPROPERTY()
	ADynamicSDMCActor* SimpleDynamicMeshActor = nullptr;


	UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
	void SetToHighlightMaterial(UMaterialInterface* Material);

	UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
	void ClearHighlightMaterial();


	UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
	bool IntersectRay(FVector RayOrigin, FVector RayDirection, FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle, FVector& TriBaryCoords, float MaxDistance = 0);



protected:

	TUniquePtr<FDynamicMesh3> SourceMesh;
	TUniquePtr<FDynamicMeshAABBTree3> MeshAABBTree;

	void UpdateSourceMesh(const FMeshDescription* MeshDescription);

	void OnExternalDynamicMeshComponentUpdate();
};



