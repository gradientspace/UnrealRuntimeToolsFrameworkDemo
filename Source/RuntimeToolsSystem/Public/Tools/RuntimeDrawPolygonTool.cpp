
#include "RuntimeDrawPolygonTool.h"
#include "RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h"
#include "MeshScene/RuntimeMeshSceneSubsystem.h"

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






void URuntimeDrawPolygonTool::Setup()
{
	UDrawPolygonTool::Setup();

	// initialize to drawing material
	this->MaterialProperties->Material = URuntimeMeshSceneSubsystem::Get()->StandardMaterial;

	// mirror properties we want to expose at runtime 
	RuntimeProperties = NewObject<URuntimeDrawPolygonToolProperties>(this);

	RuntimeProperties->SelectedPolygonType = (int)PolygonProperties->PolygonType;
	RuntimeProperties->WatchProperty(RuntimeProperties->SelectedPolygonType,
		[this](int NewType) { PolygonProperties->PolygonType = (EDrawPolygonDrawMode)NewType; });

	AddToolPropertySource(RuntimeProperties);
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