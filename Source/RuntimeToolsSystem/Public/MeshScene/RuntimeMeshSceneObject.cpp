
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
	return SimpleDynamicMeshActor->MeshComponent;
}


void URuntimeMeshSceneObject::Initialize(ADynamicPMCActor* Actor, UGeneratedMesh* NewMesh)
{
	//const FDynamicMesh3* InitialMesh = NewMesh->GetMesh().Get();

	//if (!this->MeshDescription)
	//{
	//	this->MeshDescription = MakePimpl<FMeshDescription>();
	//	FStaticMeshAttributes StaticMeshAttributes(*this->MeshDescription);
	//	StaticMeshAttributes.Register();
	//}


	//FDynamicMeshToMeshDescription Converter;
	//Converter.Convert(InitialMesh, *this->MeshDescription);

	//UpdateDynamicMeshFromMeshDescription();

	//ProcMeshActor = Actor;
	//ProcMeshActor->EditMesh([&](FDynamicMesh3& MeshToEdit)
	//{
	//	MeshToEdit = *SourceMesh;
	//});
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
	UMaterialInterface* DefaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);

	UMeshComponent* Component = GetMeshComponent();
	int32 NumMaterials = FMath::Max(1, Component->GetNumMaterials());
	for (int32 k = 0; k < NumMaterials; ++k)
	{
		Component->SetMaterial(k, DefaultMaterial);
	}

	// HACK TO FORCE MATERIAL UPDATE IN SDMC
	SimpleDynamicMeshActor->MeshComponent->NotifyMeshUpdated();
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
