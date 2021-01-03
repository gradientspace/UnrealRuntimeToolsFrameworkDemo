
#include "RuntimeDynamicMeshComponent.h"
#include "MeshQueries.h"
#include "Physics/PhysicsDataCollection.h"
#include "Engine/CollisionProfile.h"

URuntimeDynamicMeshComponent::URuntimeDynamicMeshComponent()
{
	SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

}

void URuntimeDynamicMeshComponent::NotifyMeshUpdated()
{
	USimpleDynamicMeshComponent::NotifyMeshUpdated();

	RegenerateCollision_Immediate();
}


void URuntimeDynamicMeshComponent::SetSimpleCollisionGeometry(const FSimpleShapeSet3d& SimpleShapes, bool bDeferCollisionUpdate)
{
	SimpleCollisionShapes = SimpleShapes;
	if (bDeferCollisionUpdate == false)
	{
		RegenerateCollision_Immediate();
	}
}

void URuntimeDynamicMeshComponent::SetSimpleCollisionGeometry(FSimpleShapeSet3d&& SimpleShapes, bool bDeferCollisionUpdate)
{
	SimpleCollisionShapes = MoveTemp(SimpleShapes);
	if (bDeferCollisionUpdate == false)
	{
		RegenerateCollision_Immediate();
	}
}


UBodySetup* URuntimeDynamicMeshComponent::GetBodySetup()
{
	if (MeshBodySetup == nullptr)
	{
		//MeshBodySetup = NewObject<UBodySetup>(this, NAME_None, (IsTemplate() ? RF_Public : RF_NoFlags));
		MeshBodySetup = NewObject<UBodySetup>(this);
		MeshBodySetup->BodySetupGuid = FGuid::NewGuid();
		// ??
		MeshBodySetup->bGenerateMirroredCollision = false;
		// ??
		MeshBodySetup->bDoubleSidedGeometry = true;
		MeshBodySetup->CollisionTraceFlag = bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;
	}

	return MeshBodySetup;
}



void URuntimeDynamicMeshComponent::RegenerateCollision_Immediate()
{
	UWorld* World = GetWorld();
	
	//const bool bUseAsyncCook = World && World->IsGameWorld() && bUseAsyncCooking;

	UBodySetup* UseBodySetup = GetBodySetup();


	if (SimpleCollisionShapes.TotalElementsNum() > 0)
	{
		FPhysicsDataCollection PhysicsData;
		PhysicsData.Geometry = SimpleCollisionShapes;
		PhysicsData.CopyGeometryToAggregate();
		UseBodySetup->AggGeom = MoveTemp(PhysicsData.AggGeom);
	}
	else
	{
		UseBodySetup->AggGeom = FKAggregateGeom();
	}

	// can add simple collision AggGeom here...

	UseBodySetup->CollisionTraceFlag = bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;

	// New GUID as collision has changed
	UseBodySetup->BodySetupGuid = FGuid::NewGuid();
	// Also we want cooked data for this
	UseBodySetup->bHasCookedCollisionData = true;
	UseBodySetup->InvalidatePhysicsData();
	UseBodySetup->CreatePhysicsMeshes();
	RecreatePhysicsState();
}




bool URuntimeDynamicMeshComponent::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{
	// todo: support  UPhysicsSettings::Get()->bSupportUVFromHitResults ?
	 
	const FDynamicMesh3* CurMesh = GetMesh();

	TArray<int32> VertexMap;
	bool bIsSparseV = !CurMesh->IsCompactV();
	if (bIsSparseV)
	{
		VertexMap.SetNum(CurMesh->MaxVertexID());
	}

	// copy vertices
	CollisionData->Vertices.Reserve(CurMesh->VertexCount());
	for (int32 vid : CurMesh->VertexIndicesItr())
	{
		int32 Index = CollisionData->Vertices.Add((FVector)CurMesh->GetVertex(vid));
		if (bIsSparseV)
		{
			VertexMap[vid] = Index;
		}
		else
		{
			check(vid == Index);
		}
	}

	// copy triangles
	CollisionData->Indices.Reserve(CurMesh->TriangleCount());
	CollisionData->MaterialIndices.Reserve(CurMesh->TriangleCount());
	for (int32 tid : CurMesh->TriangleIndicesItr())
	{
		FIndex3i Tri = CurMesh->GetTriangle(tid);

		FTriIndices Triangle;
		Triangle.v0 = (bIsSparseV) ? VertexMap[Tri.A] : Tri.A;
		Triangle.v1 = (bIsSparseV) ? VertexMap[Tri.B] : Tri.B;
		Triangle.v2 = (bIsSparseV) ? VertexMap[Tri.C] : Tri.C;
		CollisionData->Indices.Add(Triangle);

		CollisionData->MaterialIndices.Add(0);		// not sure what this is for...
	}

	CollisionData->bFlipNormals = true;
	CollisionData->bDeformableMesh = true;
	CollisionData->bFastCook = true;

	return true;
}


bool URuntimeDynamicMeshComponent::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
	return true;
}

bool URuntimeDynamicMeshComponent::WantsNegXTriMesh()
{
	return false;
}