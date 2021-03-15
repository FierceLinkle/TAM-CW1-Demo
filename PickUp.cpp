// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp.h"
#include "TAM_CWCharacter.h"

// Sets default values
APickUp::APickUp()
{
    OnActorBeginOverlap.AddDynamic(this, &APickUp::OnOverlap);
}

//Check for overlap with player character
void APickUp::OnOverlap(AActor* MyOverlappedActor, AActor* OtherActor) {
	if (OtherActor != nullptr && OtherActor != this)
	{
		class ATAM_CWCharacter* MyCharacter = Cast<ATAM_CWCharacter>(OtherActor);

		if (MyCharacter)
		{
			MyCharacter->UpdateExp(30.0f);
			Destroy();
		}
	}
}

