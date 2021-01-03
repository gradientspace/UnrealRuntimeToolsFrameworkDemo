
#include "RuntimeDrawPolygonTool.h"
#include "RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h"

#include "Selection/ToolSelectionUtil.h"
#include "AssetGenerationUtil.h"

#define LOCTEXT_NAMESPACE "URuntimeDrawPolygonTool"

UInteractiveTool* URuntimeDrawPolygonToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	URuntimeDrawPolygonTool* NewTool = NewObject<URuntimeDrawPolygonTool>(SceneState.ToolManager);
	NewTool->SetWorld(SceneState.World);
	NewTool->SetAssetAPI(AssetAPI);
	return NewTool;
}



void URuntimeDrawPolygonTool::EmitCurrentPolygon()
{
	FString BaseName = (PolygonProperties->OutputMode == EDrawPolygonOutputMode::MeshedPolygon) ?
		TEXT("Polygon") : TEXT("Extrude");

	// generate new mesh
	FFrame3d PlaneFrameOut;
	FDynamicMesh3 Mesh;
	double ExtrudeDist = (PolygonProperties->OutputMode == EDrawPolygonOutputMode::MeshedPolygon) ?
		0 : PolygonProperties->ExtrudeHeight;
	bool bSucceeded = GeneratePolygonMesh(PolygonVertices, PolygonHolesVertices, &Mesh, PlaneFrameOut, false, ExtrudeDist, false);
	if (!bSucceeded) // somehow made a polygon with no valid triangulation; just throw it away ...
	{
		ResetPolygon();
		return;
	}

	GetToolManager()->BeginUndoTransaction(LOCTEXT("CreatePolygon", "Create Polygon"));

	AActor* NewActor = AssetGenerationUtil::GenerateStaticMeshActor(
		AssetAPI, TargetWorld,
		&Mesh, PlaneFrameOut.ToTransform(), BaseName, MaterialProperties->Material.Get());
	if (NewActor != nullptr)
	{
		ToolSelectionUtil::SetNewActorSelection(GetToolManager(), NewActor);
	}

	GetToolManager()->EndUndoTransaction();

	ResetPolygon();
}

#undef LOCTEXT_NAMESPACE