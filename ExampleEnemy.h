// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TAM_CWCharacter.h"
#include "GameFramework/Character.h"
#include "ExampleEnemy.generated.h"

class UPawnSensingComponent;

UCLASS()
class TAM_CW_API AExampleEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AExampleEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	ATAM_CWCharacter* MyChar;

protected:

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		UPawnSensingComponent* PawnSense; //Component for sesning other pawns

	UFUNCTION()
		void SeePawn(APawn* OtherPawn);

public:
	UFUNCTION()
		void OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
