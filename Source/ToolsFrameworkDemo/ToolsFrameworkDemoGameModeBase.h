// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ToolsFrameworkDemoGameModeBase.generated.h"

class URuntimeToolsFrameworkSubsystem;
class URuntimeMeshSceneSubsystem;

/**
 * 
 */
UCLASS()
class TOOLSFRAMEWORKDEMO_API AToolsFrameworkDemoGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AToolsFrameworkDemoGameModeBase();

	virtual void Tick(float DeltaTime) override;

	virtual void StartPlay() override;


	UPROPERTY()
	URuntimeToolsFrameworkSubsystem* ToolsSystem;

	UPROPERTY()
	URuntimeMeshSceneSubsystem* SceneSystem;

};
