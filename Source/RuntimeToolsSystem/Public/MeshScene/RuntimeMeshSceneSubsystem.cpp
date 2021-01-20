// Fill out your copyright notice in the Description page of Project Settings.

#include "RuntimeMeshSceneSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "Materials/Material.h"

#define LOCTEXT_NAMESPACE "URuntimeMeshSceneSubsystem"

URuntimeMeshSceneSubsystem* URuntimeMeshSceneSubsystem::InstanceSingleton = nullptr;

void URuntimeMeshSceneSubsystem::InitializeSingleton(URuntimeMeshSceneSubsystem* Subsystem)
{
	check(InstanceSingleton == nullptr);
	InstanceSingleton = Subsystem;

	// todo: expose these somehow?

	UMaterial* DefaultObjectMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Game/RuntimeToolsFrameworkMaterials/DefaultObjectMaterial"));
	InstanceSingleton->StandardMaterial = (DefaultObjectMaterial) ? DefaultObjectMaterial : UMaterial::GetDefaultMaterial(MD_Surface);

	UMaterial* SelectionMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Game/RuntimeToolsFrameworkMaterials/SelectedMaterial"));
	InstanceSingleton->SelectedMaterial = (SelectionMaterial) ? SelectionMaterial : UMaterial::GetDefaultMaterial(MD_Surface);

	UMaterial* WireframeMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Game/RuntimeToolsFrameworkMaterials/WireframeMaterial"));
	WireframeMaterial = (WireframeMaterial) ? WireframeMaterial : UMaterial::GetDefaultMaterial(MD_Surface);
	InstanceSingleton->WireframeMaterial = WireframeMaterial;
	GEngine->WireframeMaterial = WireframeMaterial;
}


URuntimeMeshSceneSubsystem* URuntimeMeshSceneSubsystem::Get()
{
	return InstanceSingleton;
}


void URuntimeMeshSceneSubsystem::Deinitialize()
{
	InstanceSingleton = nullptr;
}


void URuntimeMeshSceneSubsystem::SetCurrentTransactionsAPI(IToolsContextTransactionsAPI* TransactionsAPIIn)
{
	TransactionsAPI = TransactionsAPIIn;
}


URuntimeMeshSceneObject* URuntimeMeshSceneSubsystem::CreateNewSceneObject()
{
	URuntimeMeshSceneObject* SceneObject = NewObject<URuntimeMeshSceneObject>(this);
	AddSceneObjectInternal(SceneObject, false);

	if (TransactionsAPI)
	{
		TUniquePtr<FAddRemoveSceneObjectChange> AddChange = MakeUnique<FAddRemoveSceneObjectChange>();
		AddChange->SceneObject = SceneObject;
		AddChange->bAdded = true;
		// use SceneObject as target so that transaction will keep it from being GC'd
		TransactionsAPI->AppendChange(SceneObject, MoveTemp(AddChange), LOCTEXT("AddObjectChange", "Add SceneObject"));
	}

	SceneObject->SetAllMaterials(StandardMaterial);

	return SceneObject;
}


URuntimeMeshSceneObject* URuntimeMeshSceneSubsystem::FindSceneObjectByActor(AActor* Actor)
{
	for (URuntimeMeshSceneObject* SceneObject : SceneObjects)
	{
		if (SceneObject->GetActor() == Actor)
		{
			return SceneObject;
		}
	}
	return nullptr;
}


bool URuntimeMeshSceneSubsystem::DeleteSceneObject(URuntimeMeshSceneObject* SceneObject)
{
	if (SceneObjects.Contains(SceneObject))
	{
		if (SelectedSceneObjects.Contains(SceneObject))
		{
			BeginSelectionChange();
			SelectedSceneObjects.Remove(SceneObject);
			EndSelectionChange();
			OnSelectionModified.Broadcast(this);
		}

		RemoveSceneObjectInternal(SceneObject, true);

		if (TransactionsAPI)
		{
			TUniquePtr<FAddRemoveSceneObjectChange> RemoveChange = MakeUnique<FAddRemoveSceneObjectChange>();
			RemoveChange->SceneObject = SceneObject;
			RemoveChange->bAdded = false;
			// use SceneObject as target so that transaction will keep it from being GC'd
			TransactionsAPI->AppendChange(SceneObject, MoveTemp(RemoveChange), LOCTEXT("RemoveObjectChange", "Delete SceneObject"));
		}

		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[URuntimeMeshSceneSubsystem::DeleteSceneObject] Tried to delete non-existant SceneObject"));
		return false;
	}
}


bool URuntimeMeshSceneSubsystem::DeleteSelectedSceneObjects()
{
	return DeleteSelectedSceneObjects(nullptr);
}

bool URuntimeMeshSceneSubsystem::DeleteSelectedSceneObjects(AActor* SkipActor)
{
	if (SelectedSceneObjects.Num() == 0) return false;

	if (TransactionsAPI)
	{
		TransactionsAPI->BeginUndoTransaction(LOCTEXT("DeleteSelectedObjectsChange", "Delete Objects"));
	}

	TArray<URuntimeMeshSceneObject*> DeleteObjects = SelectedSceneObjects;

	BeginSelectionChange();
	SelectedSceneObjects.Reset();
	EndSelectionChange();

	for (URuntimeMeshSceneObject* SceneObject : DeleteObjects)
	{
		if (SceneObject->GetActor() == SkipActor)
		{
			continue;
		}

		RemoveSceneObjectInternal(SceneObject, true);

		if (TransactionsAPI)
		{
			TUniquePtr<FAddRemoveSceneObjectChange> RemoveChange = MakeUnique<FAddRemoveSceneObjectChange>();
			RemoveChange->SceneObject = SceneObject;
			RemoveChange->bAdded = false;
			// use SceneObject as target so that transaction will keep it from being GC'd
			TransactionsAPI->AppendChange(SceneObject, MoveTemp(RemoveChange), LOCTEXT("RemoveObjectChange", "Delete SceneObject"));
		}
	}

	if (TransactionsAPI)
	{
		TransactionsAPI->EndUndoTransaction();
	}

	OnSelectionModified.Broadcast(this);
	return true;
}






void URuntimeMeshSceneSubsystem::ClearSelection()
{
	if (SelectedSceneObjects.Num() > 0)
	{
		BeginSelectionChange();

		for (URuntimeMeshSceneObject* SO : SelectedSceneObjects)
		{
			SO->ClearHighlightMaterial();
		}
		SelectedSceneObjects.Reset();

		EndSelectionChange();
		OnSelectionModified.Broadcast(this);
	}
}


void URuntimeMeshSceneSubsystem::SetSelected(URuntimeMeshSceneObject* SceneObject, bool bDeselect, bool bDeselectOthers)
{
	if (bDeselect)
	{
		if (SelectedSceneObjects.Contains(SceneObject))
		{
			BeginSelectionChange();
			SelectedSceneObjects.Remove(SceneObject);
			SceneObject->ClearHighlightMaterial();
			EndSelectionChange();
			OnSelectionModified.Broadcast(this);
		}
	}
	else
	{
		BeginSelectionChange();

		bool bIsSelected = SelectedSceneObjects.Contains(SceneObject);
		if (bDeselectOthers)
		{
			for (URuntimeMeshSceneObject* SO : SelectedSceneObjects)
			{
				if (SO != SceneObject)
				{
					SO->ClearHighlightMaterial();
				}
			}
			SelectedSceneObjects.Reset();
		}
		if (bIsSelected == false)
		{
			SceneObject->SetToHighlightMaterial(this->SelectedMaterial);
		}
		SelectedSceneObjects.Add(SceneObject);

		EndSelectionChange();
		OnSelectionModified.Broadcast(this);
	}
}


void URuntimeMeshSceneSubsystem::ToggleSelected(URuntimeMeshSceneObject* SceneObject)
{
	BeginSelectionChange();

	if (SelectedSceneObjects.Contains(SceneObject))
	{
		SelectedSceneObjects.Remove(SceneObject);
		SceneObject->ClearHighlightMaterial();
	}
	else
	{
		SelectedSceneObjects.Add(SceneObject);
		SceneObject->SetToHighlightMaterial(this->SelectedMaterial);
	}

	EndSelectionChange();
	OnSelectionModified.Broadcast(this);
}


void URuntimeMeshSceneSubsystem::SetSelection(const TArray<URuntimeMeshSceneObject*>& NewSceneObjects)
{
	BeginSelectionChange();
	SetSelectionInternal(NewSceneObjects);
	EndSelectionChange();
}
void URuntimeMeshSceneSubsystem::SetSelectionInternal(const TArray<URuntimeMeshSceneObject*>& NewSceneObjects)
{
	if (SelectedSceneObjects.Num() > 0)
	{
		for (URuntimeMeshSceneObject* SO : SelectedSceneObjects)
		{
			SO->ClearHighlightMaterial();
		}
		SelectedSceneObjects.Reset();
	}

	for (URuntimeMeshSceneObject* SO : NewSceneObjects)
	{
		if (SceneObjects.Contains(SO))
		{
			if (SelectedSceneObjects.Contains(SO) == false)
			{
				SelectedSceneObjects.Add(SO);
				SO->SetToHighlightMaterial(this->SelectedMaterial);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[URuntimeMeshSceneSubsystem::SetSelectionInternal] Tried to select non-existant SceneObject"));
		}
	}

	OnSelectionModified.Broadcast(this);
}



URuntimeMeshSceneObject* URuntimeMeshSceneSubsystem::FindNearestHitObject(FVector RayOrigin, FVector RayDirection, FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle, FVector& TriBaryCoords, float MaxDistance)
{
	URuntimeMeshSceneObject* FoundHit = nullptr;
	float MinHitDistance = TNumericLimits<float>::Max();

	for (URuntimeMeshSceneObject* SO : SceneObjects)
	{
		FVector HitPoint, BaryCoords;
		float HitDist;
		int32 NearestTri;
		if (SO->IntersectRay(RayOrigin, RayDirection, HitPoint, HitDist, NearestTri, BaryCoords, MaxDistance))
		{
			if (HitDist < MinHitDistance)
			{
				MinHitDistance = HitDist;
				WorldHitPoint = HitPoint;
				HitDistance = HitDist;
				NearestTriangle = NearestTri;
				TriBaryCoords = BaryCoords;
				FoundHit = SO;
			}
		}
	}
	return FoundHit;
}




void URuntimeMeshSceneSubsystem::BeginSelectionChange()
{
	check(!ActiveSelectionChange);

	ActiveSelectionChange = MakeUnique<FMeshSceneSelectionChange>();
	ActiveSelectionChange->OldSelection = SelectedSceneObjects;
}

void URuntimeMeshSceneSubsystem::EndSelectionChange()
{
	check(ActiveSelectionChange);
	if (SelectedSceneObjects != ActiveSelectionChange->OldSelection)
	{
		ActiveSelectionChange->NewSelection = SelectedSceneObjects;

		if (TransactionsAPI)
		{
			TransactionsAPI->AppendChange(this, MoveTemp(ActiveSelectionChange), LOCTEXT("SelectionChange", "Selection Change"));
		}
	}

	ActiveSelectionChange = nullptr;
}

void FMeshSceneSelectionChange::Apply(UObject* Object)
{
	if (URuntimeMeshSceneSubsystem* Subsystem = Cast<URuntimeMeshSceneSubsystem>(Object))
	{
		Subsystem->SetSelectionInternal(NewSelection);
	}
}
void FMeshSceneSelectionChange::Revert(UObject* Object)
{
	if (URuntimeMeshSceneSubsystem* Subsystem = Cast<URuntimeMeshSceneSubsystem>(Object))
	{
		Subsystem->SetSelectionInternal(OldSelection);
	}
}




void URuntimeMeshSceneSubsystem::AddSceneObjectInternal(URuntimeMeshSceneObject* Object, bool bIsUndoRedo)
{
	SceneObjects.Add(Object);

	if (bIsUndoRedo)
	{
		Object->GetActor()->RegisterAllComponents();
	}
}

void URuntimeMeshSceneSubsystem::RemoveSceneObjectInternal(URuntimeMeshSceneObject* Object, bool bIsUndoRedo)
{
	check(SceneObjects.Contains(Object));
	SceneObjects.Remove(Object);

	Object->GetActor()->UnregisterAllComponents(true);
}



void FAddRemoveSceneObjectChange::Apply(UObject* Object)
{
	if (bAdded)
	{
		URuntimeMeshSceneSubsystem::Get()->AddSceneObjectInternal(SceneObject, true);
	}
	else
	{
		URuntimeMeshSceneSubsystem::Get()->RemoveSceneObjectInternal(SceneObject, true);
	}
}

void FAddRemoveSceneObjectChange::Revert(UObject* Object)
{
	if (bAdded)
	{
		URuntimeMeshSceneSubsystem::Get()->RemoveSceneObjectInternal(SceneObject, true);
	}
	else
	{
		URuntimeMeshSceneSubsystem::Get()->AddSceneObjectInternal(SceneObject, true);
	}
}


#undef LOCTEXT_NAMESPACE