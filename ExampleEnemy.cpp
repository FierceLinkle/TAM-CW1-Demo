// Fill out your copyright notice in the Description page of Project Settings.


#include "ExampleEnemy.h"
#include "TAM_CWProjectile.h"
#include "TAM_CWCharacter.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Perception/PawnSensingComponent.h"

// Sets default values
AExampleEnemy::AExampleEnemy()
{
	//Initialise OnHit and Pawn sesning
	OnActorHit.AddDynamic(this, &AExampleEnemy::OnHit);

	PawnSense = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSense"));
	PawnSense->SensingInterval = 0.25f;
	PawnSense->SetPeripheralVisionAngle(25.0f);
}

// Called when the game starts or when spawned
void AExampleEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	MyChar = Cast<ATAM_CWCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	PawnSense->OnSeePawn.AddDynamic(this, &AExampleEnemy::SeePawn);
}

// Called every frame
void AExampleEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AExampleEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


//Checks for projectile collsion
void AExampleEnemy::OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit) {
	if (OtherActor != nullptr && OtherActor != this)
	{
		class ATAM_CWProjectile* Projectile = Cast< ATAM_CWProjectile>(OtherActor);

		if (Projectile) {
			MyChar->UpdateExp(50.0f);
			Destroy();
		}
	}
}

//Checks for other pawns in sense range
void AExampleEnemy::SeePawn(APawn* OtherPawn) {
	FString message = TEXT("Saw Actor ") + OtherPawn->GetName();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, message);
}

