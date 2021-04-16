#pragma once

#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "InputBehaviorSet.h"
#include "SceneObjectSelectionInteraction.generated.h"


/**
 * USceneObjectSelectionInteraction implements standard mouse-click selection interaction of 
 * "Scene Objects", ie the URuntimeMeshSceneObject's in the current URuntimeMeshSceneSubsystem.
 * 
 * - Left-Click on object changes active selection to that object
 * - Left-Click on "background" clears active selection
 * - Shift+Click modifier adds to selection
 * - Ctrl+Click modifier toggles selected/deselected
 *
 * Currently hover is not supported, but this would be relatively easy to add
 */
UCLASS()
class USceneObjectSelectionInteraction : public UObject, public IInputBehaviorSource, public IClickBehaviorTarget
{
	GENERATED_BODY()

public:

	/**
	 * Set up the Interaction, creates and registers Behaviors/etc. 
	 * @param CanChangeSelectionCallbackIn this function will be called to determine if the current Selection is allowed to be modified (for example, when a Tool is active, we may wish to lock selection)
	 */
	void Initialize(TUniqueFunction<bool()> CanChangeSelectionCallbackIn);



public:
	// click-to-select behavior
	UPROPERTY()
	USingleClickInputBehavior* ClickBehavior;

	// set of all behaviors, will be passed up to UInputRouter
	UPROPERTY()
	UInputBehaviorSet* BehaviorSet;



public:
	//
	// IInputBehaviorSource API
	//
	virtual const UInputBehaviorSet* GetInputBehaviors() const
	{
		return BehaviorSet;
	}

	//
	// IClickBehaviorTarget implementation
	//
	virtual FInputRayHit IsHitByClick(const FInputDeviceRay& ClickPos) override;
	virtual void OnClicked(const FInputDeviceRay& ClickPos) override;


	//
	// IModifierToggleBehaviorTarget implementation
	//
	virtual void OnUpdateModifierState(int ModifierID, bool bIsOn) override;


protected:

	// default change-selection callback always allows selection change
	TUniqueFunction<bool()> CanChangeSelectionCallback = []() { return true; };

	// flags used to identify behavior modifier keys/buttons
	static const int AddToSelectionModifier = 1;
	bool bAddToSelectionEnabled = false;

	static const int ToggleSelectionModifier = 2;
	bool bToggleSelectionEnabled = false;

};