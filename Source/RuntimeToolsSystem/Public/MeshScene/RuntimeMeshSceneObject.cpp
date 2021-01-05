
#include "RuntimeMeshSceneObject.h"

#include "DynamicMesh3.h"
#include "DynamicMeshAABBTree3.h"
#include "MeshDescriptionToDynamicMesh.h"
#include "DynamicMeshToMeshDescription.h"

#include "Materials/Material.h"


URuntimeMeshSceneObject::URuntimeMeshSceneObject()
{
	if (!SourceMesh)
	{
		SourceMesh = MakeUnique<FDynamicMesh3>();
	}
	if (!MeshAABBTree)
	{
		MeshAABBTree = MakeUnique<FDynamicMeshAABBTree3>();
	}

	UMaterialInterface* DefaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);
	Materials.Add(DefaultMaterial);
}


void URuntimeMeshSceneObject::Initialize(UWorld* TargetWorld, const FMeshDescription* InitialMeshDescription)
{
	FActorSpawnParameters SpawnInfo;
	SimpleDynamicMeshActor = TargetWorld->SpawnActor<ADynamicSDMCActor>(FVector::ZeroVector, FRotator(0, 0, 0), SpawnInfo);

	// listen for changes
	SimpleDynamicMeshActor->MeshComponent->OnMeshChanged.AddLambda([this]() { 
		OnExternalDynamicMeshComponentUpdate(); 
	});

	GetActor()->SourceType = EDynamicMeshActorSourceType::ExternallyGenerated;
	GetActor()->CollisionMode = EDynamicMeshActorCollisionMode::ComplexAsSimpleAsync;

	UpdateSourceMesh(InitialMeshDescription);

	GetActor()->EditMesh([&](FDynamicMesh3& MeshToEdit)
	{
		MeshToEdit = *SourceMesh;
	});

	UpdateComponentMaterials(false);
}

void URuntimeMeshSceneObject::Initialize(UWorld* TargetWorld, const FDynamicMesh3* InitialMesh)
{
	FActorSpawnParameters SpawnInfo;
	SimpleDynamicMeshActor = TargetWorld->SpawnActor<ADynamicSDMCActor>(FVector::ZeroVector, FRotator(0, 0, 0), SpawnInfo);

	// listen for changes
	SimpleDynamicMeshActor->MeshComponent->OnMeshChanged.AddLambda([this]() {
		OnExternalDynamicMeshComponentUpdate();
	});

	GetActor()->SourceType = EDynamicMeshActorSourceType::ExternallyGenerated;
	GetActor()->CollisionMode = EDynamicMeshActorCollisionMode::ComplexAsSimpleAsync;

	*SourceMesh = *InitialMesh;
	MeshAABBTree->SetMesh(SourceMesh.Get(), true);

	GetActor()->EditMesh([&](FDynamicMesh3& MeshToEdit)
	{
		MeshToEdit = *SourceMesh;
	});

	UpdateComponentMaterials(false);
}


void URuntimeMeshSceneObject::OnExternalDynamicMeshComponentUpdate()
{
	const FDynamicMesh3* Mesh = SimpleDynamicMeshActor->MeshComponent->GetMesh();
	*SourceMesh = *Mesh;
	MeshAABBTree->SetMesh(SourceMesh.Get(), true);
}


void URuntimeMeshSceneObject::SetTransform(FTransform Transform)
{
	GetActor()->SetActorTransform(Transform);
}


ADynamicMeshBaseActor* URuntimeMeshSceneObject::GetActor()
{
	return SimpleDynamicMeshActor;
}

UMeshComponent* URuntimeMeshSceneObject::GetMeshComponent()
{
	return (SimpleDynamicMeshActor) ? SimpleDynamicMeshActor->MeshComponent : nullptr;
}



void URuntimeMeshSceneObject::CopyMaterialsFromComponent()
{
	UMeshComponent* Component = GetMeshComponent();
	int32 NumMaterials = Component->GetNumMaterials();
	if (NumMaterials == 0)
	{
		Materials = { UMaterial::GetDefaultMaterial(MD_Surface) };
	}
	else
	{
		Materials.SetNum(NumMaterials);
		for (int32 k = 0; k < NumMaterials; ++k)
		{
			Materials[k] = Component->GetMaterial(k);
		}
	}
}


void URuntimeMeshSceneObject::SetAllMaterials(UMaterialInterface* SetToMaterial)
{
	int32 NumMaterials = Materials.Num();
	for (int32 k = 0; k < NumMaterials; ++k)
	{
		Materials[k] = SetToMaterial;
	}
	UpdateComponentMaterials(true);
}


void URuntimeMeshSceneObject::SetToHighlightMaterial(UMaterialInterface* Material)
{
	UMeshComponent* Component = GetMeshComponent();
	int32 NumMaterials = FMath::Max(1, Component->GetNumMaterials());
	for (int32 k = 0; k < NumMaterials; ++k)
	{
		Component->SetMaterial(k, Material);
	}

	// HACK TO FORCE MATERIAL UPDATE IN SDMC
	SimpleDynamicMeshActor->MeshComponent->NotifyMeshUpdated();
}

void URuntimeMeshSceneObject::ClearHighlightMaterial()
{
	UpdateComponentMaterials(true);
}


void URuntimeMeshSceneObject::UpdateComponentMaterials(bool bForceRefresh)
{
	UMaterialInterface* DefaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);

	UMeshComponent* Component = GetMeshComponent();
	if (!Component) return;

	int32 NumMaterials = FMath::Max(1, Component->GetNumMaterials());
	for (int32 k = 0; k < NumMaterials; ++k)
	{
		UMaterialInterface* SetMaterial = (k < Materials.Num()) ? Materials[k] : DefaultMaterial;
		Component->SetMaterial(k, SetMaterial);
	}

	// HACK TO FORCE MATERIAL UPDATE IN SDMC
	if (bForceRefresh)
	{
		SimpleDynamicMeshActor->MeshComponent->NotifyMeshUpdated();
	}
}



void URuntimeMeshSceneObject::UpdateSourceMesh(const FMeshDescription* MeshDescriptionIn)
{
	FMeshDescriptionToDynamicMesh Converter;
	FDynamicMesh3 TmpMesh;
	Converter.Convert(MeshDescriptionIn, TmpMesh);
	*SourceMesh = MoveTemp(TmpMesh);

	MeshAABBTree->SetMesh(SourceMesh.Get(), true);
}



bool URuntimeMeshSceneObject::IntersectRay(FVector RayOrigin, FVector RayDirection, FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle, FVector& TriBaryCoords, float MaxDistance)
{
	if (!ensure(SourceMesh)) return false;

	FTransform3d ActorToWorld(GetActor()->GetActorTransform());
	FVector3d WorldDirection(RayDirection); WorldDirection.Normalize();
	FRay3d LocalRay(ActorToWorld.InverseTransformPosition((FVector3d)RayOrigin),
		ActorToWorld.InverseTransformNormal(WorldDirection));
	IMeshSpatial::FQueryOptions QueryOptions;
	if (MaxDistance > 0)
	{
		QueryOptions.MaxDistance = MaxDistance;
	}
	NearestTriangle = MeshAABBTree->FindNearestHitTriangle(LocalRay, QueryOptions);
	if (SourceMesh->IsTriangle(NearestTriangle))
	{
		FIntrRay3Triangle3d IntrQuery = TMeshQueries<FDynamicMesh3>::TriangleIntersection(*SourceMesh, NearestTriangle, LocalRay);
		if (IntrQuery.IntersectionType == EIntersectionType::Point)
		{
			HitDistance = IntrQuery.RayParameter;
			WorldHitPoint = (FVector)ActorToWorld.TransformPosition(LocalRay.PointAt(IntrQuery.RayParameter));
			TriBaryCoords = (FVector)IntrQuery.TriangleBaryCoords;
			return true;
		}
	}
	return false;
}
