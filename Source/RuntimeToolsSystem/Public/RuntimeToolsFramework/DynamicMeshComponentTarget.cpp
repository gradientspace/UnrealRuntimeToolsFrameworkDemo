// Copyright Epic Games, Inc. All Rights Reserved.

#include "DynamicMeshComponentTarget.h"

#include "MeshDescriptionToDynamicMesh.h"
#include "DynamicMeshToMeshDescription.h"
#include "SimpleDynamicMeshComponent.h"

#include "MeshDescription.h"
#include "StaticMeshAttributes.h"

#include "RuntimeToolsFrameworkSubsystem.h"  // to emit change transaction


#define LOCTEXT_NAMESPACE "FSimpleDynamicMeshComponentTargetFactory"


void FSimpleDynamicMeshComponentTargetFactory::RegisterFactory()
{
	static bool bFactoryRegistered = false;
	if (!bFactoryRegistered)
	{
		AddComponentTargetFactory(TUniquePtr<FComponentTargetFactory>{new FSimpleDynamicMeshComponentTargetFactory{} });
		bFactoryRegistered = true;
	}
}



bool FSimpleDynamicMeshComponentTargetFactory::CanBuild(UActorComponent* Component)
{
	return !!Cast<USimpleDynamicMeshComponent>(Component);
}

TUniquePtr<FPrimitiveComponentTarget> FSimpleDynamicMeshComponentTargetFactory::Build(UPrimitiveComponent* Component)
{
	USimpleDynamicMeshComponent* SimpleDynamicMeshComponent = Cast<USimpleDynamicMeshComponent>(Component);
	if (SimpleDynamicMeshComponent != nullptr)
	{
		return TUniquePtr<FPrimitiveComponentTarget> { new FSimpleDynamicMeshComponentTarget{ Component } };
	}
	return {};
}



FSimpleDynamicMeshComponentTarget::FSimpleDynamicMeshComponentTarget(UPrimitiveComponent* Component)
	: FPrimitiveComponentTarget{ Cast<USimpleDynamicMeshComponent>(Component) }
{
	MeshDescription = MakeUnique<FMeshDescription>();
	FStaticMeshAttributes StaticMeshAttributes(*this->MeshDescription);
	StaticMeshAttributes.Register();
}

FMeshDescription* FSimpleDynamicMeshComponentTarget::GetMesh()
{
	FDynamicMeshToMeshDescription Converter;
	const FDynamicMesh3* Mesh = Cast<USimpleDynamicMeshComponent>(Component)->GetMesh();
	Converter.Convert(Mesh, *MeshDescription);
	return MeshDescription.Get();
}

void FSimpleDynamicMeshComponentTarget::CommitMesh(const FCommitter& ModifyFunc)
{
	ModifyFunc({ MeshDescription.Get() });

	USimpleDynamicMeshComponent* MeshComponent = Cast<USimpleDynamicMeshComponent>(Component);
	FDynamicMesh3* Mesh = MeshComponent->GetMesh();

	TSharedPtr<FDynamicMesh3> MeshBefore = MakeShared<FDynamicMesh3>(*Mesh);

	TSharedPtr<FDynamicMesh3> MeshAfter = MakeShared<FDynamicMesh3>();
	FMeshDescriptionToDynamicMesh Converter;
	Converter.Convert(MeshDescription.Get(), *MeshAfter);

	TUniquePtr<FMeshReplacementChange> ReplaceChange = MakeUnique<FMeshReplacementChange>(MeshBefore, MeshAfter);
	MeshComponent->ApplyChange(ReplaceChange.Get(), false);

	URuntimeToolsFrameworkSubsystem::Get()->GetTransactionsAPI()->AppendChange(MeshComponent, MoveTemp(ReplaceChange),
		LOCTEXT("UpdateMeshChange", "Update Mesh"));
}


#undef LOCTEXT_NAMESPACE