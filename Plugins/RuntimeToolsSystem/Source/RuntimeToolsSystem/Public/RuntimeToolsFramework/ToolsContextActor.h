#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "ToolsContextActor.generated.h"

class URuntimeToolsFrameworkSubsystem;

/**
 * Camera-control modes that AToolsContextActor has implemented
 */
UENUM()
enum class EToolActorInteractionMode : uint8
{
	NoInteraction,
	RightMouseCameraControl,
	AltCameraControl
};

/**
 * AToolsContextActor is the Pawn used in the AToolsFrameworkDemoGameModeBase.
 * This Game Mode initializes the URuntimeMeshSceneSubsystem and URuntimeToolsFrameworkSubsystem.
 * Essentially, this Actor has two jobs:
 *
 * 1) to forward Input events (from the PlayerInputComponent) to these subsystems,
 * both mouse events and Actions configured in the project settings.
 * 
 * 2) to implement "camera control", as the rendering is done from this actors position (ie as a first-person view).
 * The base ADefaultPawn implements standard right-mouse-fly controls, and so if right mouse is held down,
 * we just call those functions. In addition, Hold-alt camera controls are also done here, rather
 * than using the ITF. (*However*, the alt-controls are not fully implemented, ie it is not Maya-style alt+3-mouse-button controls)
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API AToolsContextActor : public ADefaultPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AToolsContextActor();

	// set in PossessedBy(), we keep track of this so that the ToolsFramework can access it to figure out mouse cursor rays
	UPROPERTY()
	APlayerController* PlayerController;

	// return current interaction mode, ie are we in a camera interaction or not
	UFUNCTION(BlueprintCallable)
	EToolActorInteractionMode GetCurrentInteractionMode() { return CurrentInteractionMode; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// called when despawned
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// called on startup
	virtual void PossessedBy(AController* inController) override;


protected:
	URuntimeToolsFrameworkSubsystem* ToolsSystem;

	EToolActorInteractionMode CurrentInteractionMode = EToolActorInteractionMode::NoInteraction;
	bool bIsRightMouseDown = false;
	bool bIsMiddleMouseDown = false;
	bool bIsLeftMouseDown = false;
	bool bIsAltKeyDown = false;

	virtual void OnLeftMouseDown();
	virtual void OnLeftMouseUp();

	virtual void OnMiddleMouseDown();
	virtual void OnMiddleMouseUp();

	virtual void OnRightMouseDown();
	virtual void OnRightMouseUp();

	virtual void OnAltKeyDown();
	virtual void OnAltKeyUp();

	virtual void OnToolAccept();
	virtual void OnToolExit();

	virtual void OnDelete();
	virtual void OnUndo();
	virtual void OnRedo();

	virtual void OnMoveForwardKeyAxis(float MoveDelta);
	virtual void OnMoveRightKeyAxis(float MoveDelta);
	virtual void OnMoveUpKeyAxis(float MoveDelta);

	virtual void OnMouseMoveX(float MoveX);
	virtual void OnMouseMoveY(float MoveY);
};
