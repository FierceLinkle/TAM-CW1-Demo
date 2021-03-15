// Copyright Epic Games, Inc. All Rights Reserved.

#include "TAM_CWCharacter.h"
#include "TAM_CWProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Components/TimelineComponent.h"
#include <cmath>
#include "Blueprint/UserWidget.h"
#include "TAM_CWGameMode.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ATAM_CWCharacter

ATAM_CWCharacter::ATAM_CWCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;

	static ConstructorHelpers::FClassFinder<UUserWidget> PauseBar(TEXT("/Game/CW1_Content/PauseUI"));
	PauseWidgetClass = PauseBar.Class;
}

void ATAM_CWCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Initialise variables
	CurrentLevel = 1;
	SkillPoints = 0;
	LevelCap = 100;
	MaxExp = 100.0f;
	CurrentExp = 0.0f;
	PreviousCurrentExp = 0.0f;
	ExpPercentage = 0.0f;
	ExpGrowth = 1.2f;
	ExpOverflow = 0.0f;
	ExpLeft = MaxExp;

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}
}

void ATAM_CWCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Checks for correct condition to level up
	if ((CurrentExp >= MaxExp) && (CurrentLevel < LevelCap)) {
		LevelUp();
	}
	
	//Checks if level cap has been reached
	if (CurrentLevel >= LevelCap) {
		LevelCapped();
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATAM_CWCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATAM_CWCharacter::OnFire);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ATAM_CWCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ATAM_CWCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATAM_CWCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ATAM_CWCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ATAM_CWCharacter::LookUpAtRate);

	//Debug action
	PlayerInputComponent->BindAction("AddExp", IE_Pressed, this, &ATAM_CWCharacter::AddExp);

	//Save and load exp data
	PlayerInputComponent->BindAction("SaveExp", IE_Pressed, this, &ATAM_CWCharacter::SaveGame);
	PlayerInputComponent->BindAction("LoadExp", IE_Pressed, this, &ATAM_CWCharacter::LoadData);

	//Open pause menu
	PlayerInputComponent->BindAction("PauseGame", IE_Pressed, this, &ATAM_CWCharacter::OpenPauseMenu);
}

void ATAM_CWCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<ATAM_CWProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<ATAM_CWProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void ATAM_CWCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ATAM_CWCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void ATAM_CWCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void ATAM_CWCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void ATAM_CWCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ATAM_CWCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ATAM_CWCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ATAM_CWCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool ATAM_CWCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ATAM_CWCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &ATAM_CWCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &ATAM_CWCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}

// UI representation functions //

float ATAM_CWCharacter::GetExp() {
	return ExpPercentage;
}

FText ATAM_CWCharacter::GetExpIntText() {
	//int32 XP = FMath::RoundHalfFromZero(ExpPercentage * 100);
	FString XPS = FString::FromInt(CurrentExp);
	FString FullXPS = FString::FromInt(MaxExp);
	FString XPHUD = XPS + FString(TEXT("/")) + FullXPS;
	FText XPText = FText::FromString(XPHUD);
	return XPText;
}

FText ATAM_CWCharacter::GetLevelIntText() {
	FString LV = FString::FromInt(CurrentLevel);
	FString LVHUD = FString(TEXT("Level: ")) + LV;
	FText LVText = FText::FromString(LVHUD);
	return LVText;
}

FText ATAM_CWCharacter::GetSkillPointsIntText() {
	FString SP = FString::FromInt(SkillPoints);
	FString SPHUD = FString(TEXT("Skill Points: ")) + SP;
	FText SPText = FText::FromString(SPHUD);
	return SPText;
}

FText ATAM_CWCharacter::GetSaveExpIntText() {
	FString CXP = FString::FromInt(ExpProgressData[0]);
	FString MAXXP = FString::FromInt(ExpProgressData[1]);
	FString CL = FString::FromInt(LevelProgressData[0]);
	FString SP = FString::FromInt(LevelProgressData[1]);
	//FString SDHUD = CXP + FString(TEXT(", ")) + MAXXP + FString(TEXT(", ")) + CL + FString(TEXT(", ")) + SP;
	FString SDHUD = FString(TEXT("Level: ")) + CL + FString(TEXT(" SkillPoints ")) + SP;
	FText SDText = FText::FromString(SDHUD);
	return SDText;
}


FText ATAM_CWCharacter::GetExpLeftText() {
	FString EL = FString::FromInt(ExpLeft);
	FString ELHUD = FString(TEXT("Exp left to level up: ")) + EL;
	FText ELText = FText::FromString(ELHUD);
	return ELText;
}


void ATAM_CWCharacter::PrintLevelUpMessage() {
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, TEXT("Level Up!"));
}

// Adds Exp and calculates leftover exp for next level //

void ATAM_CWCharacter::UpdateExp(float ExpChange) {
	if (CurrentLevel < LevelCap) {
		PreviousCurrentExp = CurrentExp;
		CurrentExp = CurrentExp += ExpChange;
		ExpLeft = MaxExp - CurrentExp;
		//CurrentExp = FMath::Clamp(CurrentExp += ExpChange, 0.0f, MaxExp);
		ExpPercentage = CurrentExp / MaxExp;
	}
}

void ATAM_CWCharacter::AddExp() {
	UpdateExp(20.0f);
}

void ATAM_CWCharacter::BufferExp() {
	if (CurrentExp != MaxExp) {
		ExpOverflow = CurrentExp - MaxExp;
		CurrentExp = 0.0f + ExpOverflow;
		CurrentExp = round(CurrentExp);
		ExpOverflow = 0.0f;
		
	}
	else {
		CurrentExp = 0.0f;
	}
	ExpLeft = (MaxExp * ExpGrowth) - CurrentExp;
}

// Handles leveling up //

void ATAM_CWCharacter::LevelUp() {

	CurrentLevel++;
	SkillPoints++;

	BufferExp();

	MaxExp *= ExpGrowth;
	ExpPercentage = CurrentExp / MaxExp;

	GetWorld()->GetTimerManager().SetTimer(_loopTimerHandle, this, &ATAM_CWCharacter::PrintLevelUpMessage, 1.f, false, 0.f);
}

void ATAM_CWCharacter::LevelCapped() {
	MaxExp = CurrentExp;
	ExpOverflow = CurrentExp;
}

// These functions store save data and load them back //

void ATAM_CWCharacter::SaveGame() {
	SaveData(CurrentExp, MaxExp, CurrentLevel, SkillPoints);
	hasSaved = true;
}

void ATAM_CWCharacter::SaveData(float SCurrentExp, float SMaxExp, int SCurrentLevel, int SSkillpoints) {
	ExpProgressData[0] = SCurrentExp;
	ExpProgressData[1] = SMaxExp;
	LevelProgressData[0] = SCurrentLevel;
	LevelProgressData[1] = SSkillpoints;
}

void ATAM_CWCharacter::LoadData() {
	if (hasSaved) {
		CurrentExp = ExpProgressData[0];
		MaxExp = ExpProgressData[1];
		CurrentLevel = LevelProgressData[0];
		SkillPoints = LevelProgressData[1];

		ExpPercentage = ExpProgressData[0] / ExpProgressData[1];
		ExpLeft = MaxExp - CurrentExp;
	}
}

// Handles pause menu and it's features //

void ATAM_CWCharacter::OpenPauseMenu() {
	PauseWidget = CreateWidget<UUserWidget>(GetWorld(), PauseWidgetClass);

	//Add pause UI to viewport
	if (PauseWidgetClass != nullptr)
	{
		if (PauseWidget)
		{
			PauseWidget->AddToViewport();
		}
	}

	PauseGame(true);
}

void ATAM_CWCharacter::ResumeGame() {
	PauseGame(false);
}

void ATAM_CWCharacter::PauseGame(bool isPaused) {
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	PC->bShowMouseCursor = true;
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;

	APlayerController* const MyChar = Cast<APlayerController>(GEngine->GetFirstLocalPlayerController(GetWorld()));

	if (MyChar != NULL)
	{
		MyChar->SetPause(isPaused);
	}

	if (!isPaused) {
		PC->bShowMouseCursor = false;
		PC->bEnableClickEvents = false;
		PC->bEnableMouseOverEvents = false;

		PauseWidget->RemoveFromParent();
		
	}
}

void ATAM_CWCharacter::SpendSkillPoints() {
	SkillPointsCost(1);
}

void ATAM_CWCharacter::SkillPointsCost(int Cost) {
	if (SkillPoints > 0) {
		SkillPoints -= Cost;
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, TEXT("No skill points to spend"));
	}
}

