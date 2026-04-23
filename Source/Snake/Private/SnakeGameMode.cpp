// Fill out your copyright notice in the Description page of Project Settings.


#include "Snake/Public/SnakeGameMode.h"
#include "Snake/Public/SnakeGameState.h"
#include "Snake/ABoxRoverPawn.h"
#include "FoodActor.h"
#include "Snake/GridManager.h"
#include "Kismet//GameplayStatics.h"


void ASnakeGameMode::BeginPlay()
{
	Super::BeginPlay();
	CacheGridManager();
	StartPlayingRun();
}

void ASnakeGameMode::CacheGridManager()
{
	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(this, AGridManager::StaticClass()));
	
}

ASnakeGameState* ASnakeGameMode::GetSnakeGameState() const
{
	return GetGameState<ASnakeGameState>();
}

void ASnakeGameMode::StartPlayingRun()
{
	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->Score = 0;
		GS->GameState = ESnakeGameState::Playing;
	}
	
	SpawnSnake();
	SpawnFood();
	MoveFoodToRandomFreeCell();
}

void ASnakeGameMode::SpawnSnake()
{
	if (!SnakePawnClass || !GridManager)
	{
		return;
	}
	
	if (SpawnedSnakePawn)
	{
		SpawnedSnakePawn->Destroy();
		SpawnedSnakePawn = nullptr;
	}
	
	const FIntPoint SnakeSpawnCell(10,10);
	const FVector SnakeSpawnLocation = GridManager->GetCellWorldPosition(SnakeSpawnCell);
	
	SpawnedSnakePawn = GetWorld()->SpawnActor<AABoxRoverPawn>
	(SnakePawnClass, SnakeSpawnLocation, FRotator::ZeroRotator);
	
	if (!SpawnedSnakePawn)
	{
		return;
	}
	
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->Possess(SpawnedSnakePawn);
	}
	
	SpawnedSnakePawn->OnFoodConsumed.AddDynamic(this, &ASnakeGameMode::HandleFoodConsumed);
	SpawnedSnakePawn->OnSnakeDied.AddDynamic(this, &ASnakeGameMode::HandleSnakeDeath);
	
	
	
}

void ASnakeGameMode::SpawnFood()
{
	if (!FoodActorClass ||!GridManager)
		return;;
	
	if (SpawnedFoodActor)
	{
		SpawnedFoodActor->Destroy();
		SpawnedFoodActor = nullptr;
	}
	
	SpawnedFoodActor = GetWorld()->SpawnActor<AFoodActor>(FoodActorClass, FVector::ZeroVector, FRotator::ZeroRotator);
}

void ASnakeGameMode::MoveFoodToRandomFreeCell()
{
	if (!GridManager || !SpawnedFoodActor || !SpawnedSnakePawn)
		return;
	
	TArray<FIntPoint> ForbiddenCells = SpawnedSnakePawn->GetAllOccupiedGridCells();
	
	FIntPoint NewFoodCell;
	if (GridManager->TryGetRandomFreeCell(NewFoodCell, ForbiddenCells))
	{
		SpawnedFoodActor->RespawnFood(NewFoodCell, GridManager->GetCellWorldPosition(NewFoodCell));
	}
}

void ASnakeGameMode::HandleFoodConsumed(int32 ScoreValue)
{
	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->Score += ScoreValue;
	}
	
	MoveFoodToRandomFreeCell();
	
}

void ASnakeGameMode::HandleSnakeDeath()
{
	
	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->GameState = ESnakeGameState::Outro;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("GameMode HandleSnakeDeath received"));
}

void ASnakeGameMode::RetartRun()
{
	StartPlayingRun();
}

void ASnakeGameMode::ReturnToMainMenu()
{
	UGameplayStatics::OpenLevel(this, FName(TEXT("MainMenuMap")));
}
