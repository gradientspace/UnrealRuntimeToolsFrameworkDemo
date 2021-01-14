
#include "SceneObjectSelectionInteraction.h"
#include "MeshScene/RuntimeMeshSceneSubsystem.h"


void USceneObjectSelectionInteraction::Initialize(TUniqueFunction<bool()> CanChangeSelectionCallbackIn)
{
	CanChangeSelectionCallback = MoveTemp(CanChangeSelectionCallbackIn);

	// create click behavior and set ourselves as click target
	ClickBehavior = NewObject<USingleClickInputBehavior>();
	ClickBehavior->Modifiers.RegisterModifier(AddToSelectionModifier, FInputDeviceState::IsShiftKeyDown);
	ClickBehavior->Modifiers.RegisterModifier(ToggleSelectionModifier, FInputDeviceState::IsCtrlKeyDown);
	ClickBehavior->Initialize(this);

	BehaviorSet = NewObject<UInputBehaviorSet>();
	BehaviorSet->Add(ClickBehavior, this);
}


void USceneObjectSelectionInteraction::OnUpdateModifierState(int ModifierID, bool bIsOn)
{
	// update modifier state flags
	if (ModifierID == AddToSelectionModifier)
	{
		bAddToSelectionEnabled = bIsOn;
	}
	else if (ModifierID == ToggleSelectionModifier)
	{
		bToggleSelectionEnabled = bIsOn;
	}
}


FInputRayHit USceneObjectSelectionInteraction::IsHitByClick(const FInputDeviceRay& ClickPos)
{
	FInputRayHit RayHit;

	if (CanChangeSelectionCallback() == false)
	{
		return RayHit;
	}

	FVector HitPoint, BaryCoords;
	float HitDist;
	int32 HitTri;
	URuntimeMeshSceneObject* HitObject = URuntimeMeshSceneSubsystem::Get()->FindNearestHitObject(
		ClickPos.WorldRay.Origin, ClickPos.WorldRay.Direction, HitPoint, HitDist, HitTri, BaryCoords);

	if (HitObject != nullptr)
	{
		RayHit.bHit = true;
		RayHit.HitDepth = HitDist;
		//RayHit.HitNormal = ;			// todo - can compute from bary coords
		//RayHit.bHasHitNormal = ;		// todo - can compute from bary coords
		RayHit.HitIdentifier = HitTri;
		RayHit.HitOwner = HitObject;
	}
	else 
	{
		RayHit.bHit = true;
		RayHit.HitDepth = TNumericLimits<float>::Max();
		RayHit.HitIdentifier = 0;
		RayHit.HitOwner = this;
	}
	return RayHit;
}

void USceneObjectSelectionInteraction::OnClicked(const FInputDeviceRay& ClickPos)
{
	FVector HitPoint, BaryCoords;
	float HitDist;
	int32 HitTri;
	URuntimeMeshSceneObject* HitObject = URuntimeMeshSceneSubsystem::Get()->FindNearestHitObject(
		ClickPos.WorldRay.Origin, ClickPos.WorldRay.Direction, HitPoint, HitDist, HitTri, BaryCoords);

	if (HitObject != nullptr)
	{
		if (bAddToSelectionEnabled)
		{
			URuntimeMeshSceneSubsystem::Get()->SetSelected(HitObject, false, false);
		}
		else if (bToggleSelectionEnabled)
		{
			URuntimeMeshSceneSubsystem::Get()->ToggleSelected(HitObject);
		}
		else
		{
			URuntimeMeshSceneSubsystem::Get()->SetSelected(HitObject, false, true);
		}
	}
	else
	{
		URuntimeMeshSceneSubsystem::Get()->ClearSelection();
	}

}
