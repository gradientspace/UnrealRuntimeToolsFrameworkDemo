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

/**
 * Extension of UDrawPolygonTool that overrides EmitCurrentPolygon() to work at Runtime,
 * because the base implementation calls checkNoEntry() in non-Editor builds.
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API URuntimeDrawPolygonTool : public UDrawPolygonTool
{
	GENERATED_BODY()

public:
	virtual void EmitCurrentPolygon() override;
};