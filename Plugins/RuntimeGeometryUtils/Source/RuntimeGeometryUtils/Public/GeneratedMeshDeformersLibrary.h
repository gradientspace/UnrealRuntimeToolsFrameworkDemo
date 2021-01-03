
#include "Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "GeneratedMesh.h"
#include "GeneratedMeshDeformersLibrary.generated.h"


/**
 * A BP Library of functions for applying deformations to the vertices of a UGeneratedMesh
 */
UCLASS(meta = (ScriptName = "GeneratedMeshDeformersLibrary"))
class UGeneratedMeshDeformersLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/**
	 * Displace the mesh vertices using a 1D Sin wave.
	 * The displacement is based on a parameter t measured along the projection Axis.
	 * Once t is known, the new position is CurPos + Magnitude * Sin( (t*Frequency) + FrequencyShift ) * UpVector
	 */
	UFUNCTION(BlueprintCallable) static UPARAM(DisplayName = "Input Mesh") 
	UGeneratedMesh* DeformMeshAxisSinWave1D(UGeneratedMesh* Mesh, float Magnitude = 1, float Frequency = 1, float FrequencyShift = 0, FVector Axis = FVector(1,0,0), FVector UpVector = FVector(0,0,1));

	/**
	 * Displace the mesh vertices using a 2D Sin wave.
	 */
	UFUNCTION(BlueprintCallable) static UPARAM(DisplayName = "Input Mesh") 
	UGeneratedMesh* DeformMeshAxisSinWaveRadial(UGeneratedMesh* Mesh, float Magnitude = 1, float Frequency = 1, float FrequencyShift = 0, FVector Axis = FVector(1,0,0));

	/**
	 * Displace the mesh vertices along their vertex normal directions using 3D Perlin Noise
	 */
	UFUNCTION(BlueprintCallable) static UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* DeformMeshPerlinNoiseNormal(UGeneratedMesh* Mesh, float Magnitude = 1, float Frequency = 1, FVector FrequencyShift = FVector(0,0,0), int RandomSeed = 31337);

	/**
	 * Apply N iterations of explicit uniform Laplacian mesh smoothing to the vertex positions, with the given Alpha in range [0,1]. Clamps to max 100 iterations.
	 */
	UFUNCTION(BlueprintCallable) static UPARAM(DisplayName = "Input Mesh")
	UGeneratedMesh* SmoothMeshUniform(UGeneratedMesh* Mesh, float Alpha = 0.3, int32 Iterations = 1);


};