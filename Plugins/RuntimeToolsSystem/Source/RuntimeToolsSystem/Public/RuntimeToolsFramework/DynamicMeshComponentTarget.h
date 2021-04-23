// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ComponentSourceInterfaces.h"
#include "MeshDescription.h"
#include "SimpleDynamicMeshComponent.h"


/**
 * Factory for FSimpleDynamicMeshComponentTarget.
 *
 * Call RegisterFactory() somewhere in setup code, before creating/initializing any ToolBuilders.
 * Safe to call RegisterFactory() multiple times.
 */
class RUNTIMETOOLSSYSTEM_API FSimpleDynamicMeshComponentTargetFactory : public FComponentTargetFactory
{
public:
	bool CanBuild(UActorComponent* Candidate) override;
	TUniquePtr<FPrimitiveComponentTarget> Build(UPrimitiveComponent* PrimitiveComponent) override;

	static void RegisterFactory();
};


/**
 * FSimpleDynamicMeshComponentTarget provides a FPrimitiveComponentTarget interface to 
 * a FSimpleDynamicMeshComponent.
 */
class RUNTIMETOOLSSYSTEM_API FSimpleDynamicMeshComponentTarget : public FPrimitiveComponentTarget
{
public:
	FSimpleDynamicMeshComponentTarget(UPrimitiveComponent* Component);

	FMeshDescription* GetMesh() override;
	void CommitMesh(const FCommitter&) override;
	virtual bool HasSameSourceData(const FPrimitiveComponentTarget& OtherTarget) const override
	{
		return OtherTarget.Component == Component;
	}
private:
	TUniquePtr<FMeshDescription> MeshDescription;
};
