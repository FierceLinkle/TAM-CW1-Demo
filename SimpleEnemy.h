// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TAM_CWCharacter.h"
#include "GameFramework/Actor.h"
#include "SimpleEnemy.generated.h"

UCLASS()
class TAM_CW_API ASimpleEnemy : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASimpleEnemy();

protected:

	virtual void BeginPlay() override;

public:

	ATAM_CWCharacter* MyChar;

	UFUNCTION()
		void OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

};
