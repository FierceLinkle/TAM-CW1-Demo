// Fill out your copyright notice in the Description page of Project Settings.


#include "SimpleEnemy.h"
#include "TAM_CWProjectile.h"
#include "TAM_CWCharacter.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

// Sets default values
ASimpleEnemy::ASimpleEnemy()
{
	OnActorHit.AddDynamic(this, &ASimpleEnemy::OnHit);
}

void ASimpleEnemy::BeginPlay() {
	Super::BeginPlay();

	MyChar = Cast<ATAM_CWCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void ASimpleEnemy::OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit) {
	if (OtherActor != nullptr && OtherActor != this)
	{
		class ATAM_CWProjectile* Projectile = Cast< ATAM_CWProjectile>(OtherActor);

		if (Projectile) {
			MyChar->UpdateExp(50.0f);
			Destroy();
		}
	}
}

