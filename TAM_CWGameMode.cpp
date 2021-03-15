// Copyright Epic Games, Inc. All Rights Reserved.

#include "TAM_CWGameMode.h"
#include "TAM_CWHUD.h"
#include "Kismet/GameplayStatics.h"
#include "TAM_CWCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"

ATAM_CWGameMode::ATAM_CWGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	static ConstructorHelpers::FClassFinder<UUserWidget> ExpBar(TEXT("/Game/CW1_Content/ExpUI"));
	HUDWidgetClass = ExpBar.Class;

	// use our custom HUD class
	HUDClass = ATAM_CWHUD::StaticClass();

	//Add exp bar UI to viewport
	if (HUDWidgetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);

		if (CurrentWidget)
		{
			CurrentWidget->AddToViewport();
		}
	}
}

void ATAM_CWGameMode::BeginPlay()
{
	Super::BeginPlay();

	MyCharacter = Cast<ATAM_CWCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
}
