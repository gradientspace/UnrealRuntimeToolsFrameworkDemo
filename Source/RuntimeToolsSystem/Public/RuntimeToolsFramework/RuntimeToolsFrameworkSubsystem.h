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
	

public:
	static void InitializeSingleton(URuntimeToolsFrameworkSubsystem* Subsystem);
	static URuntimeToolsFrameworkSubsystem* Get();
protected:
	static URuntimeToolsFrameworkSubsystem* InstanceSingleton;


public:
	virtual void Deinitialize() override;


public:

	void InitializeToolsContext(UWorld* TargetWorld);
	void ShutdownToolsContext();

	void SetContextActor(AToolsContextActor* ActorIn);

	virtual void Tick(float DeltaTime);

	IToolsContextTransactionsAPI* GetTransactionsAPI();


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


	UFUNCTION(BlueprintCallable)
	TArray<UObject*> GetActiveToolPropertySets();



	UFUNCTION(BlueprintCallable)
	URuntimeMeshSceneObject* ImportMeshSceneObject(const FString Path, bool bFlipOrientation);


	UFUNCTION(BlueprintCallable)
	USceneHistoryManager* GetSceneHistory() { return SceneHistory;  }


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


public:
	UFUNCTION(BlueprintCallable)
	bool IsCapturingMouse() const;


	IToolsContextAssetAPI* GetAssetAPI();


protected:
	TSharedPtr<FRuntimeToolsContextQueriesImpl> ContextQueriesAPI;
	TSharedPtr<FRuntimeToolsContextTransactionImpl> ContextTransactionsAPI;
	TSharedPtr<FRuntimeToolsContextAssetImpl> ContextAssetAPI;

	void OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool);
	void OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool);

	void OnSceneHistoryStateChange();

public:
	// input handling
	void OnLeftMouseDown();
	void OnLeftMouseUp();

protected:
	FVector2D PrevMousePosition = FVector2D::ZeroVector;

	FInputDeviceState CurrentMouseState;
	bool bPendingMouseStateChange = false;

	void ProcessAccumulatedMouseInput();

	FViewCameraState CurrentViewCameraState;

	void InternalConsistencyChecks();
	bool bIsShuttingDown = false;


public:
	void AddPropertySetKeepalive(UInteractiveToolPropertySet* PropertySet);
	void AddAllPropertySetKeepalives(UInteractiveTool* Tool);

protected:
	UPROPERTY()
	TArray<UObject*> PropertySetKeepAlives;
};
