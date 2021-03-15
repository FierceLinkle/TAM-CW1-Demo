// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TAM_CWCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;
class UTimelineComponent;

UCLASS(config=Game)
class ATAM_CWCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USceneComponent* FP_MuzzleLocation;

	/** Gun mesh: VR view (attached to the VR controller directly, no arm, just the actual gun) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* VR_Gun;

	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USceneComponent* VR_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* L_MotionController;

public:
	ATAM_CWCharacter();

protected:
	virtual void BeginPlay();

	virtual void Tick(float DeltaTime) override;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class ATAM_CWProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint8 bUsingMotionControllers : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp")
		float MaxExp; //Stores max exp for each level

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp")
		float CurrentExp; //Stores current exp gained for the current level

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp")
		float PreviousCurrentExp; //Store exp before current gain

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp")
		float ExpPercentage; //holds current exp as a percentage

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp")
		int CurrentLevel; //stores current level

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp")
		int SkillPoints; //stores current skill points

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp")
		int LevelCap; //stores level limit

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp")
		float ExpGrowth; //stores value for level growth needed to level up

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp")
		float ExpOverflow; //stores overflow exp from leveling up

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp")
		float ExpLeft; //stores exp needed to level up

	float ExpProgressData[2]; //Stores current and max exp
	int LevelProgressData[2]; //Stores current level and skillpints 

	/** Get Exp */
	UFUNCTION(BlueprintPure, Category = "Exp")
		float GetExp();

	/** Get Current Exp Text */
	UFUNCTION(BlueprintPure, Category = "Exp")
		FText GetExpIntText();

	/** Get Current Level Text */
	UFUNCTION(BlueprintPure, Category = "Exp")
		FText GetLevelIntText();

	/** Get Current Skill Points Text */
	UFUNCTION(BlueprintPure, Category = "Exp")
		FText GetSkillPointsIntText();

	/** Get Saved Exp Data Text */
	UFUNCTION(BlueprintPure, Category = "Exp")
		FText GetSaveExpIntText();

	/** Get Exp Left to Level Up Text */
	UFUNCTION(BlueprintPure, Category = "Exp")
		FText GetExpLeftText();

	/** Prints Level Up message Text */
	UFUNCTION(BlueprintCallable, Category = "Exp")
		void PrintLevelUpMessage();

	FTimerHandle _loopTimerHandle;

	/** Add Exp */
	UFUNCTION(BlueprintCallable, Category = "Exp")
		void UpdateExp(float ExpChange);

	UFUNCTION(BlueprintCallable, Category = "Exp")
		void AddExp();

	UFUNCTION(BlueprintCallable, Category = "Exp")
		void BufferExp();

	/** Level up */
	UFUNCTION(BlueprintCallable, Category = "Exp")
		void LevelUp();

	UFUNCTION(BlueprintCallable, Category = "Exp")
		void LevelCapped();

	/** Exp Save System */
	UFUNCTION(BlueprintCallable, Category = "Exp")
		void SaveGame();

	UFUNCTION(BlueprintCallable, Category = "Exp")
		void SaveData(float SCurrentExp, float SMaxExp, int SCurrentLevel, int SSkillpoints);

	UFUNCTION(BlueprintCallable, Category = "Exp")
		void LoadData();

	bool hasSaved = false;

	/** Pause Menu */
	UPROPERTY(EditAnywhere, Category = "Exp")
		TSubclassOf<class UUserWidget> PauseWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Exp")
		class UUserWidget* PauseWidget;

	//Pause Button function
	UFUNCTION(BlueprintCallable, Category = "Exp")
		void OpenPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Exp")
		void PauseGame(bool isPaused);

	UFUNCTION(BlueprintCallable, Category = "Exp")
		void ResumeGame();

	/** Skill Points Allocation */
	UFUNCTION(BlueprintCallable, Category = "Exp")
		void SkillPointsCost(int Cost);

	//Spend skill points button function
	UFUNCTION(BlueprintCallable, Category = "Exp")
		void SpendSkillPoints();


protected:
	
	/** Fires a projectile. */
	void OnFire();

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

