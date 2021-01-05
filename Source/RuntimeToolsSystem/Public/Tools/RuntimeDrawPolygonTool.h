#pragma once

#include "DrawPolygonTool.h"
#include "RuntimeDrawPolygonTool.generated.h"


/** ToolBuilder for URuntimeDrawPolygonTool instances */
UCLASS()
class RUNTIMETOOLSSYSTEM_API URuntimeDrawPolygonToolBuilder : public UDrawPolygonToolBuilder
{
	GENERATED_BODY()

public:
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};


UENUM(BlueprintType)
enum class ERuntimeDrawPolygonType : uint8
{
	Freehand = 0,
	Circle = 1,
	Square = 2,
	Rectangle = 3,
	RoundedRectangle = 4,
	HoleyCircle = 5
};


UCLASS(BlueprintType)
class RUNTIMETOOLSSYSTEM_API URuntimeDrawPolygonToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	int SelectedPolygonType;
};




/**
 * Extension of UDrawPolygonTool that overrides EmitCurrentPolygon() to work at Runtime,
 * because the base implementation calls checkNoEntry() in non-Editor builds.
 */
UCLASS(BlueprintType)
class RUNTIMETOOLSSYSTEM_API URuntimeDrawPolygonTool : public UDrawPolygonTool
{
	GENERATED_BODY()

public:
	virtual void Setup() override;

	virtual void EmitCurrentPolygon() override;

	UPROPERTY(BlueprintReadOnly)
	URuntimeDrawPolygonToolProperties* RuntimeProperties;
};