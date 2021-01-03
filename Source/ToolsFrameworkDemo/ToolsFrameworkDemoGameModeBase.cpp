// Copyright Epic Games, Inc. All Rights Reserved.


#include "ToolsFrameworkDemoGameModeBase.h"

#include "RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h"
#include "MeshScene/RuntimeMeshSceneSubsystem.h"

#include "AddPrimitiveTool.h"
#include "DrawAndRevolveTool.h"
#include "MeshVertexSculptTool.h"
#include "DynamicMeshSculptTool.h"

#include "Tools/RuntimeDrawPolygonTool.h"

AToolsFrameworkDemoGameModeBase::AToolsFrameworkDemoGameModeBase()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}


void AToolsFrameworkDemoGameModeBase::StartPlay()
{
	Super::StartPlay();

	UWorld* World = GetWorld();
	check(World);

	UGameInstance* GameInstance = GetGameInstance();
	SceneSystem = UGameInstance::GetSubsystem<URuntimeMeshSceneSubsystem>(GameInstance);
	URuntimeMeshSceneSubsystem::InitializeSingleton(SceneSystem);

	ToolsSystem = UGameInstance::GetSubsystem<URuntimeToolsFrameworkSubsystem>(GameInstance);
	URuntimeToolsFrameworkSubsystem::InitializeSingleton(ToolsSystem);
	check(ToolsSystem);

	ToolsSystem->InitializeToolsContext(World);
	SceneSystem->SetCurrentTransactionsAPI(ToolsSystem->GetTransactionsAPI());

	UInteractiveToolManager* ToolManager = ToolsSystem->ToolsContext->ToolManager;

	auto AddPrimitiveToolBuilder = NewObject<UAddPrimitiveToolBuilder>();
	AddPrimitiveToolBuilder->AssetAPI = ToolsSystem->GetAssetAPI();
	AddPrimitiveToolBuilder->ShapeType = UAddPrimitiveToolBuilder::EMakeMeshShapeType::Box;
	ToolsSystem->ToolsContext->ToolManager->RegisterToolType("AddPrimitiveBox", AddPrimitiveToolBuilder);


	auto DrawPolygonToolBuilder = NewObject<URuntimeDrawPolygonToolBuilder>();
	DrawPolygonToolBuilder->AssetAPI = ToolsSystem->GetAssetAPI();
	ToolsSystem->ToolsContext->ToolManager->RegisterToolType("DrawPolygon", DrawPolygonToolBuilder);

	auto PolyRevolveToolBuilder = NewObject<UDrawAndRevolveToolBuilder>();
	PolyRevolveToolBuilder->AssetAPI = ToolsSystem->GetAssetAPI();
	ToolsSystem->ToolsContext->ToolManager->RegisterToolType("PolyRevolve", PolyRevolveToolBuilder);

	auto VertexSculptToolBuilder = NewObject<UMeshVertexSculptToolBuilder>();
	ToolsSystem->ToolsContext->ToolManager->RegisterToolType("VertexSculpt", VertexSculptToolBuilder);

	auto DynaSculptToolBuilder = NewObject<UDynamicMeshSculptToolBuilder>();
	DynaSculptToolBuilder->bEnableRemeshing = true;
	ToolsSystem->ToolsContext->ToolManager->RegisterToolType("DynaSculpt", DynaSculptToolBuilder);

	UE_LOG(LogTemp, Warning, TEXT("STARTED PLAY!"));
}



void AToolsFrameworkDemoGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ToolsSystem)
	{
		ToolsSystem->Tick(DeltaTime);
	}
}