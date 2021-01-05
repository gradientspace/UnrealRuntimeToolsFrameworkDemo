#pragma once

#include "RemeshMeshTool.h"
#include "RuntimeRemeshMeshTool.generated.h"

/** ToolBuilder for URuntimeDrawPolygonTool instances */
UCLASS()
class RUNTIMETOOLSSYSTEM_API URuntimeRemeshMeshToolBuilder : public URemeshMeshToolBuilder
{
	GENERATED_BODY()
public:
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};



UCLASS(BlueprintType)
class RUNTIMETOOLSSYSTEM_API URuntimeRemeshMeshToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	int TargetTriangleCount;

	UPROPERTY(BlueprintReadWrite)
	bool bDiscardAttributes;
};


UCLASS(BlueprintType)
class RUNTIMETOOLSSYSTEM_API URuntimeRemeshMeshTool : public URemeshMeshTool
{
	GENERATED_BODY()

public:
	virtual void Setup() override;

	UPROPERTY(BlueprintReadOnly)
	URuntimeRemeshMeshToolProperties* RuntimeProperties;
};