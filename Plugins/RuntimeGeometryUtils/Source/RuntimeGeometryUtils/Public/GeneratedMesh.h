#pragma once

#include "CoreMinimal.h"
#include "DynamicMesh3.h"
#include "DynamicMeshAABBTree3.h"
#include "Spatial/FastWinding.h"
#include "GeneratedMesh.generated.h"

class ADynamicMeshBaseActor;


UENUM(BlueprintType)
enum class EGeneratedMeshBooleanOperation : uint8
{
	Union = 0,
	Subtraction = 1,
	Intersection = 2
};


/**
 * UGeneratedMesh stores a "temporary mesh" and provides a set of operations to create
 * and manipulate the geometry of that mesh, run spatial queries against it, and so on.
 * The mesh does not have any visual representation or game functionality, this is 
 * purely a data-storage container with modification operations.
 */
UCLASS(BlueprintType, Transient)
class RUNTIMEGEOMETRYUTILS_API UGeneratedMesh : public UObject
{
	GENERATED_BODY()

public:
	UGeneratedMesh();


	/** Clear the mesh and reset the Append Transform */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|Initialization") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* ResetMesh();

	/** Copy the SourceMesh from a MeshActor */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|Initialization") UPARAM(DisplayName = "New Mesh")
	UGeneratedMesh* InitializeFrom(ADynamicMeshBaseActor* MeshActor);

	/** Copy the Mesh from another GeneratedMesh */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|Initialization") UPARAM(DisplayName = "New Mesh")
	UGeneratedMesh* MakeDuplicate(UGeneratedMesh* Mesh);

	/**
	 * Update SourceMesh by reading external mesh file at Path. Optionally flip orientation.
	 * Note: Path may be relative to Content folder, otherwise it must be an absolute path.
	 * @return false if mesh read failed
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|Initialization")
	bool ReadMeshFromFile(FString Path, bool bFlipOrientation);

	/**
	 * Set the Append Transform. This transform will be applied to any shapes created using the AppendX() functions (AppendBox, AppendSphere, etc)
	 * before merging them into the accumulated mesh.
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|ShapeGenerators") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* SetAppendTransform(FTransform Transform);

	/**
	 * Clear the current Append Transform
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|ShapeGenerators") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* ClearAppendTransform();


	/**
	 * Append an Axis-Aligned Box defined by the given Min and Max corners
	 * @param StepsX number of subdivisions along X axis
	 * @param StepsY number of subdivisions along Y axis
	 * @param StepsZ number of subdivisions along Z axis
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|ShapeGenerators") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* AppendAxisBox(FVector Min = FVector(-50,-50,-50), FVector Max = FVector(50, 50, 50), int32 StepsX = 0, int32 StepsY = 0, int32 StepsZ = 0);

	/**
	 * Append an Axis-Aligned Box defined by an FBox
	 * @param StepsX number of subdivisions along X axis
	 * @param StepsY number of subdivisions along Y axis
	 * @param StepsZ number of subdivisions along Z axis
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|ShapeGenerators") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* AppendBox(FBox Box, int32 StepsX = 0, int32 StepsY = 0, int32 StepsZ = 0);

	/**
	 * Append a Sphere centered at the Origin that has a standard polar triangulation
	 * @param Radius radius of the sphere
	 * @param Slices number of vertical sections of the sphere, ie if it were an orange
	 * @param Stacks number of horizontal sections of the sphere
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|ShapeGenerators") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* AppendSphere(float Radius = 25.0, int32 Slices = 6, int32 Stacks = 6);

	/**
	 * Append a Sphere centered at the Origin that has a normalized box triangulation
	 * @param Radius radius of the sphere
	 * @param Steps number of subdivisions along each box axis
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|ShapeGenerators") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* AppendSphereBox(float Radius = 25.0, int32 Steps = 6);

	/**
	 * Append a Cylinder with base at the Origin and aligned up the +Z axis
	 * @param Radius radius of the cylinder
	 * @param Height height of the cylinder
	 * @param Slices number of vertical sections of the cylinder, ie if it were an orange
	 * @param Stacks number of horizontal sections of the cylinder
	 * @param bCapped if true, the ends of the cylinder are filled
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|ShapeGenerators") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* AppendCylinder(float Radius = 25.0, float Height = 100.0, int32 Slices = 6, int32 Stacks = 2, bool bCapped = true);

	/**
	 * Append a Cone with base at the Origin and aligned up the +Z axis
	 * @param BaseRadius radius of the cone at it's base
	 * @param TopRadius radius of the cone at it's top
	 * @param Height height of the cone
	 * @param Slices number of vertical sections of the cone, ie if it were an orange
	 * @param Stacks number of horizontal sections of the cone
	 * @param bCapped if true, the ends of the cone are closed
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|ShapeGenerators") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* AppendCone(float BaseRadius = 25.0, float TopRadius = 5.0, float Height = 100.0, int Slices = 6, int Stacks = 2, bool bCapped = true);

	/**
	 * Append a Torus/Donut, with the major circle lying in the XY plane
	 * @param Radius radius of XY-plane circle
	 * @param SectionRadius radius of the vertical profile-curve circle swept around the XY circle
	 * @param CircleSlices number of sample points on the XY-plane circle
	 * @param SectionSlices number of sample points on the profile circle
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|ShapeGenerators") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* AppendTorus(float Radius = 50.0, float SectionRadius = 10.0, int CircleSlices = 6, int SectionSlices = 6);

	/**
	 * Revolve a 2D polygon around the +Z axis to create a closed shape
	 * @param Polygon points on the 2D polygon
	 * @param Radius radius of XY-plane circle
	 * @param RevolveSteps number of sample sample points on the circle of revolution
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|ShapeGenerators") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* AppendRevolvePolygon(TArray<FVector2D> Polygon, float Radius = 100.0, int RevolveSteps = 6);

	/**
	 * Extrude a 2D polygon upwards from the Origin along the +Z axis
	 * @param Polygon points on the 2D polygon
	 * @param Height height of the extrusion
	 * @param bCapped if true, endcaps of the extrusion are triangulated
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|ShapeGenerators") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* AppendExtrusion(TArray<FVector2D> Polygon, float Height = 10.0, bool bCapped = true);


	/**
	 * Append another GeneratedMesh multiple times, applying a Transform to each repetition.
	 * Note that the Append Transform is *not* applied in this case.
	 * @param OtherMesh the mesh to append
	 * @param Transform the transform to repeatedly apply at each iteration
	 * @param RepeatCount the number of times to append the OtherMesh
	 * @param bApplyBefore if true, we apply Transform to OtherMesh *before* each iteration, otherwise it is applied *after*
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|CompositionOps") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* AppendTiled(UGeneratedMesh* OtherMesh, FTransform Transform, int RepeatCount = 3, bool bApplyBefore = false);

	/** Compute the specified a Boolean operation with OtherMesh */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|CompositionOps") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* BooleanWith(UGeneratedMesh* OtherMesh, EGeneratedMeshBooleanOperation Operation);

	/** Compute the specified a Boolean operation with Transform(OtherMesh) */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|CompositionOps") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* BooleanWithTransformed(UGeneratedMesh* OtherMesh, FTransform Transform, EGeneratedMeshBooleanOperation Operation);


	/** 
	 * Cut the mesh with a 3D plane defined by the Origin and Normal. Positive side is kept.
	 * @param bFillHole if true, any holes created by the cut are triangulated
	 * @param bFlipSide reverse which side of the cut is kept
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|CuttingOps") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* CutWithPlane(FVector Origin, FVector Normal, bool bFillHole = true, bool bFlipSide = false);

	/**
	 * Mirror the mesh across the 3D plane defined by the origin and normal
	 * @param bApplyPlaneCut if true, mesh is cut before mirroring and stitched along cut loops
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|CuttingOps") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* Mirror(FVector Origin, FVector Normal, bool bApplyPlaneCut = true);




	/** Create a "solid" verison of Mesh by voxelizing with the fast winding number at the given grid resolution */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|RemeshingOps") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* SolidifyMesh(int VoxelResolution = 64, float WindingThreshold = 0.5);


	/** Simplify current Mesh to the target triangle count */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|RemeshingOps") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* SimplifyMeshToTriCount(int32 TargetTriangleCount, bool bDiscardAttributes = false);




	/**
	 * Find NearestMeshWorldPoint on SourceMesh to WorldPoint, as well as NearestTriangle ID and barycentric coordinates of NearestMeshWorldPoint in triangle
	 * @return distance to point
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|SpatialQueries")
	float DistanceToPoint(FVector WorldPoint, FVector& NearestMeshWorldPoint, int& NearestTriangle, FVector& TriBaryCoords);

	/**
	 * @return nearest world-space point on SourceMesh to WorldPoint
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|SpatialQueries")
	FVector NearestPoint(FVector WorldPoint);

	/**
	 * @return true if mesh contains WorldPoint, which is defined as the mesh winding number being >= WindingThreshold
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|SpatialQueries")
	bool ContainsPoint(FVector WorldPoint, float WindingThreshold = 0.5);

	/**
	 * Calculate intersection of given 3D World-Space ray defined by (RayOrigin,RayDirection) with the SourceMesh.
	 * If hit, returns WorldHitPoint position, distance along ray in HitDistance, NearestTriangle ID, and barycentric coordinates of hit point in triangle
	 * Pass MaxDistance > 0 to limit the allowable ray-hit distance
	 * @return true if hit is found
	 */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|SpatialQueries")
	bool IntersectRay(FVector RayOrigin, FVector RayDirection, FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle, FVector& TriBaryCoords, float MaxDistance = 0);



	/** Translate the vertices of the Mesh by the given 3D Translation */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|Transforms") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* Translate(FVector Translation);

	/** Rotate the vertices of the Mesh around the Origin point using the given Rotation */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|Transforms") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* Rotate(FRotator Rotation, FVector Origin = FVector(0, 0, 0));

	/** Rotate the vertices of the Mesh around the Origin point using the given Rotation */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|Transforms") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* RotateQuat(FQuat Rotation, FVector Origin = FVector(0,0,0));

	/** Uniform-Scale the vertices of the Mesh relative to the Origin point using the given Scale */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|Transforms") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* ScaleUniform(float Scale, FVector Origin = FVector(0,0,0));

	/** Scale the vertices of the Mesh relative to the Origin point using the given Scale */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|Transforms") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* Scale(FVector Scale, FVector Origin = FVector(0, 0, 0));

	/** Apply the given Transform to the vertices of the Mesh */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|Transforms") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* Transform(FTransform Transform);



	/** Set the normals of the mesh to Per-Triangle/Face Normals */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|Normals") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* SetToFaceNormals();

	/** Set the normals of the mesh to averaged Per-Vertex Normals */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|Normals") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* SetToVertexNormals();

	/** Set the normals of the mesh to hard normals when the edge opening angle exceeds the given Threshold, otherwise to averaged per-Vertex normals */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|Normals") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* SetToAngleThresholdNormals(float AngleThresholdDeg = 180.0f);

	/** Recompute the averaged normals for the current hard-normal topology */
	UFUNCTION(BlueprintCallable, Category = "GeneratedMesh|Normals") UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* RecomputeNormals();


protected:
	FTransform3d AppendTransform;
	TUniquePtr<FDynamicMesh3> Mesh;

	TUniquePtr<FDynamicMeshAABBTree3> MeshAABBTree;
	TUniquePtr<TFastWindingTree<FDynamicMesh3>> FastWinding;

public:
	const TUniquePtr<FDynamicMesh3>& GetMesh() const { return Mesh; }
	TUniquePtr<FDynamicMeshAABBTree3>& GetAABBTree();		// note: cannot return const because query functions are non-const
	const TUniquePtr<TFastWindingTree<FDynamicMesh3>>& GetFastWindingTree();

	void SetMesh(const FDynamicMesh3& MeshIn) { *Mesh = MeshIn; OnMeshUpdated(); }
	void AppendMeshWithAppendTransform(FDynamicMesh3&& ToAppend, bool bPostMeshUpdate = true);

	void OnMeshUpdated();

	// warning: not safe to use AABBTree or FastWindingTree during this function
	virtual void EditMeshInPlace(TFunctionRef<void(FDynamicMesh3&)> EditFunc)
	{
		EditFunc(*Mesh);
		OnMeshUpdated();
	}
};




/**
 * UGeneratedMeshPool manages a Pool of UGeneratedMesh objects. This allows
 * the meshes to be re-used instead of being garbage-collected.
 * 
 * Usage is to call RequestMesh() to take ownership of an available UGeneratedMesh (which
 * will allocate a new one if the pool is empty) and ReturnMesh() to return it to the pool.
 *
 * ReturnAllMeshes() can be called to return all allocated meshes.
 *
 * In both cases, there is nothing preventing you from still holding on to the mesh.
 * So, be careful.
 *
 * FreeAllMeshes() calls ReturnAllMeshes() and then releases the pool's references to
 * the allocated meshes, so they can be Garbage Collected
 * 
 * If you Request() more meshes than you Return(), the Pool will still be holding on to 
 * references to those meshes, and they will never be Garbage Collected (ie memory leak).
 * As a failsafe, if the number of allocated meshes exceeds MeshCountSafetyThreshold, 
 * the Pool will release all it's references and run garbage collection on the next call to RequestMesh(). 
 * (Do not rely on this as a memory management strategy)
 *
 * An alternate strategy that could be employed here is for the Pool to not hold
 * references to meshes it has provided, only those that have been explicitly returned.
 * Then non-returned meshes would simply be garbage-collected, however it allows
 * potentially a large amount of memory to be consumed until that occurs.
 *
 * UGeneratedMesh::ResetMesh() is called on the object returned to the Pool, which clears
 * the internal FDynamicMesh3 (which uses normal C++ memory management, so no garbage collection involved)
 * So the Pool does not re-use mesh memory, only the UObject containers.
 */
UCLASS(Transient)
class RUNTIMEGEOMETRYUTILS_API UGeneratedMeshPool : public UObject
{
	GENERATED_BODY()

public:
	/** @return an available GeneratedMesh from the pool (possibly allocating a new mesh) */
	UFUNCTION(BlueprintCallable)
	UGeneratedMesh* RequestMesh();

	/** Release a GeneratedMesh returned by RequestMesh() back to the pool */
	UFUNCTION(BlueprintCallable)
	void ReturnMesh(UGeneratedMesh* Mesh);

	/** Release all GeneratedMeshes back to the pool */
	UFUNCTION(BlueprintCallable)
	void ReturnAllMeshes();

	/** Release all GeneratedMeshes back to the pool and allow them to be garbage collected */
	UFUNCTION(BlueprintCallable)
	void FreeAllMeshes();


protected:
	UPROPERTY()
	int32 MeshCountSafetyThreshold = 1000;

	/** Meshes in the pool that are available */
	UPROPERTY()
	TArray<UGeneratedMesh*> CachedMeshes;

	/** All meshes the pool has allocated */
	UPROPERTY()
	TArray<UGeneratedMesh*> AllCreatedMeshes;
};
