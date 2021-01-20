#include "RuntimeMeshBooleanTool.h"
#include "RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h"
#include "MeshScene/RuntimeMeshSceneSubsystem.h"

#include "ToolBuilderUtil.h"
#include "MeshTransforms.h"
#include "DynamicMeshToMeshDescription.h"

#define LOCTEXT_NAMESPACE "URuntimeMeshBooleanTool"


void URuntimeMeshBooleanTool::Setup()
{
	UCSGMeshesTool::Setup();

	this->CSGProperties->bAttemptFixHoles = true;
	// write to first input asset
	this->HandleSourcesProperties->WriteOutputTo = EBaseCreateFromSelectedTargetType::FirstInputAsset;

	// mirror properties we want to expose at runtime 
	RuntimeProperties = NewObject<URuntimeMeshBooleanToolProperties>(this);

	RuntimeProperties->OperationType = (int)CSGProperties->Operation;
	RuntimeProperties->WatchProperty(RuntimeProperties->OperationType,
		[this](int NewType) { CSGProperties->Operation = (ECSGOperation)NewType; Preview->InvalidateResult(); });

	AddToolPropertySource(RuntimeProperties);
}


void URuntimeMeshBooleanTool::Shutdown(EToolShutdownType ShutdownType)
{
	FDynamicMeshOpResult Result = Preview->Shutdown();
	for (auto& ComponentTarget : ComponentTargets)
	{
		ComponentTarget->SetOwnerVisibility(true);
	}

	if (ShutdownType == EToolShutdownType::Accept)
	{
		GetToolManager()->BeginUndoTransaction(GetActionName());

		// Generate the result
		AActor* KeepActor = nullptr;
		TUniquePtr<FPrimitiveComponentTarget>& UpdateTarget = ComponentTargets[0];
		KeepActor = UpdateTarget->GetOwnerActor();

		FTransform3d TargetToWorld = (FTransform3d)UpdateTarget->GetWorldTransform();
		FTransform3d WorldToTarget = TargetToWorld.Inverse();

		FTransform3d ResultTransform = Result.Transform;
		MeshTransforms::ApplyTransform(*Result.Mesh, ResultTransform);
		MeshTransforms::ApplyTransform(*Result.Mesh, WorldToTarget);
		UpdateTarget->CommitMesh([&](const FPrimitiveComponentTarget::FCommitParams& CommitParams)
		{
			FDynamicMeshToMeshDescription Converter;
			Converter.Convert(Result.Mesh.Get(), *CommitParams.MeshDescription);
		});

		URuntimeMeshSceneObject* SO = URuntimeMeshSceneSubsystem::Get()->FindSceneObjectByActor(KeepActor);
		URuntimeMeshSceneSubsystem::Get()->SetSelected(SO, true, false);
		URuntimeMeshSceneSubsystem::Get()->DeleteSelectedSceneObjects(KeepActor);

		GetToolManager()->EndUndoTransaction();
	}

	UInteractiveGizmoManager* GizmoManager = GetToolManager()->GetPairedGizmoManager();
	GizmoManager->DestroyAllGizmosByOwner(this);
}


#undef LOCTEXT_NAMESPACE