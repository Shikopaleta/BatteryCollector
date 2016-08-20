// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "BatteryCollector.h"
#include "BatteryCollectorGameMode.h"
#include "BatteryCollectorCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "BLueprint/UserWidget.h"
#include "SpawnVolume.h"

ABatteryCollectorGameMode::ABatteryCollectorGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{	
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// Base decay rate
	DecayRate = 0.01f;
}

void ABatteryCollectorGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Find all SpawnVolume actors
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundActors);

	for (auto Actor : FoundActors)
	{
		// Find only SpawnVolume actors
		ASpawnVolume* SpawnVolumeActor = Cast<ASpawnVolume>(Actor);
		{
			if (SpawnVolumeActor)
			{
				// Add to the array of SpawnVolumeActors
				SpawnVolumeActors.AddUnique(SpawnVolumeActor);
			}
		}
	}

	// Set the game state to playing when the game starts
	SetCurrentState(EBatteryPlayState::EPlaying);

	// Set the score to win
	ABatteryCollectorCharacter* MyCharacter = Cast<ABatteryCollectorCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		PowerToWin = (MyCharacter->GetInitialPower()) * 1.25f;
	}

	if (HUDWidghetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidghetClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}

	
}

void ABatteryCollectorGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Check that we are using the battery collector character
	ABatteryCollectorCharacter* MyCharacter = Cast<ABatteryCollectorCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (MyCharacter)
	{
		// Win condition
		if (MyCharacter->GetCurrentPower() >= PowerToWin)
		{
			// Set game state to won
			SetCurrentState(EBatteryPlayState::EWon);
		}
		// If the character's power is posiive
		else if (MyCharacter->GetCurrentPower() > 0)
		{
			// Decrease the character's power using the decay rate
			MyCharacter->UpdatePower(-DeltaTime*DecayRate*(MyCharacter->GetInitialPower()));
		}
		// When the character's power is less than 0
		else
		{
			SetCurrentState(EBatteryPlayState::EGameOver);
		}
		
	}
}

// Returns the power to win
float ABatteryCollectorGameMode::GetPowerToWin() const
{
	return PowerToWin;
}

// Returns the current play state
EBatteryPlayState ABatteryCollectorGameMode::GetCurrentState() const
{
	return PlayState;
}

// Sets the current play state
void ABatteryCollectorGameMode::SetCurrentState(EBatteryPlayState NewState)
{
	PlayState = NewState;
	HandleState(PlayState);
}

void ABatteryCollectorGameMode::HandleState(EBatteryPlayState NewState)
{
	switch (PlayState)
	{
	// If the game is playing
	case EPlaying:
		// Make Spawn volumes active
		for (ASpawnVolume* Volume : SpawnVolumeActors)	
		{
			Volume->SetSpawningActive(true);
		}
		break;

	// If we've lost the game
	case EGameOver:
	{
		// Make spawn volumes inactive
		for (ASpawnVolume* Volume : SpawnVolumeActors)
		{
			Volume->SetSpawningActive(false);
		}

		// Block player input
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (PlayerController)
		{
			PlayerController->SetCinematicMode(true, false, false, true, true);
		}

		// Ragdoll the character
		ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
		if (PlayerCharacter)
		{
			PlayerCharacter->GetMesh()->SetSimulatePhysics(true);
			PlayerCharacter->GetMovementComponent()->MovementState.bCanJump = false;
		}
	}

		break;

	// If we've won the game
	case EWon:
		// Make spawn volumes inactive
		for (ASpawnVolume* Volume : SpawnVolumeActors)
		{
			Volume->SetSpawningActive(false);
		}
		break;

	// Default
	case EUnknown:
	default:
		break;
	}
}