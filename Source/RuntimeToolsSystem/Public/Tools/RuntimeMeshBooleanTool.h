#pragma once

#include "BaseTools/BaseCreateFromSelectedTool.h"
#include "CSGMeshesTool.h"
#include "RuntimeMeshBooleanTool.generated.h"


UENUM(BlueprintType)
enum class ERuntimeMeshBooleanOpType : uint8
{
	DifferenceAB = 0,
	DifferenceBA = 1,
	Intersect = 2,
	Union = 3,
	TrimA = 4,
	TrimB = 5
};


UCLASS(BlueprintType)
class RUNTIMETOOLSSYSTEM_API URuntimeMeshBooleanToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	int OperationType;
};


UCLASS(BlueprintType)
class RUNTIMETOOLSSYSTEM_API URuntimeMeshBooleanTool : public UCSGMeshesTool
{
	GENERATED_BODY()

public:
	virtual void Setup() override;

	virtual void Shutdown(EToolShutdownType ShutdownType) override;

	UPROPERTY(BlueprintReadOnly)
	URuntimeMeshBooleanToolProperties* RuntimeProperties;
};



UCLASS()
class RUNTIMETOOLSSYSTEM_API URuntimeMeshBooleanToolBuilder : public UCSGMeshesToolBuilder
{
	GENERATED_BODY()
public:
	virtual UBaseCreateFromSelectedTool* MakeNewToolInstance(UObject* Outer) const override
	{
		return NewObject<URuntimeMeshBooleanTool>(Outer);
	}
};
