#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InteractiveToolsContext.h"
#include "RuntimeMeshSceneObject.h"
#include "RuntimeMeshSceneSubsystem.generated.h"

class FMeshSceneSelectionChange;
class FAddRemoveSceneObjectChange;

/**
 * URuntimeMeshSceneSubsystem manages a "Scene" of "SceneObjects", currently only URuntimeMeshSceneObject (SO).
 *
 * Use CreateNewSceneObject() to create a new SO, and the various Delete functions to remove them.
 * These changes will be undo-able, ie they will send Change events to the USceneHistoryManager instance.
 * 
 * An active Selection Set is tracked, and there are API functions for modifying this Selection set, also undo-able.
 * 
 * Cast rays into the scene using FindNearestHitObject()
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API URuntimeMeshSceneSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static void InitializeSingleton(URuntimeMeshSceneSubsystem* Subsystem);
	static URuntimeMeshSceneSubsystem* Get();
protected:
	static URuntimeMeshSceneSubsystem* InstanceSingleton;

public:
	virtual void Deinitialize() override;


	virtual void SetCurrentTransactionsAPI(IToolsContextTransactionsAPI* TransactionsAPI);


public:
	UPROPERTY()
	UMaterialInterface* StandardMaterial;

	UPROPERTY()
	UMaterialInterface* SelectedMaterial;

	UPROPERTY()
	UMaterialInterface* WireframeMaterial;

public:

	UFUNCTION(BlueprintCallable)
	URuntimeMeshSceneObject* CreateNewSceneObject();

	UFUNCTION(BlueprintCallable)
	URuntimeMeshSceneObject* FindSceneObjectByActor(AActor* Actor);

	UFUNCTION(BlueprintCallable)
	bool DeleteSceneObject(URuntimeMeshSceneObject* Object);

	UFUNCTION(BlueprintCallable)
	bool DeleteSelectedSceneObjects();

	bool DeleteSelectedSceneObjects(AActor* SkipActor);


public:

	UFUNCTION(BlueprintCallable, Category = "URuntimeMeshSceneSubsystem")
	TArray<URuntimeMeshSceneObject*> GetSelection() const { return SelectedSceneObjects; }

	UFUNCTION(BlueprintCallable, Category = "URuntimeMeshSceneSubsystem")
	void ClearSelection();

	UFUNCTION(BlueprintCallable, Category = "URuntimeMeshSceneSubsystem")
	void SetSelected(URuntimeMeshSceneObject* SceneObject, bool bDeselect = false, bool bDeselectOthers = true);

	UFUNCTION(BlueprintCallable, Category = "URuntimeMeshSceneSubsystem")
	void ToggleSelected(URuntimeMeshSceneObject* SceneObject);

	UFUNCTION(BlueprintCallable, Category = "URuntimeMeshSceneSubsystem")
	void SetSelection(const TArray<URuntimeMeshSceneObject*>& SceneObjects);


	DECLARE_MULTICAST_DELEGATE_OneParam(FMeshSceneSelectionChangedEvent, URuntimeMeshSceneSubsystem*);
	FMeshSceneSelectionChangedEvent OnSelectionModified;




public:


	UFUNCTION(BlueprintCallable, Category = "URuntimeMeshSceneSubsystem")
	URuntimeMeshSceneObject* FindNearestHitObject(FVector RayOrigin, FVector RayDirection, FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle, FVector& TriBaryCoords, float MaxDistance = 0);


protected:

	IToolsContextTransactionsAPI* TransactionsAPI = nullptr;

	UPROPERTY()
	TArray<URuntimeMeshSceneObject*> SceneObjects;

	void AddSceneObjectInternal(URuntimeMeshSceneObject* Object, bool bIsUndoRedo);
	void RemoveSceneObjectInternal(URuntimeMeshSceneObject* Object, bool bIsUndoRedo);

	UPROPERTY()
	TArray<URuntimeMeshSceneObject*> SelectedSceneObjects;

	void SetSelectionInternal(const TArray<URuntimeMeshSceneObject*>& SceneObjects);
	
	TUniquePtr<FMeshSceneSelectionChange> ActiveSelectionChange;
	void BeginSelectionChange();
	void EndSelectionChange();

	friend class FMeshSceneSelectionChange;
	friend class FAddRemoveSceneObjectChange;
};





/**
 * FMeshSelectionChange represents an reversible change to a UMeshSelectionSet
 */
class RUNTIMETOOLSSYSTEM_API FMeshSceneSelectionChange : public FToolCommandChange
{
public:
	TArray<URuntimeMeshSceneObject*> OldSelection;
	TArray<URuntimeMeshSceneObject*> NewSelection;

	virtual void Apply(UObject* Object) override;
	virtual void Revert(UObject* Object) override;
	virtual FString ToString() const override { return TEXT("FMeshSceneSelectionChange"); }
};



class RUNTIMETOOLSSYSTEM_API FAddRemoveSceneObjectChange : public FToolCommandChange
{
public:
	URuntimeMeshSceneObject* SceneObject;
	bool bAdded = true;

public:
	virtual void Apply(UObject* Object) override;
	virtual void Revert(UObject* Object) override;
	virtual FString ToString() const override { return TEXT("FAddRemoveSceneObjectChange"); }
};

