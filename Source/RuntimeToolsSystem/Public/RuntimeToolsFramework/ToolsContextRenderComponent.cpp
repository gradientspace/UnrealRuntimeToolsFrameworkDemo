// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolsContextRenderComponent.h"
#include "PrimitiveSceneProxy.h"
#include "SceneManagement.h"


// SceneProxy for UToolsContextRenderComponent. Just uses the PDI's available in GetDynamicMeshElements
// to draw the lines/points accumulated by the Component.
class FToolsContextRenderComponentSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FToolsContextRenderComponentSceneProxy(
		const UToolsContextRenderComponent* InComponent,
		TUniqueFunction<void(TArray<UToolsContextRenderComponent::FPDILine>&, TArray<UToolsContextRenderComponent::FPDIPoint>&)>&& GeometryQueryFunc)
		: FPrimitiveSceneProxy(InComponent)
	{
		GetGeometryQueryFunc = MoveTemp(GeometryQueryFunc);
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		TArray<UToolsContextRenderComponent::FPDILine> Lines;
		TArray<UToolsContextRenderComponent::FPDIPoint> Points;
		GetGeometryQueryFunc(Lines, Points);

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

				int32 NumLines = Lines.Num(), NumPoints = Points.Num();
				for (int32 k = 0; k < NumLines; ++k)
				{
					PDI->DrawLine(Lines[k].Start, Lines[k].End, Lines[k].Color, Lines[k].DepthPriorityGroup, Lines[k].Thickness, Lines[k].DepthBias, Lines[k].bScreenSpace);
				}
				for (int32 k = 0; k < NumPoints; ++k)
				{
					PDI->DrawPoint(Points[k].Position, Points[k].Color, Points[k].PointSize, Points[k].DepthPriorityGroup);
				}
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = false;
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		return Result;
	}

	//virtual bool CanBeOccluded() const override
	//{
	//	return false;
	//}

	virtual uint32 GetMemoryFootprint(void) const override { return sizeof * this + GetAllocatedSize(); }
	uint32 GetAllocatedSize(void) const { return FPrimitiveSceneProxy::GetAllocatedSize(); }


	// set to lambda that steals current lines/points from the Component
	TUniqueFunction<void(TArray<UToolsContextRenderComponent::FPDILine>&, TArray<UToolsContextRenderComponent::FPDIPoint>&)> GetGeometryQueryFunc;
};



// implementation of FPrimitiveDrawInterface that forwards DrawLine/DrawPoint calls to
// a UToolsContextRenderComponent instance. No other PDI functionality is implemented.
// Instances of this class are created by GetPDIForView() below, which is called once per frame 
// by the URuntimeToolsFrameworkSubsystem, in the FRuntimeToolsFrameworkRenderAPI implementation.
class FToolsContextRenderComponentPDI : public FPrimitiveDrawInterface
{
public:
	UToolsContextRenderComponent* RenderComponent;

	FToolsContextRenderComponentPDI(const FSceneView* InView, UToolsContextRenderComponent* RenderComponentIn) : FPrimitiveDrawInterface(InView)
	{
		RenderComponent = RenderComponentIn;
	}

	virtual bool IsHitTesting() { return false; }
	virtual void SetHitProxy(HHitProxy* HitProxy) { };
	virtual void RegisterDynamicResource(FDynamicPrimitiveResource* DynamicResource) { ensure(false); }
	virtual void AddReserveLines(uint8 DepthPriorityGroup, int32 NumLines, bool bDepthBiased = false, bool bThickLines = false) { ensure(false); }
	virtual int32 DrawMesh(const FMeshBatch& Mesh) { ensure(false); return 0; }
	virtual void DrawSprite(
		const FVector& Position, float SizeX, float SizeY,
		const FTexture* Sprite, const FLinearColor& Color, uint8 DepthPriorityGroup,
		float U, float UL, float V, float VL, uint8 BlendMode = 1 /*SE_BLEND_Masked*/ ) { ensure(false); }

	virtual void DrawLine( const FVector& Start, const FVector& End, const FLinearColor& Color,
		uint8 DepthPriorityGroup, float Thickness = 0.0f, float DepthBias = 0.0f, bool bScreenSpace = false	)
	{
		RenderComponent->DrawLine(Start, End, Color, DepthPriorityGroup, Thickness, DepthBias, bScreenSpace);
	}

	virtual void DrawPoint( const FVector& Position, const FLinearColor& Color, float PointSize, uint8 DepthPriorityGroup )
	{
		RenderComponent->DrawPoint(Position, Color, PointSize, DepthPriorityGroup);
	}

};


TSharedPtr<FPrimitiveDrawInterface> UToolsContextRenderComponent::GetPDIForView(const FSceneView* InView)
{
	return MakeShared<FToolsContextRenderComponentPDI>(InView, this);
}



void UToolsContextRenderComponent::DrawLine(
	const FVector& Start,
	const FVector& End,
	const FLinearColor& Color,
	uint8 DepthPriorityGroupIn,
	float Thickness,
	float DepthBias,
	bool bScreenSpace
)
{
	GeometryLock.Lock();
	CurrentLines.Add(
		FPDILine{ Start, End, Color, DepthPriorityGroupIn, Thickness, DepthBias, bScreenSpace });
	GeometryLock.Unlock();
}

void UToolsContextRenderComponent::DrawPoint(
	const FVector& Position,
	const FLinearColor& Color,
	float PointSize,
	uint8 DepthPriorityGroupIn
)
{
	GeometryLock.Lock();
	CurrentPoints.Add(
		FPDIPoint{ Position, Color, PointSize, DepthPriorityGroupIn });
	GeometryLock.Unlock();
}


TUniqueFunction<void(TArray<UToolsContextRenderComponent::FPDILine>&, TArray<UToolsContextRenderComponent::FPDIPoint>&)> UToolsContextRenderComponent::MakeGetCurrentGeometryQueryFunc()
{
	return [this](TArray<FPDILine>& LineStorage, TArray<FPDIPoint>& PointStorage)
	{
		GeometryLock.Lock();
		LineStorage = MoveTemp(CurrentLines);
		PointStorage = MoveTemp(CurrentPoints);
		GeometryLock.Unlock();
	};
}

FPrimitiveSceneProxy* UToolsContextRenderComponent::CreateSceneProxy()
{
	return new FToolsContextRenderComponentSceneProxy(this, MakeGetCurrentGeometryQueryFunc());
}

bool UToolsContextRenderComponent::LineTraceComponent(FHitResult& OutHit, const FVector Start, const FVector End, const FCollisionQueryParams& Params)
{
	// hit testing not supported
	return false;
}


FBoxSphereBounds UToolsContextRenderComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	// infinite bounds. TODO: accumulate actual bounds? will result in unstable depth sorting...
	float f = 9999999.0f;
	return FBoxSphereBounds(FBox(-FVector(f, f, f), FVector(f, f, f)));
}