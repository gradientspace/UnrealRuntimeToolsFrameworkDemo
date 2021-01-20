#include "ToolsFrameworkDemoGameModeBase.h"

#include "RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h"
#include "MeshScene/RuntimeMeshSceneSubsystem.h"

#include "AddPrimitiveTool.h"
#include "DrawAndRevolveTool.h"
#include "MeshVertexSculptTool.h"
#include "PlaneCutTool.h"

#include "Tools/RuntimeDrawPolygonTool.h"
#include "Tools/RuntimeDynamicMeshSculptTool.h"
#include "Tools/RuntimeRemeshMeshTool.h"
#include "Tools/RuntimeMeshBooleanTool.h"
#include "Tools/RuntimePolyEditTool.h"


AToolsFrameworkDemoGameModeBase::AToolsFrameworkDemoGameModeBase()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}


void AToolsFrameworkDemoGameModeBase::StartPlay()
{
	Super::StartPlay();
	InitializeToolsSystem();
}


void AToolsFrameworkDemoGameModeBase::InitializeToolsSystem()
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = GetGameInstance();
	check(World && GameInstance);

	// create Scene subsystem
	SceneSystem = UGameInstance::GetSubsystem<URuntimeMeshSceneSubsystem>(GameInstance);
	URuntimeMeshSceneSubsystem::InitializeSingleton(SceneSystem);

	// create Tools subsystem
	ToolsSystem = UGameInstance::GetSubsystem<URuntimeToolsFrameworkSubsystem>(GameInstance);
	URuntimeToolsFrameworkSubsystem::InitializeSingleton(ToolsSystem);

	check(SceneSystem && ToolsSystem);

	// initialize Tools and Scene systems
	ToolsSystem->InitializeToolsContext(World);
	SceneSystem->SetCurrentTransactionsAPI(ToolsSystem->GetTransactionsAPI());

	RegisterTools();
}


void AToolsFrameworkDemoGameModeBase::RegisterTools()
{
	UInteractiveToolManager* ToolManager = ToolsSystem->ToolsContext->ToolManager;

	auto AddPrimitiveToolBuilder = NewObject<UAddPrimitiveToolBuilder>();
	AddPrimitiveToolBuilder->AssetAPI = ToolsSystem->GetAssetAPI();
	AddPrimitiveToolBuilder->ShapeType = UAddPrimitiveToolBuilder::EMakeMeshShapeType::Box;
	ToolManager->RegisterToolType("AddPrimitiveBox", AddPrimitiveToolBuilder);

	auto DrawPolygonToolBuilder = NewObject<URuntimeDrawPolygonToolBuilder>();
	DrawPolygonToolBuilder->AssetAPI = ToolsSystem->GetAssetAPI();
	ToolManager->RegisterToolType("DrawPolygon", DrawPolygonToolBuilder);

	auto PolyRevolveToolBuilder = NewObject<UDrawAndRevolveToolBuilder>();
	PolyRevolveToolBuilder->AssetAPI = ToolsSystem->GetAssetAPI();
	ToolManager->RegisterToolType("PolyRevolve", PolyRevolveToolBuilder);

	auto PolyEditToolBuilder = NewObject<URuntimePolyEditToolBuilder>();
	ToolManager->RegisterToolType("EditPolygons", PolyEditToolBuilder);

	auto MeshPlaneCutToolBuilder = NewObject<UPlaneCutToolBuilder>();
	MeshPlaneCutToolBuilder->AssetAPI = ToolsSystem->GetAssetAPI();
	ToolManager->RegisterToolType("PlaneCut", MeshPlaneCutToolBuilder);

	auto RemeshMeshToolBuilder = NewObject<URuntimeRemeshMeshToolBuilder>();
	RemeshMeshToolBuilder->AssetAPI = ToolsSystem->GetAssetAPI();
	ToolManager->RegisterToolType("RemeshMesh", RemeshMeshToolBuilder);

	auto VertexSculptToolBuilder = NewObject<UMeshVertexSculptToolBuilder>();
	ToolManager->RegisterToolType("VertexSculpt", VertexSculptToolBuilder);

	auto DynaSculptToolBuilder = NewObject<URuntimeDynamicMeshSculptToolBuilder>();
	DynaSculptToolBuilder->bEnableRemeshing = true;
	ToolManager->RegisterToolType("DynaSculpt", DynaSculptToolBuilder);

	auto MeshBooleanToolBuilder = NewObject<URuntimeMeshBooleanToolBuilder>();
	MeshBooleanToolBuilder->AssetAPI = ToolsSystem->GetAssetAPI();
	ToolManager->RegisterToolType("MeshBoolean", MeshBooleanToolBuilder);
}


void AToolsFrameworkDemoGameModeBase::ShutdownToolsSystem()
{
	// TODO: implement this
	check(false);
}


void AToolsFrameworkDemoGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ToolsSystem)
	{
		ToolsSystem->Tick(DeltaTime);
	}
}



