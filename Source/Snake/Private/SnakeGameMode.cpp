// Fill out your copyright notice in the Description page of Project Settings.


#include "Snake/Public/SnakeGameMode.h"
#include "Snake/Public/SnakeGameState.h"
#include "Snake/ABoxRoverPawn.h"
#include "FoodActor.h"
#include "Snake/GridManager.h"
#include "SnakePlayerController.h"
#include "Kismet//GameplayStatics.h"

ASnakeGameMode::ASnakeGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}


void ASnakeGameMode::BeginPlay()
{
	Super::BeginPlay();
	CacheGridManager();
	StartPlayingRun();
}

void ASnakeGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	const FString ModeString = UGameplayStatics::ParseOption(Options, TEXT("Mode"));

	if (ModeString.Equals(TEXT("Single"), ESearchCase::IgnoreCase))
	{
		SnakeMode = ESnakeMode::Single;
	}
	else if (ModeString.Equals(TEXT("Coop"), ESearchCase::IgnoreCase))
	{
		SnakeMode = ESnakeMode::Coop;
	}
	else if (ModeString.Equals(TEXT("Battle"), ESearchCase::IgnoreCase))
	{
		SnakeMode = ESnakeMode::Battle;
	}

	UE_LOG(LogTemp, Warning, TEXT("InitGame Options=%s | ModeString=%s | SnakeMode=%s"),
		*Options,
		*ModeString,
		*UEnum::GetValueAsString(SnakeMode));
}

void ASnakeGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ASnakeGameState* GS = GetSnakeGameState();
	if (!GS || GS->GameState != ESnakeGameState::Playing)
	{
		return;
	}

	if (Stages.Num() <= 0)
	{
		return;
	}

	RemainingStageTime -= DeltaSeconds;

	const int32 DisplaySeconds = FMath::CeilToInt(RemainingStageTime);

	if (DisplaySeconds != LastDisplayedRemainingSeconds)
	{
		LastDisplayedRemainingSeconds = DisplaySeconds;

		if (GS)
		{
			GS->SetRemainingStageTime(FMath::Max(0.f, RemainingStageTime));
		}
	}

	if (RemainingStageTime <= 0.f)
	{
		RemainingStageTime = 0.f;
		HandleStageTimeExpired();
	}
}

int32 ASnakeGameMode::GetRequiredPlayerCount() const
{
	switch (SnakeMode)
	{
	case ESnakeMode::Single:
		return 1;

	case ESnakeMode::Coop:
	case ESnakeMode::Battle:
		return 2;

	default:
		return 1;
	}
}

int32 ASnakeGameMode::GetRequiredFoodCount() const
{
	switch (SnakeMode)
	{
	case ESnakeMode::Single:
		return 1;

	case ESnakeMode::Coop:
	case ESnakeMode::Battle:
		return 2;

	default:
		return 1;
	}
}

void ASnakeGameMode::CacheGridManager()
{
	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(this, AGridManager::StaticClass()));
	
}

ASnakeGameState* ASnakeGameMode::GetSnakeGameState() const
{
	return GetGameState<ASnakeGameState>();
}

void ASnakeGameMode::EnsureLocalPlayers()
{
	const int32 RequiredPlayerCount = GetRequiredPlayerCount();

	UE_LOG(LogTemp, Warning, TEXT("EnsureLocalPlayers RequiredPlayerCount=%d"), RequiredPlayerCount);

	for (int32 PlayerIndex = 1; PlayerIndex < RequiredPlayerCount; PlayerIndex++)
	{
		APlayerController* ExistingPC = UGameplayStatics::GetPlayerController(this, PlayerIndex);

		UE_LOG(LogTemp, Warning, TEXT("Before CreatePlayer Index=%d ExistingPC=%s"),
			PlayerIndex,
			*GetNameSafe(ExistingPC));

		if (!ExistingPC)
		{
			APlayerController* NewPC = UGameplayStatics::CreatePlayer(this, -1, true);

			UE_LOG(LogTemp, Warning, TEXT("CreatePlayer result for Index=%d: %s"),
				PlayerIndex,
				*GetNameSafe(NewPC));
		}

		APlayerController* AfterPC = UGameplayStatics::GetPlayerController(this, PlayerIndex);

		UE_LOG(LogTemp, Warning, TEXT("After CreatePlayer Index=%d PC=%s"),
			PlayerIndex,
			*GetNameSafe(AfterPC));
	}
}

void ASnakeGameMode::StartPlayingRun()
{
	CurrentStageIndex = 0;
	StageScore = 0;
	TotalScore = 0;
	
	const int32 PlayerCount = GetRequiredPlayerCount();
	PlayerTotalScores.Init(0, PlayerCount);
	
	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->SetScore(0);
		GS->SetSnakeGameState(ESnakeGameState::Playing);
	}
	
	EnsureLocalPlayers();
	SpawnAllSnakes();
	SpawnFoods();
	
	StartCurrentStage();
	
	MoveAllFoodsToRandomFreeCells();
	
	if (ASnakePlayerController* PC = Cast<ASnakePlayerController>(
		UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->ShowHUD();
	}
}

void ASnakeGameMode::SpawnAllSnakes()
{
	if (!SnakePawnClass || !GridManager)
	{
		return;
	}

	for (AABoxRoverPawn* Snake : SpawnedSnakePawns)
	{
		if (Snake)
		{
			Snake->Destroy();
		}
	}

	SpawnedSnakePawns.Empty();
	SpawnedSnakePawn = nullptr;

	const int32 PlayerCount = GetRequiredPlayerCount();

	for (int32 PlayerIndex = 0; PlayerIndex < PlayerCount; PlayerIndex++)
	{
		SpawnSnakeForPlayer(PlayerIndex);
	}

	ConfigureSnakeInteractions();
	
	if (SpawnedSnakePawns.IsValidIndex(0))
	{
		SpawnedSnakePawn = SpawnedSnakePawns[0];
	}
}

void ASnakeGameMode::SpawnSnakeForPlayer(int32 PlayerIndex)
{
	// APlayerController* PC = UGameplayStatics::GetPlayerController(this, PlayerIndex);
	// if (!PC)
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("No PlayerController for PlayerIndex %d, skip spawning snake"), PlayerIndex);
	// 	return;
	// }

	const TArray<FIntVector> SpawnCells =
	{
		FIntVector(4, 4, 0),
		FIntVector(11, 11, 0)
	};

	if (!SpawnCells.IsValidIndex(PlayerIndex))
	{
		return;
	}

	const FVector SnakeSpawnLocation = GridManager->GetCellWorldPosition(SpawnCells[PlayerIndex]);

	AABoxRoverPawn* NewSnake = GetWorld()->SpawnActor<AABoxRoverPawn>(
		SnakePawnClass,
		SnakeSpawnLocation,
		FRotator::ZeroRotator
	);

	if (!NewSnake)
	{
		return;
	}
	
	NewSnake->SetPlayerIndex(PlayerIndex);
	
	SpawnedSnakePawns.Add(NewSnake);

	if (ASnakePlayerController* PC = Cast<ASnakePlayerController>(
	UGameplayStatics::GetPlayerController(this, PlayerIndex)))
	{
		PC->Possess(NewSnake);
		//PC->SetViewTargetWithBlend(NewSnake, 0.f);
		PC->SetControlledSnake(NewSnake);

		UE_LOG(LogTemp, Warning, TEXT(
			"PlayerIndex=%d | PC=%s | Pawn=%s | ViewTarget=%s | ControlledSnake=%s"
		),
			PlayerIndex,
			*GetNameSafe(PC),
			*GetNameSafe(PC->GetPawn()),
			*GetNameSafe(PC->GetViewTarget()),
			*GetNameSafe(NewSnake));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No SnakePlayerController for PlayerIndex %d"), PlayerIndex);
	}
	
	// PC->Possess(NewSnake);
	// PC->SetViewTargetWithBlend(NewSnake, 0.f);
	//
	// UE_LOG(LogTemp, Warning, TEXT(
	// "PlayerIndex=%d | PC=%s | Pawn=%s | ViewTarget=%s | LocalPlayer=%s"
	// ),
	// PlayerIndex,
	// *GetNameSafe(PC),
	// *GetNameSafe(PC->GetPawn()),
	// *GetNameSafe(PC->GetViewTarget()),
	// *GetNameSafe(PC->GetLocalPlayer()));

	NewSnake->OnFoodConsumed.AddDynamic(this, &ASnakeGameMode::HandleFoodConsumed);
	NewSnake->OnSnakeDied.AddDynamic(this, &ASnakeGameMode::HandleSnakeDeath);
}

// void ASnakeGameMode::SpawnSnake()
// {
// 	if (!SnakePawnClass || !GridManager)
// 	{
// 		return;
// 	}
// 	
// 	if (SpawnedSnakePawn)
// 	{
// 		SpawnedSnakePawn->Destroy();
// 		SpawnedSnakePawn = nullptr;
// 	}
// 	
// 	const FIntPoint SnakeSpawnCell(10,10);
// 	const FVector SnakeSpawnLocation = GridManager->GetCellWorldPosition(SnakeSpawnCell);
// 	
// 	SpawnedSnakePawn = GetWorld()->SpawnActor<AABoxRoverPawn>
// 	(SnakePawnClass, SnakeSpawnLocation, FRotator::ZeroRotator);
// 	
// 	if (!SpawnedSnakePawn)
// 	{
// 		return;
// 	}
// 	
// 	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
// 	{
// 		PC->Possess(SpawnedSnakePawn);
// 	}
// 	
// 	SpawnedSnakePawn->OnFoodConsumed.AddDynamic(this, &ASnakeGameMode::HandleFoodConsumed);
// 	SpawnedSnakePawn->OnSnakeDied.AddDynamic(this, &ASnakeGameMode::HandleSnakeDeath);
// 	
// 	
// 	
// }

void ASnakeGameMode::SpawnFoods()
{
	if (!FoodActorClass || !GridManager)
	{
		return;
	}

	for (AFoodActor* Food : SpawnedFoodActors)
	{
		if (Food)
		{
			Food->Destroy();
		}
	}

	SpawnedFoodActors.Empty();

	const int32 FoodCount = GetRequiredFoodCount();

	for (int32 i = 0; i < FoodCount; i++)
	{
		AFoodActor* NewFood = GetWorld()->SpawnActor<AFoodActor>(
			FoodActorClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator
		);

		if (NewFood)
		{
			SpawnedFoodActors.Add(NewFood);
		}
	}
}

void ASnakeGameMode::StartCurrentStage()
{
	const FSnakeStageConfig* Stage = GetCurrentStageConfig();
	if (!Stage)
	{
		UE_LOG(LogTemp, Error, TEXT("No valid stage config."));
		return;
	}

	StageScore = 0;
	RemainingStageTime = Stage->TimeLimit;
	
	const int32 PlayerCount = GetRequiredPlayerCount();
	PlayerStageScores.Init(0, PlayerCount);

	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		if (SnakeMode == ESnakeMode::Battle)
		{
			GS->SetBattleScores(0, 0);
		}
		else
		{
			GS->SetScore(StageScore);
		}
		
		//GS->SetScore(StageScore);
		GS->SetStage(CurrentStageIndex, Stage->RequirementScore);
		GS->SetRemainingStageTime(RemainingStageTime);
	}

	if (SpawnedSnakePawn)
	{
		for (AABoxRoverPawn* Snake : SpawnedSnakePawns)
		{
			if (Snake)
			{
				Snake->SetMoveInterval(Stage->MoveInterval);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Start Stage %d | Requirement: %d | Time: %.1f | MoveInterval: %.2f"),
		CurrentStageIndex + 1,
		Stage->RequirementScore,
		Stage->TimeLimit,
		Stage->MoveInterval);
}

const FSnakeStageConfig* ASnakeGameMode::GetCurrentStageConfig() const
{
	if (!Stages.IsValidIndex(CurrentStageIndex))
	{
		return nullptr;
	}

	return &Stages[CurrentStageIndex];
}

bool ASnakeGameMode::HasMetStageRequirement() const
{
	const FSnakeStageConfig* Stage = GetCurrentStageConfig();
	if (!Stage)
	{
		return false;
	}

	switch (SnakeMode)
	{
	case ESnakeMode::Single:
	case ESnakeMode::Coop:
		return StageScore >= Stage->RequirementScore;

	case ESnakeMode::Battle:
		{
			const int32 PlayerCount = GetRequiredPlayerCount();

			for (int32 i = 0; i < PlayerCount; i++)
			{
				if (!PlayerStageScores.IsValidIndex(i) ||
					PlayerStageScores[i] < Stage->RequirementScore)
				{
					return false;
				}
			}

			return true;
		}

	default:
		return false;
	}
}

void ASnakeGameMode::AdvanceToNextStage()
{
	CurrentStageIndex++;

	if (!Stages.IsValidIndex(CurrentStageIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("All stages completed!"));
		OnAllStagesCompleted.Broadcast();
		
		if (ASnakeGameState* GS = GetSnakeGameState())
		{
			GS->SetSnakeGameState(ESnakeGameState::Outro);
			//you win?
		}

		ShowOutroForAllPlayers();

		return;
	}

	OnStageAdvanced.Broadcast(CurrentStageIndex);
	
	StartCurrentStage();
	MoveAllFoodsToRandomFreeCells();
}

void ASnakeGameMode::MoveAllFoodsToRandomFreeCells()
{
	for (AFoodActor* Food : SpawnedFoodActors)
	{
		if (Food)
		{
			MoveFoodToRandomFreeCell(Food);
		}
	}
}

void ASnakeGameMode::HandleStageTimeExpired()
{
	if (SnakeMode == ESnakeMode::Battle)
	{
		ResolveBattleStageTimeExpired();
		return;
	}

	if (HasMetStageRequirement())
	{
		AdvanceToNextStage();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage failed. Score: %d"), StageScore);
		EndRun();
	}
}

void ASnakeGameMode::MoveFoodToRandomFreeCell(AFoodActor* FoodToMove)
{
	if (!GridManager || !FoodToMove)
	{
		return;
	}

	TArray<FIntVector> ForbiddenCells;

	for (AABoxRoverPawn* Snake : SpawnedSnakePawns)
	{
		if (Snake)
		{
			ForbiddenCells.Append(Snake->GetAllOccupiedGridCells());
		}
	}

	for (AFoodActor* Food : SpawnedFoodActors)
	{
		if (Food && Food != FoodToMove && Food->IsFoodActive())
		{
			ForbiddenCells.Add(Food->GetFoodGridPosition());
		}
	}

	FIntVector NewFoodCell;
	if (GridManager->TryGetRandomFreeCell(NewFoodCell, ForbiddenCells))
	{
		FoodToMove->RespawnFood(
			NewFoodCell,
			GridManager->GetCellWorldPosition(NewFoodCell)
		);
	}
}

void ASnakeGameMode::HandleFoodConsumed(int32 PlayerIndex, int32 ScoreValue, AFoodActor* ConsumedFood)
{
	const FSnakeStageConfig* Stage = GetCurrentStageConfig();
	const int32 FinalScoreValue = Stage ? Stage->FoodScore : ScoreValue;

	if (SnakeMode == ESnakeMode::Battle)
	{
		if (PlayerStageScores.IsValidIndex(PlayerIndex))
		{
			PlayerStageScores[PlayerIndex] += FinalScoreValue;
		}

		if (PlayerTotalScores.IsValidIndex(PlayerIndex))
		{
			PlayerTotalScores[PlayerIndex] += FinalScoreValue;
		}

		if (ASnakeGameState* GS = GetSnakeGameState())
		{
			const int32 P0Score = PlayerStageScores.IsValidIndex(0) ? PlayerStageScores[0] : 0;
			const int32 P1Score = PlayerStageScores.IsValidIndex(1) ? PlayerStageScores[1] : 0;

			GS->SetBattleScores(P0Score, P1Score);
		}
	}
	else
	{
		StageScore += FinalScoreValue;
		TotalScore += FinalScoreValue;

		if (ASnakeGameState* GS = GetSnakeGameState())
		{
			GS->SetScore(StageScore);
		}
	}

	if (HasMetStageRequirement())
	{
		AdvanceToNextStage();
		return;
	}

	MoveFoodToRandomFreeCell(ConsumedFood);
}

void ASnakeGameMode::HandleSnakeDeath(int32 DeadPlayerIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("GameMode HandleSnakeDeath received. DeadPlayer=%d"),
		DeadPlayerIndex);

	if (SnakeMode == ESnakeMode::Battle)
	{
		const int32 WinnerIndex = DeadPlayerIndex == 0 ? 1 : 0;
		EndBattleWithWinner(WinnerIndex);
		return;
	}

	EndRun();
}

void ASnakeGameMode::RetartRun()
{
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName), true, GetCurrentModeOption());
}

FString ASnakeGameMode::GetCurrentModeOption() const
{
	switch (SnakeMode)
	{
	case ESnakeMode::Single:
		return TEXT("Mode=Single");

	case ESnakeMode::Coop:
		return TEXT("Mode=Coop");

	case ESnakeMode::Battle:
		return TEXT("Mode=Battle");

	default:
		return TEXT("Mode=Single");
	}
}

void ASnakeGameMode::ReturnToMainMenu()
{
	UGameplayStatics::OpenLevel(this, FName(TEXT("MainMenuMap")));
}

void ASnakeGameMode::ResolveBattleStageTimeExpired()
{
	const FSnakeStageConfig* Stage = GetCurrentStageConfig();
	if (!Stage)
	{
		return;
	}

	const bool bP0Passed =
		PlayerStageScores.IsValidIndex(0) &&
		PlayerStageScores[0] >= Stage->RequirementScore;

	const bool bP1Passed =
		PlayerStageScores.IsValidIndex(1) &&
		PlayerStageScores[1] >= Stage->RequirementScore;

	if (bP0Passed && bP1Passed)
	{
		AdvanceToNextStage();
		return;
	}

	if (bP0Passed && !bP1Passed)
	{
		EndBattleWithWinner(0);
		return;
	}

	if (!bP0Passed && bP1Passed)
	{
		EndBattleWithWinner(1);
		return;
	}

	const int32 P0Score = PlayerStageScores.IsValidIndex(0) ? PlayerStageScores[0] : 0;
	const int32 P1Score = PlayerStageScores.IsValidIndex(1) ? PlayerStageScores[1] : 0;

	if (P0Score > P1Score)
	{
		EndBattleWithWinner(0);
	}
	else if (P1Score > P0Score)
	{
		EndBattleWithWinner(1);
	}
	else
	{
		EndBattleDraw();
	}
}

void ASnakeGameMode::EndBattleWithWinner(int32 WinnerPlayerIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Battle ended. Winner: Player %d"), WinnerPlayerIndex);

	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->SetSnakeGameState(ESnakeGameState::Outro);
	}

	ShowOutroForAllPlayers();
}

void ASnakeGameMode::EndBattleDraw()
{
	UE_LOG(LogTemp, Warning, TEXT("Battle ended in draw."));

	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->SetSnakeGameState(ESnakeGameState::Outro);
	}

	ShowOutroForAllPlayers();
}

void ASnakeGameMode::ShowOutroForAllPlayers()
{
	const int32 PlayerCount = GetRequiredPlayerCount();

	for (int32 i = 0; i < PlayerCount; i++)
	{
		if (ASnakePlayerController* PC = Cast<ASnakePlayerController>(
			UGameplayStatics::GetPlayerController(this, i)))
		{
			PC->ShowOutro();
		}
	}
}


void ASnakeGameMode::ConfigureSnakeInteractions()
{
	TArray<AABoxRoverPawn*> AllSnakes;

	for (AABoxRoverPawn* Snake : SpawnedSnakePawns)
	{
		if (Snake)
		{
			AllSnakes.Add(Snake);
		}
	}

	const bool bBattle = SnakeMode == ESnakeMode::Battle;

	for (AABoxRoverPawn* Snake : AllSnakes)
	{
		if (!Snake)
		{
			continue;
		}

		Snake->SetOtherSnakes(AllSnakes);
		Snake->SetCanHitOtherSnakes(bBattle);
	}
}


void ASnakeGameMode::EndRun()
{
	if (ASnakeGameState* GS = GetSnakeGameState())
	{
		GS->SetSnakeGameState(ESnakeGameState::Outro);
	}

	ShowOutroForAllPlayers();
}
