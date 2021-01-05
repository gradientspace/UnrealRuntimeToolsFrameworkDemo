#include "DynamicSDMCActor.h"
#include "MeshComponentRuntimeUtils.h"
#include "DynamicMesh3.h"
#include "Operations/MeshConvexHull.h"
#include "Materials/Material.h"


// Sets default values
ADynamicSDMCActor::ADynamicSDMCActor()
{
	MeshComponent = CreateDefaultSubobject<URuntimeDynamicMeshComponent>(TEXT("MeshComponent"), false);
	SetRootComponent(MeshComponent);
}

// Called when the game starts or when spawned
void ADynamicSDMCActor::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void ADynamicSDMCActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



void ADynamicSDMCActor::OnMeshEditedInternal()
{
	UpdateSDMCMesh();
	Super::OnMeshEditedInternal();
}

void ADynamicSDMCActor::UpdateSDMCMesh()
{
	if (MeshComponent)
	{
		*(MeshComponent->GetMesh()) = SourceMesh;

		if (this->CollisionMode == EDynamicMeshActorCollisionMode::ComplexAsSimple
			|| this->CollisionMode == EDynamicMeshActorCollisionMode::ComplexAsSimpleAsync)
		{
			//MeshComponent->bUseAsyncCooking = (this->CollisionMode == EDynamicMeshActorCollisionMode::ComplexAsSimpleAsync);
			MeshComponent->bUseComplexAsSimpleCollision = true;
		}
		else if (this->CollisionMode == EDynamicMeshActorCollisionMode::SimpleConvexHull)
		{
			// generate convex collision
			FMeshConvexHull HullCompute(&SourceMesh);
			int32 NumTris = FMath::Clamp(this->MaxHullTriangles, 0, 1000);
			if (NumTris != 0)
			{
				HullCompute.bPostSimplify = true;
				HullCompute.MaxTargetFaceCount = NumTris;
			}
			if (HullCompute.Compute())
			{
				FSimpleShapeSet3d ShapeSet;
				FConvexShape3d& Convex = ShapeSet.Convexes.Emplace_GetRef();
				Convex.Mesh = MoveTemp(HullCompute.ConvexHull);

				MeshComponent->bUseComplexAsSimpleCollision = false;
				MeshComponent->SetSimpleCollisionGeometry(MoveTemp(ShapeSet));
			}
		}

		MeshComponent->NotifyMeshUpdated();

		// update material
		UMaterialInterface* UseMaterial = (this->Material != nullptr) ? this->Material : UMaterial::GetDefaultMaterial(MD_Surface);
		MeshComponent->SetMaterial(0, UseMaterial);
	}
}

