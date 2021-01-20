#include "RuntimePolyEditTool.h"
#include "RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h"

#include "ToolBuilderUtil.h"

#define LOCTEXT_NAMESPACE "URuntimeEditMeshPolygonsTool"


UMeshSurfacePointTool* URuntimePolyEditToolBuilder::CreateNewTool(const FToolBuilderState& SceneState) const
{
	URuntimePolyEditTool* PolyEditTool = NewObject<URuntimePolyEditTool>(SceneState.ToolManager);
	return PolyEditTool;
}




void URuntimePolyEditTool::Setup()
{
	UEditMeshPolygonsTool::Setup();

	// mirror properties we want to expose at runtime 
	RuntimeProperties = NewObject<URuntimePolyEditToolProperties>(this);

	AddToolPropertySource(RuntimeProperties);


	// hack to workaround garbage collection bug in 4.26.0
	DynamicMeshComponent->GetSecondaryRenderMaterial()->AddToRoot();

	check(GEngine->WireframeMaterial != nullptr);
}


void URuntimePolyEditTool::BeginExtrudeAction()
{
	RequestAction(EEditMeshPolygonsToolActions::Extrude);
}

void URuntimePolyEditTool::BeginInsetAction()
{
	RequestAction(EEditMeshPolygonsToolActions::Inset);
}


void URuntimePolyEditTool::BeginOutsetAction()
{
	RequestAction(EEditMeshPolygonsToolActions::Outset);
}

void URuntimePolyEditTool::BeginCutFacesAction()
{
	RequestAction(EEditMeshPolygonsToolActions::CutFaces);
}


#undef LOCTEXT_NAMESPACE