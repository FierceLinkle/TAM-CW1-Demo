// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TAM_CWGameMode.generated.h"

class ATAM_CWCharacter;

UCLASS(minimalapi)
class ATAM_CWGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATAM_CWGameMode();

	virtual void BeginPlay() override;

	//virtual void Tick(float DeltaTime) override;

	ATAM_CWCharacter* MyCharacter;

	UPROPERTY(EditAnywhere, Category = "Exp")
		TSubclassOf<class UUserWidget> HUDWidgetClass; //HUD class

	UPROPERTY(EditAnywhere, Category = "Exp")
		class UUserWidget* CurrentWidget;
};



