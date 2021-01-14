#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ToolsFrameworkDemoGameModeBase.generated.h"

class URuntimeToolsFrameworkSubsystem;
class URuntimeMeshSceneSubsystem;

/**
 * AToolsFrameworkDemoGameModeBase is a GameMode that initializes the URuntimeMeshSceneSubsystem and 
 * URuntimeToolsFrameworkSubsystem, and then registers various Tools (see InitializeToolsSystem).
 * 
 * The GameMode Tick also ticks the Tools system
 */
UCLASS()
class TOOLSFRAMEWORKDEMO_API AToolsFrameworkDemoGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AToolsFrameworkDemoGameModeBase();

	virtual void Tick(float DeltaTime) override;
	virtual void StartPlay() override;

	virtual void InitializeToolsSystem();
	virtual void ShutdownToolsSystem();

	virtual void RegisterTools();

	UPROPERTY()
	URuntimeToolsFrameworkSubsystem* ToolsSystem;

	UPROPERTY()
	URuntimeMeshSceneSubsystem* SceneSystem;

};
