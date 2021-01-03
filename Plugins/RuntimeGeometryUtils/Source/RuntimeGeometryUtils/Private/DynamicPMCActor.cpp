#include "DynamicPMCActor.h"
#include "MeshComponentRuntimeUtils.h"
#include "DynamicMesh3.h"
#include "Operations/MeshConvexHull.h"


// Sets default values
ADynamicPMCActor::ADynamicPMCActor()
{
	MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"), false);
	SetRootComponent(MeshComponent);
}

// Called when the game starts or when spawned
void ADynamicPMCActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADynamicPMCActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ADynamicPMCActor::OnMeshEditedInternal()
{
	UpdatePMCMesh();
	Super::OnMeshEditedInternal();
}

void ADynamicPMCActor::UpdatePMCMesh()
{
	if (MeshComponent)
	{
		bool bUseFaceNormals = (this->NormalsMode == EDynamicMeshActorNormalsMode::FaceNormals);
		bool bUseUV0 = true;
		bool bUseVertexColors = false;

		bool bGenerateSectionCollision = false;
		if (this->CollisionMode == EDynamicMeshActorCollisionMode::ComplexAsSimple
			|| this->CollisionMode == EDynamicMeshActorCollisionMode::ComplexAsSimpleAsync)
		{
			bGenerateSectionCollision = true;
			MeshComponent->bUseAsyncCooking = (this->CollisionMode == EDynamicMeshActorCollisionMode::ComplexAsSimpleAsync);
			MeshComponent->bUseComplexAsSimpleCollision = true;
		}

		RTGUtils::UpdatePMCFromDynamicMesh_SplitTriangles(MeshComponent, &SourceMesh, bUseFaceNormals, bUseUV0, bUseVertexColors, bGenerateSectionCollision);

		// update material on new section
		UMaterialInterface* UseMaterial = (this->Material != nullptr) ? this->Material : UMaterial::GetDefaultMaterial(MD_Surface);
		MeshComponent->SetMaterial(0, UseMaterial);

		// generate convex collision
		if (this->CollisionMode == EDynamicMeshActorCollisionMode::SimpleConvexHull)
		{
			FMeshConvexHull HullCompute(&SourceMesh);
			int32 NumTris = FMath::Clamp(this->MaxHullTriangles, 0, 1000);
			if (NumTris != 0)
			{
				HullCompute.bPostSimplify = true;
				HullCompute.MaxTargetFaceCount = NumTris;
			}
			if (HullCompute.Compute())
			{
				TArray<FVector> Points;
				for (FVector3d Pos : HullCompute.ConvexHull.VerticesItr())
				{
					Points.Add((FVector)Pos);
				}
				MeshComponent->bUseComplexAsSimpleCollision = false;
				MeshComponent->ClearCollisionConvexMeshes();
				MeshComponent->AddCollisionConvexMesh(Points);
			}
		}
	}
}