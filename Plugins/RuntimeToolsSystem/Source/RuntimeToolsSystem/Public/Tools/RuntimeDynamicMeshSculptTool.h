#pragma once

#include "DynamicMeshSculptTool.h"
#include "RuntimeDynamicMeshSculptTool.generated.h"

UCLASS()
class RUNTIMETOOLSSYSTEM_API URuntimeDynamicMeshSculptToolBuilder : public UDynamicMeshSculptToolBuilder
{
	GENERATED_BODY()
public:
	virtual UMeshSurfacePointTool* CreateNewTool(const FToolBuilderState& SceneState) const override;
};


UENUM(BlueprintType)
enum class ERuntimeDynamicMeshSculptBrushType : uint8
{
	Move = 0,
	Sculpt = 1,
	Smooth = 2,
	Inflate = 3,
	Flatten = 4
};


UCLASS(BlueprintType)
class RUNTIMETOOLSSYSTEM_API URuntimeDynamicMeshSculptToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	float BrushSize;

	UPROPERTY(BlueprintReadWrite)
	float BrushStrength;

	UPROPERTY(BlueprintReadWrite)
	float BrushFalloff;

	UPROPERTY(BlueprintReadWrite)
	int SelectedBrushType;
};


UCLASS(BlueprintType)
class RUNTIMETOOLSSYSTEM_API URuntimeDynamicMeshSculptTool : public UDynamicMeshSculptTool
{
	GENERATED_BODY()

public:
	virtual void Setup() override;

	UPROPERTY(BlueprintReadOnly)
	URuntimeDynamicMeshSculptToolProperties* RuntimeProperties;
};