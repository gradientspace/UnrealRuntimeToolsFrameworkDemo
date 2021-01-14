// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InteractiveToolsContext.h"

#include "ToolsContextRenderComponent.h"
#include "MeshScene/SceneHistoryManager.h"
#include "Interaction/SceneObjectSelectionInteraction.h"
#include "Interaction/SceneObjectTransformInteraction.h"

#include "RuntimeToolsFrameworkSubsystem.generated.h"


class FRuntimeToolsContextQueriesImpl;
class FRuntimeToolsContextTransactionImpl;
class FRuntimeToolsContextAssetImpl;
class AToolsContextActor;

/**
 * 
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API URuntimeToolsFrameworkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	//
	// Small hack to workaround the fact that you generally need the UGameInstance pointer to
	// look up a GameInstance subsystem. We store the pointer and then allow ::Get() to return it (ie actually a Singleton)
	//
public:
	static void InitializeSingleton(URuntimeToolsFrameworkSubsystem* Subsystem);
	static URuntimeToolsFrameworkSubsystem* Get();
protected:
	static URuntimeToolsFrameworkSubsystem* InstanceSingleton;


	//
	// UGameInstanceSubsystem API implementation
	//
public:
	virtual void Deinitialize() override;


	//
	// Functions to setup/shutdown/operate the RuntimeToolsFramework
	//
public:
	void InitializeToolsContext(UWorld* TargetWorld);
	void ShutdownToolsContext();
	void SetContextActor(AToolsContextActor* ActorIn);
	virtual void Tick(float DeltaTime);


	//
	// Access to various data structures created/tracked by the Subsystem
	//

	IToolsContextTransactionsAPI* GetTransactionsAPI();
	IToolsContextAssetAPI* GetAssetAPI();

	UFUNCTION(BlueprintCallable)
	USceneHistoryManager* GetSceneHistory() { return SceneHistory;  }

	TArray<UObject*> GetActiveToolPropertySets();


	//
	// Tool creation/management BP API
	//

	UFUNCTION(BlueprintCallable)
	bool CanActivateToolByName(FString Name);

	UFUNCTION(BlueprintCallable)
	UInteractiveTool* BeginToolByName(FString Name);

	UFUNCTION(BlueprintCallable)
	bool HaveActiveTool();

	UFUNCTION(BlueprintCallable)
	UInteractiveTool* GetActiveTool();

	UFUNCTION(BlueprintCallable)
	bool IsActiveToolAcceptCancelType();

	UFUNCTION(BlueprintCallable)
	bool CanAcceptActiveTool();

	UFUNCTION(BlueprintCallable)
	bool AcceptActiveTool();

	UFUNCTION(BlueprintCallable)
	bool CancelOrCompleteActiveTool();




	//
	// Support for tracking World/Local coordinate system global state
	//

	UPROPERTY()
	EToolContextCoordinateSystem CurrentCoordinateSystem = EToolContextCoordinateSystem::World;

	//UFUNCTION(BlueprintCallable)		// unsupported because EToolContextCoordinateSystem is not a uint8
	EToolContextCoordinateSystem GetCurrentCoordinateSystem() const { return CurrentCoordinateSystem; }

	//UFUNCTION(BlueprintCallable)		// unsupported because EToolContextCoordinateSystem is not a uint8
	void SetCurrentCoordinateSystem(EToolContextCoordinateSystem CoordSystem);

	UFUNCTION(BlueprintCallable)
	void CycleCurrentCoordinateSystem();

	UFUNCTION(BlueprintCallable)
	bool IsWorldCoordinateSystem() { return CurrentCoordinateSystem == EToolContextCoordinateSystem::World; }



	//
	// random utility BP functions
	//

	UFUNCTION(BlueprintCallable)
	URuntimeMeshSceneObject* ImportMeshSceneObject(const FString Path, bool bFlipOrientation);



	//
	// mouse state queries/functions
	//

public:
	UFUNCTION(BlueprintCallable)
	bool IsCapturingMouse() const;
protected:
	void OnLeftMouseDown();
	void OnLeftMouseUp();


public:
	UPROPERTY()
	UWorld* TargetWorld;

	UPROPERTY()
	UInteractiveToolsContext* ToolsContext;

	UPROPERTY()
	AToolsContextActor* ContextActor;

	UPROPERTY()
	AActor* PDIRenderActor;

	UPROPERTY()
	UToolsContextRenderComponent* PDIRenderComponent;

	UPROPERTY()
	USceneObjectSelectionInteraction* SelectionInteraction;

	UPROPERTY()
	USceneObjectTransformInteraction* TransformInteraction;

	UPROPERTY()
	USceneHistoryManager* SceneHistory;


protected:
	TSharedPtr<FRuntimeToolsContextQueriesImpl> ContextQueriesAPI;
	TSharedPtr<FRuntimeToolsContextTransactionImpl> ContextTransactionsAPI;
	TSharedPtr<FRuntimeToolsContextAssetImpl> ContextAssetAPI;

	void InternalConsistencyChecks();
	bool bIsShuttingDown = false;

	void OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool);
	void OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool);

	void OnSceneHistoryStateChange();


	// mouse things

	friend class AToolsContextActor;

	FVector2D PrevMousePosition = FVector2D::ZeroVector;

	FInputDeviceState CurrentMouseState;
	bool bPendingMouseStateChange = false;

	FViewCameraState CurrentViewCameraState;


	// property set keepalive hack

	void AddPropertySetKeepalive(UInteractiveToolPropertySet* PropertySet);
	void AddAllPropertySetKeepalives(UInteractiveTool* Tool);

	UPROPERTY()
	TArray<UObject*> PropertySetKeepAlives;

};
