// Fill out your copyright notice in the Description page of Project Settings.


#include "RuntimeToolsFrameworkSubsystem.h"

#include "ToolContextInterfaces.h"
#include "ToolsContextActor.h"
#include "MeshScene/RuntimeMeshSceneSubsystem.h"
#include "GeneratedMesh.h"

#include "Framework/Application/SlateApplication.h"
#include "Slate/SceneViewport.h"
#include "Slate/SGameLayerManager.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

#include "DynamicMeshComponentTarget.h"

#include "Materials/Material.h"

#include "BaseGizmos/GizmoRenderingUtil.h"








class FRuntimeToolsContextQueriesImpl : public IToolsContextQueriesAPI
{
public:
	FRuntimeToolsContextQueriesImpl(UInteractiveToolsContext* InContext, UWorld* InWorld)
	{
		ToolsContext = InContext;
		TargetWorld = InWorld;
	}

	virtual void SetContextActor(AToolsContextActor* ContextActorIn)
	{
		ContextActor = ContextActorIn;
	}


	virtual void GetCurrentSelectionState(FToolBuilderState& StateOut) const override
	{
		StateOut.ToolManager = ToolsContext->ToolManager;
		StateOut.GizmoManager = ToolsContext->GizmoManager;
		StateOut.World = TargetWorld;

		const TArray<URuntimeMeshSceneObject*>& Selection = URuntimeMeshSceneSubsystem::Get()->GetSelection();
		for (URuntimeMeshSceneObject* SO : Selection)
		{
			StateOut.SelectedActors.Add(SO->GetActor());
			StateOut.SelectedComponents.Add(SO->GetMeshComponent());
		}
	}

	virtual void GetCurrentViewState(FViewCameraState& StateOut) const override
	{
		if (!ContextActor)
		{
			return;
		}

		bool bHasCamera = ContextActor->HasActiveCameraComponent();

		FVector Location;
		FRotator Rotation;
		ContextActor->GetActorEyesViewPoint(Location, Rotation);

		StateOut.Position = Location;
		StateOut.Orientation = Rotation.Quaternion();
		StateOut.HorizontalFOVDegrees = 90;
		StateOut.OrthoWorldCoordinateWidth = 1;
		StateOut.AspectRatio = 1.0;
		StateOut.bIsOrthographic = false;
		StateOut.bIsVR = false;
	}

	virtual EToolContextCoordinateSystem GetCurrentCoordinateSystem() const override
	{
		return URuntimeToolsFrameworkSubsystem::Get()->GetCurrentCoordinateSystem();
	}

	virtual bool ExecuteSceneSnapQuery(const FSceneSnapQueryRequest& Request, TArray<FSceneSnapQueryResult>& Results) const override
	{
		return false;
	}

	virtual UMaterialInterface* GetStandardMaterial(EStandardToolContextMaterials MaterialType) const override
	{
		return UMaterial::GetDefaultMaterial(MD_Surface);
	}

#if WITH_EDITOR
	virtual HHitProxy* GetHitProxy(int32 X, int32 Y) const override
	{
		return nullptr;
	}
#endif

protected:
	UInteractiveToolsContext* ToolsContext;
	UWorld* TargetWorld;
	AToolsContextActor* ContextActor = nullptr;
};




class FRuntimeToolsContextTransactionImpl : public IToolsContextTransactionsAPI
{
public:
	
	bool bInTransaction = false;

	virtual void DisplayMessage(const FText& Message, EToolMessageLevel Level) override
	{
		UE_LOG(LogTemp, Warning, TEXT("[ToolMessage] %s"), *Message.ToString());
	}

	virtual void PostInvalidation() override
	{
		// not necessary in runtime context
	}

	virtual void BeginUndoTransaction(const FText& Description) override
	{
		URuntimeToolsFrameworkSubsystem::Get()->SceneHistory->BeginTransaction(Description);
		bInTransaction = true;
	}

	virtual void EndUndoTransaction() override
	{
		URuntimeToolsFrameworkSubsystem::Get()->SceneHistory->EndTransaction();
		bInTransaction = false;
	}

	virtual void AppendChange(UObject* TargetObject, TUniquePtr<FToolCommandChange> Change, const FText& Description) override
	{
		bool bCloseTransaction = false;
		if (!bInTransaction)
		{
			BeginUndoTransaction(Description);
			bCloseTransaction = true;
		}

		URuntimeToolsFrameworkSubsystem::Get()->SceneHistory->AppendChange(TargetObject, MoveTemp(Change), Description);

		if (bCloseTransaction)
		{
			EndUndoTransaction();
		}
	}

	virtual bool RequestSelectionChange(const FSelectedOjectsChangeList& SelectionChange) override
	{
		// not supported. Would need to map elements of SelectionChange to MeshSceneObjects.
		return false;
	}
};






class FRuntimeToolsContextAssetImpl : public IToolsContextAssetAPI
{
public:
	virtual FString GetWorldRelativeAssetRootPath(const UWorld* World) override
	{
		return FString("");
	}

	virtual FString GetActiveAssetFolderPath() override
	{
		return FString("");
	}

	virtual FString InteractiveSelectAssetPath(const FString& DefaultAssetName, const FText& DialogTitleMessage) override
	{
		return FString("");
	}

	virtual UPackage* MakeNewAssetPackage(const FString& FolderPath, const FString& AssetBaseName, FString& UniqueAssetNameOut) override
	{
		return NewObject<UPackage>();
	}

	virtual void InteractiveSaveGeneratedAsset(UObject* Asset, UPackage* AssetPackage) override
	{
		ensure(false);
	}

	virtual void AutoSaveGeneratedAsset(UObject* Asset, UPackage* AssetPackage) override
	{
		ensure(false);
	}

	virtual void NotifyGeneratedAssetModified(UObject* Asset, UPackage* AssetPackage) override
	{
		ensure(false);
	}

	virtual AActor* GenerateStaticMeshActor(
		UWorld* TargetWorld,
		FTransform Transform,
		FString ObjectBaseName,
		FGeneratedStaticMeshAssetConfig&& AssetConfig) override
	{

		URuntimeMeshSceneObject* SceneObject = URuntimeMeshSceneSubsystem::Get()->CreateNewSceneObject();
		SceneObject->Initialize(TargetWorld, AssetConfig.MeshDescription.Get());
		SceneObject->SetTransform(Transform);
		return SceneObject->GetActor();
	}

	virtual bool SaveGeneratedTexture2D(
		UTexture2D* GeneratedTexture,
		FString ObjectBaseName,
		const UObject* RelativeToAsset) override
	{
		ensure(false);
		return false;
	}

};



URuntimeToolsFrameworkSubsystem* URuntimeToolsFrameworkSubsystem::InstanceSingleton = nullptr;

void URuntimeToolsFrameworkSubsystem::InitializeSingleton(URuntimeToolsFrameworkSubsystem* Subsystem)
{
	check(InstanceSingleton == nullptr);
	InstanceSingleton = Subsystem;
}


URuntimeToolsFrameworkSubsystem* URuntimeToolsFrameworkSubsystem::Get()
{
	check(InstanceSingleton);
	return InstanceSingleton;
}


void URuntimeToolsFrameworkSubsystem::Deinitialize()
{
	ShutdownToolsContext();

	InstanceSingleton = nullptr;
}


void URuntimeToolsFrameworkSubsystem::InitializeToolsContext(UWorld* TargetWorldIn)
{
	TargetWorld = TargetWorldIn;

	ToolsContext = NewObject<UInteractiveToolsContext>();
	
	ContextQueriesAPI = MakeShared<FRuntimeToolsContextQueriesImpl>(ToolsContext, TargetWorld);
	if (ContextActor)
	{
		ContextQueriesAPI->SetContextActor(ContextActor);
	}

	ContextTransactionsAPI = MakeShared<FRuntimeToolsContextTransactionImpl>();

	ContextAssetAPI = MakeShared<FRuntimeToolsContextAssetImpl>();

	ToolsContext->Initialize(ContextQueriesAPI.Get(), ContextTransactionsAPI.Get());

	// register event handlers
	ToolsContext->ToolManager->OnToolStarted.AddUObject(this, &URuntimeToolsFrameworkSubsystem::OnToolStarted);
	ToolsContext->ToolManager->OnToolEnded.AddUObject(this, &URuntimeToolsFrameworkSubsystem::OnToolEnded);

	// create scene history
	SceneHistory = NewObject<USceneHistoryManager>(this);
	SceneHistory->OnHistoryStateChange.AddUObject(this, &URuntimeToolsFrameworkSubsystem::OnSceneHistoryStateChange);


	// register selection interaction
	SelectionInteraction = NewObject<USceneObjectSelectionInteraction>();
	SelectionInteraction->Initialize([this]() 
	{
		return HaveActiveTool() == false;
	});
	ToolsContext->InputRouter->RegisterSource(SelectionInteraction);


	// create transform interaction
	TransformInteraction = NewObject<USceneObjectTransformInteraction>();
	TransformInteraction->Initialize([this]()
	{
		return HaveActiveTool() == false;
	});


	// create PDI rendering bridge Component
	FActorSpawnParameters SpawnInfo;
	PDIRenderActor = TargetWorld->SpawnActor<AActor>(FVector::ZeroVector, FRotator(0,0,0), SpawnInfo);
	PDIRenderComponent = NewObject<UToolsContextRenderComponent>(PDIRenderActor);
	PDIRenderActor->SetRootComponent(PDIRenderComponent);
	PDIRenderComponent->RegisterComponent();


	// have to disable this for current tools framework handling of property defaults.
	GShouldVerifyGCAssumptions = false;

	// make sure we have registered FPrimitiveComponentTarget factories
	FSimpleDynamicMeshComponentTargetFactory::RegisterFactory();
}


void URuntimeToolsFrameworkSubsystem::ShutdownToolsContext()
{
	bIsShuttingDown = true;

	if (ToolsContext != nullptr)
	{
		CancelOrCompleteActiveTool();

		TransformInteraction->Shutdown();

		ToolsContext->Shutdown();
	}

	if (PDIRenderActor)
	{
		PDIRenderActor->Destroy();

		PDIRenderActor = nullptr;
		PDIRenderComponent = nullptr;
	}

	TargetWorld = nullptr;
	ToolsContext = nullptr;
	ContextActor = nullptr;

	ContextQueriesAPI = nullptr;
	ContextTransactionsAPI = nullptr;

	SelectionInteraction = nullptr;
	TransformInteraction = nullptr;

	bIsShuttingDown = false;
}



void URuntimeToolsFrameworkSubsystem::OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	AddAllPropertySetKeepalives(Tool);

	TransformInteraction->ForceUpdateGizmoState();
}

void URuntimeToolsFrameworkSubsystem::OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	if (!bIsShuttingDown)
	{
		TransformInteraction->ForceUpdateGizmoState();
	}
}

void URuntimeToolsFrameworkSubsystem::OnSceneHistoryStateChange()
{
	if (!bIsShuttingDown)
	{
		TransformInteraction->ForceUpdateGizmoState();
	}
}


bool URuntimeToolsFrameworkSubsystem::IsCapturingMouse() const
{
	return ToolsContext && ToolsContext->InputRouter->HasActiveMouseCapture();
}




class FRuntimeToolsFrameworkRenderImpl : public IToolsContextRenderAPI
{
public:
	UToolsContextRenderComponent* RenderComponent;
	TSharedPtr<FPrimitiveDrawInterface> PDI;
	const FSceneView* SceneView;
	FViewCameraState ViewCameraState;

	FRuntimeToolsFrameworkRenderImpl(UToolsContextRenderComponent* RenderComponentIn, const FSceneView* ViewIn, FViewCameraState CameraState)
		: RenderComponent(RenderComponentIn), SceneView(ViewIn), ViewCameraState(CameraState)
	{
		PDI = RenderComponentIn->GetPDIForView(ViewIn);
	}

	virtual FPrimitiveDrawInterface* GetPrimitiveDrawInterface() override
	{
		return PDI.Get();
	}

	virtual const FSceneView* GetSceneView() override
	{
		return SceneView;
	}

	virtual FViewCameraState GetCameraState() override
	{
		return ViewCameraState;
	}

	virtual EViewInteractionState GetViewInteractionState() override
	{
		return EViewInteractionState::Focused;
	}
};




PRAGMA_DISABLE_OPTIMIZATION
void URuntimeToolsFrameworkSubsystem::Tick(float DeltaTime)
{
	if (ensure(ContextActor) == false) return;

	GizmoRenderingUtil::SetGlobalFocusedEditorSceneView(nullptr);

	FInputDeviceState InputState = CurrentMouseState;
	InputState.InputDevice = EInputDevices::Mouse;

	FVector2D MousePosition = FSlateApplication::Get().GetCursorPos();
	FVector2D LastMousePosition = FSlateApplication::Get().GetLastCursorPos();
	FModifierKeysState ModifierState = FSlateApplication::Get().GetModifierKeys();

	UGameViewportClient* ViewportClient = TargetWorld->GetGameViewport();
	TSharedPtr<IGameLayerManager> LayerManager = ViewportClient->GetGameLayerManager();
	FGeometry ViewportGeometry;
	if (ensure(LayerManager.IsValid()))
	{
		ViewportGeometry = LayerManager->GetViewportWidgetHostGeometry();
	}
	// why do we need this scale here? what is it for?
	FVector2D ViewportMousePos = ViewportGeometry.Scale * ViewportGeometry.AbsoluteToLocal(MousePosition);


	// update modifier keys
	InputState.SetModifierKeyStates(
		ModifierState.IsLeftShiftDown(),
		ModifierState.IsAltDown(),
		ModifierState.IsControlDown(),
		ModifierState.IsCommandDown());

	if (ViewportClient)
	{
		FSceneViewport* Viewport = ViewportClient->GetGameViewport();

		FEngineShowFlags* ShowFlags = ViewportClient->GetEngineShowFlags();
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			ViewportClient->Viewport,
			TargetWorld->Scene,
			*ShowFlags)
			.SetRealtimeUpdate(true));

		ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(ContextActor->PlayerController->Player);
		FVector ViewLocation;
		FRotator ViewRotation;
		FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily,  /*out*/ ViewLocation, /*out*/ ViewRotation, LocalPlayer->ViewportClient->Viewport);
		if (SceneView == nullptr)
		{ 
			return;		// abort abort
		}

		CurrentViewCameraState.Position = ViewLocation;
		CurrentViewCameraState.Orientation = ViewRotation.Quaternion();
		CurrentViewCameraState.HorizontalFOVDegrees = SceneView->FOV;
		CurrentViewCameraState.AspectRatio = Viewport->GetDesiredAspectRatio(); //ViewportClient->AspectRatio;
		CurrentViewCameraState.bIsOrthographic = false;
		CurrentViewCameraState.bIsVR = false;
		CurrentViewCameraState.OrthoWorldCoordinateWidth = 1;

		FVector4 ScreenPos = SceneView->PixelToScreen(ViewportMousePos.X, ViewportMousePos.Y, 0);

		const FMatrix InvViewMatrix = SceneView->ViewMatrices.GetInvViewMatrix();
		const FMatrix InvProjMatrix = SceneView->ViewMatrices.GetInvProjectionMatrix();

		const float ScreenX = ScreenPos.X;
		const float ScreenY = ScreenPos.Y;

		FVector Origin;
		FVector Direction;
		if (! ViewportClient->IsOrtho())
		{
			Origin = SceneView->ViewMatrices.GetViewOrigin();
			Direction = InvViewMatrix.TransformVector(FVector(InvProjMatrix.TransformFVector4(FVector4(ScreenX * GNearClippingPlane, ScreenY * GNearClippingPlane, 0.0f, GNearClippingPlane)))).GetSafeNormal();
		}
		else
		{
			Origin = InvViewMatrix.TransformFVector4(InvProjMatrix.TransformFVector4(FVector4(ScreenX, ScreenY, 0.5f, 1.0f)));
			Direction = InvViewMatrix.TransformVector(FVector(0, 0, 1)).GetSafeNormal();
		}

		// fudge factor so we don't hit actor...
		Origin += 1.0 * Direction;

		InputState.Mouse.Position2D = ViewportMousePos;
		InputState.Mouse.Delta2D = CurrentMouseState.Mouse.Position2D - PrevMousePosition;
		PrevMousePosition = InputState.Mouse.Position2D;
		InputState.Mouse.WorldRay = FRay(Origin, Direction);


		// if we are in camera control we don't send any events
		bool bInCameraControl = (ContextActor->GetCurrentInteractionMode() != EToolActorInteractionMode::NoInteraction);
		if (bInCameraControl)
		{
			ensure(bPendingMouseStateChange == false);
			ensure(ToolsContext->InputRouter->HasActiveMouseCapture() == false);
			//ToolsContext->InputRouter->PostHoverInputEvent(InputState);
		}
		else if (bPendingMouseStateChange || ToolsContext->InputRouter->HasActiveMouseCapture())
		{
			ToolsContext->InputRouter->PostInputEvent(InputState);
		}
		else
		{
			ToolsContext->InputRouter->PostHoverInputEvent(InputState);
		}

		// clear down or up flags now that we have sent event
		if (bPendingMouseStateChange)
		{
			if (CurrentMouseState.Mouse.Left.bDown)
			{
				CurrentMouseState.Mouse.Left.SetStates(false, true, false);
			}
			else
			{
				CurrentMouseState.Mouse.Left.SetStates(false, false, false);
			}
			bPendingMouseStateChange = false;
		}


		// tick things
		ToolsContext->ToolManager->Tick(DeltaTime);
		ToolsContext->GizmoManager->Tick(DeltaTime);

		// render things
		FRuntimeToolsFrameworkRenderImpl RenderAPI(PDIRenderComponent, SceneView, CurrentViewCameraState);
		ToolsContext->ToolManager->Render(&RenderAPI);
		ToolsContext->GizmoManager->Render(&RenderAPI);

		// force rendering flush so that PDI lines get drawn
		FlushRenderingCommands();
	}
}
PRAGMA_ENABLE_OPTIMIZATION

void URuntimeToolsFrameworkSubsystem::SetContextActor(AToolsContextActor* ActorIn)
{
	ContextActor = ActorIn;
	if (ContextQueriesAPI)
	{
		ContextQueriesAPI->SetContextActor(ContextActor);
	}

}


IToolsContextTransactionsAPI* URuntimeToolsFrameworkSubsystem::GetTransactionsAPI() 
{ 
	return ContextTransactionsAPI.Get(); 
}











IToolsContextAssetAPI* URuntimeToolsFrameworkSubsystem::GetAssetAPI()
{
	return ContextAssetAPI.Get();
}



void URuntimeToolsFrameworkSubsystem::OnLeftMouseDown()
{
	CurrentMouseState.Mouse.Left.SetStates(true, false, false);
	bPendingMouseStateChange = true;
}

void URuntimeToolsFrameworkSubsystem::OnLeftMouseUp()
{
	CurrentMouseState.Mouse.Left.SetStates(false, false, true);
	bPendingMouseStateChange = true;
}



bool URuntimeToolsFrameworkSubsystem::CanActivateToolByName(FString Name)
{
	return true;
}


UInteractiveTool* URuntimeToolsFrameworkSubsystem::BeginToolByName(FString Name)
{
	if (ToolsContext && ToolsContext->ToolManager)
	{
		bool bFound = ToolsContext->ToolManager->SelectActiveToolType(EToolSide::Mouse, Name);
		if (bFound)
		{
			bool bActivated = ToolsContext->ToolManager->ActivateTool(EToolSide::Mouse);
			if (bActivated)
			{
				UInteractiveTool* NewTool = ToolsContext->ToolManager->GetActiveTool(EToolSide::Mouse);
				return NewTool;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("URuntimeToolsFrameworkSubsystem::BeginToolByName - Failed to Activate Tool of type %s!"), *Name);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("URuntimeToolsFrameworkSubsystem::BeginToolByName - Tool Type %s Not Registered!"), *Name);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("URuntimeToolsFrameworkSubsystem::BeginToolByName - Tools Context is not initialized!"));
	}
	return nullptr;
}


bool URuntimeToolsFrameworkSubsystem::HaveActiveTool()
{
	return (ToolsContext != nullptr)
		&& (ToolsContext->ToolManager != nullptr)
		&& ToolsContext->ToolManager->HasActiveTool(EToolSide::Mouse);
}


UInteractiveTool* URuntimeToolsFrameworkSubsystem::GetActiveTool()
{
	return HaveActiveTool() ? ToolsContext->ToolManager->GetActiveTool(EToolSide::Mouse) : nullptr;
}


bool URuntimeToolsFrameworkSubsystem::IsActiveToolAcceptCancelType()
{
	return (ToolsContext != nullptr)
		&& (ToolsContext->ToolManager != nullptr)
		&& ToolsContext->ToolManager->HasActiveTool(EToolSide::Mouse)
		&& ToolsContext->ToolManager->GetActiveTool(EToolSide::Mouse)->HasAccept();
}

bool URuntimeToolsFrameworkSubsystem::CanAcceptActiveTool()
{
	return (ToolsContext != nullptr)
		&& (ToolsContext->ToolManager != nullptr)
		&& ToolsContext->ToolManager->CanAcceptActiveTool(EToolSide::Mouse);
}

bool URuntimeToolsFrameworkSubsystem::AcceptActiveTool()
{
	if (ToolsContext && ToolsContext->ToolManager)
	{
		bool bActive = ToolsContext->ToolManager->HasActiveTool(EToolSide::Mouse);
		if (bActive)
		{
			bool bCanAccept = ToolsContext->ToolManager->CanAcceptActiveTool(EToolSide::Mouse);
			if (bCanAccept)
			{
				ToolsContext->ToolManager->DeactivateTool(EToolSide::Mouse, EToolShutdownType::Accept);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("URuntimeToolsFrameworkSubsystem::AcceptActiveTool - Cannot Accept Active Tool!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("URuntimeToolsFrameworkSubsystem::AcceptActiveTool - No Active Tool!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("URuntimeToolsFrameworkSubsystem::AcceptActiveTool - Tools Context is not initialized!"));
	}

	InternalConsistencyChecks();
	return false;
}


bool URuntimeToolsFrameworkSubsystem::CancelOrCompleteActiveTool()
{
	if (ToolsContext && ToolsContext->ToolManager)
	{
		bool bActive = ToolsContext->ToolManager->HasActiveTool(EToolSide::Mouse);
		if (bActive)
		{
			EToolShutdownType ShutdownType = ToolsContext->ToolManager->CanCancelActiveTool(EToolSide::Mouse) ?
				EToolShutdownType::Cancel : EToolShutdownType::Completed;
			ToolsContext->ToolManager->DeactivateTool(EToolSide::Mouse, ShutdownType);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("URuntimeToolsFrameworkSubsystem::CancelOrCompleteActiveTool - No Active Tool!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("URuntimeToolsFrameworkSubsystem::CancelOrCompleteActiveTool - Tools Context is not initialized!"));
	}

	InternalConsistencyChecks();
	return false;
}



void URuntimeToolsFrameworkSubsystem::InternalConsistencyChecks()
{
	if (GetSceneHistory())
	{
		if (!ensure(GetSceneHistory()->IsBuildingTransaction() == false))
		{
			UE_LOG(LogTemp, Warning, TEXT("[URuntimeToolsFrameworkSubsystem::InternalConsistencyChecks] still Building Transaction! Likely forgot to EndTransaction() somewhere!!"));
		}
	}
}



TArray<UObject*> URuntimeToolsFrameworkSubsystem::GetActiveToolPropertySets()
{
	TArray<UObject*> Result;
	if (HaveActiveTool())
	{
		Result = ToolsContext->ToolManager->GetActiveTool(EToolSide::Mouse)->GetToolProperties();
	}
	return Result;
}



URuntimeMeshSceneObject* URuntimeToolsFrameworkSubsystem::ImportMeshSceneObject(const FString ImportPath, bool bFlipOrientation)
{
	FString UsePath = ImportPath;
	if (FPaths::FileExists(UsePath) == false && FPaths::IsRelative(UsePath))
	{
		UsePath = FPaths::ProjectContentDir() + ImportPath;
	}

	UGeneratedMesh* ImportMesh = NewObject<UGeneratedMesh>();
	if (ImportMesh->ReadMeshFromFile(UsePath, bFlipOrientation) == false)
	{
		ImportMesh->AppendSphere(200, 8, 8);
	}

	URuntimeMeshSceneObject* SceneObject = URuntimeMeshSceneSubsystem::Get()->CreateNewSceneObject();
	SceneObject->Initialize(TargetWorld, ImportMesh->GetMesh().Get());

	return SceneObject;
}



void URuntimeToolsFrameworkSubsystem::SetCurrentCoordinateSystem(EToolContextCoordinateSystem CoordSystem) 
{ 
	CurrentCoordinateSystem = CoordSystem; 
	//TransformInteraction->ForceUpdateGizmoState();
}

void URuntimeToolsFrameworkSubsystem::CycleCurrentCoordinateSystem() 
{ 
	int32 CurCoordSystem = (int)CurrentCoordinateSystem;
	int32 NewCoordSystem = (CurCoordSystem + 1) % 2;
	SetCurrentCoordinateSystem( (EToolContextCoordinateSystem)(NewCoordSystem) );
}




void URuntimeToolsFrameworkSubsystem::AddAllPropertySetKeepalives(UInteractiveTool* Tool)
{
	TArray<UObject*> PropertySets = Tool->GetToolProperties(false);
	for (UObject* PropSetObj : PropertySets)
	{
		if (UInteractiveToolPropertySet* PropertySet = Cast<UInteractiveToolPropertySet>(PropSetObj))
		{
			AddPropertySetKeepalive(PropertySet);
		}
	}
}


void URuntimeToolsFrameworkSubsystem::AddPropertySetKeepalive(UInteractiveToolPropertySet* PropertySet)
{
	if (ensure(PropertySet != nullptr))
	{
		bool bCached = false;
		
		UInteractiveToolPropertySet* CDO = GetMutableDefault<UInteractiveToolPropertySet>(PropertySet->GetClass());

		for (TFieldIterator<FProperty> It(PropertySet->GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
		{
			FString Name = It->GetName();
			if (Name == TEXT("CachedProperties"))
			{
				const FProperty* Property = *It;
				if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
				{
					UObject* TargetObject = ObjectProperty->GetObjectPropertyValue(Property->ContainerPtrToValuePtr<UObject*>(CDO));
					PropertySetKeepAlives.AddUnique(TargetObject);
					bCached = true;
				}
			}
		}

		if (bCached == false)
		{
			FString PropSetName;
			PropertySet->GetClass()->GetName(PropSetName);
			UE_LOG(LogTemp, Warning, TEXT("Failed to find PropertySet Keepalive for %s!"), *PropSetName);
		}
	}
}

