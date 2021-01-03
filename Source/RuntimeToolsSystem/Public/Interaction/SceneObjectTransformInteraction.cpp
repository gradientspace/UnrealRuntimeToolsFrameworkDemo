
#include "SceneObjectTransformInteraction.h"

#include "MeshScene/RuntimeMeshSceneSubsystem.h"
#include "RuntimeToolsFramework/RuntimeToolsFrameworkSubsystem.h"

#include "BaseGizmos/TransformGizmo.h"
#include "BaseGizmos/TransformProxy.h"

void USceneObjectTransformInteraction::Initialize(TUniqueFunction<bool()> GizmoEnabledCallbackIn)
{
	GizmoEnabledCallback = MoveTemp(GizmoEnabledCallbackIn);

	SelectionChangedEventHandle = URuntimeMeshSceneSubsystem::Get()->OnSelectionModified.AddLambda([this](URuntimeMeshSceneSubsystem* SceneSubsystem)
	{
		UpdateGizmoTargets(SceneSubsystem->GetSelection());
	});

}

void USceneObjectTransformInteraction::Shutdown()
{
	if (SelectionChangedEventHandle.IsValid())
	{
		if (URuntimeMeshSceneSubsystem::Get())
		{
			URuntimeMeshSceneSubsystem::Get()->OnSelectionModified.Remove(SelectionChangedEventHandle);
		}
		SelectionChangedEventHandle = FDelegateHandle();
	}

	TArray<URuntimeMeshSceneObject*> EmptySelection;
	UpdateGizmoTargets(EmptySelection);
}


void USceneObjectTransformInteraction::SetEnableScaling(bool bEnable)
{
	if (bEnable != bEnableScaling)
	{
		bEnableScaling = bEnable;
		ForceUpdateGizmoState();
	}
}

void USceneObjectTransformInteraction::SetEnableNonUniformScaling(bool bEnable)
{
	if (bEnable != bEnableNonUniformScaling)
	{
		bEnableNonUniformScaling = bEnable;
		ForceUpdateGizmoState();
	}
}

void USceneObjectTransformInteraction::ForceUpdateGizmoState()
{
	if (URuntimeMeshSceneSubsystem::Get())
	{
		UpdateGizmoTargets(URuntimeMeshSceneSubsystem::Get()->GetSelection());
	}
}


void USceneObjectTransformInteraction::UpdateGizmoTargets(const TArray<URuntimeMeshSceneObject*>& Selection)
{
	UInteractiveGizmoManager* GizmoManager = URuntimeToolsFrameworkSubsystem::Get()->ToolsContext->GizmoManager;

	// destroy existing gizmos if we have any
	if (TransformGizmo != nullptr)
	{
		GizmoManager->DestroyAllGizmosByOwner(this);
		TransformGizmo = nullptr;
		TransformProxy = nullptr;
	}

	// if no selection, no gizmo
	if (Selection.Num() == 0 || GizmoEnabledCallback() == false )
	{
		return;
	}

	TransformProxy = NewObject<UTransformProxy>(this);
	for (URuntimeMeshSceneObject* SO : Selection)
	{
		// would be nice if this worked on Actors...
		TransformProxy->AddComponent(SO->GetMeshComponent());
	}

	ETransformGizmoSubElements GizmoElements = ETransformGizmoSubElements::FullTranslateRotateScale;
	if (bEnableScaling == false)
	{
		GizmoElements = ETransformGizmoSubElements::StandardTranslateRotate;
	}
	else if (bEnableNonUniformScaling == false || Selection.Num() > 1)		// cannot nonuniform-scale multiple objects
	{
		GizmoElements = ETransformGizmoSubElements::TranslateRotateUniformScale;
	}

	TransformGizmo = GizmoManager->CreateCustomTransformGizmo(GizmoElements, this);
	TransformGizmo->SetActiveTarget(TransformProxy);

	// optionally ignore coordinate system setting
	//TransformGizmo->bUseContextCoordinateSystem = false;
	//TransformGizmo->CurrentCoordinateSystem = EToolContextCoordinateSystem::Local;
}