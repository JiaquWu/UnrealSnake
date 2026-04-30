// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnakeGameMode.generated.h"

class AABoxRoverPawn;
class AFoodActor;
class AGridManager;
class ASnakeGameState;


/**
 * 
 */
UENUM(BlueprintType)
enum class ESnakeMode : uint8
{
	Single UMETA(DisplayName = "Single Player"),
	Coop   UMETA(DisplayName = "Co-op"),
	Battle UMETA(DisplayName = "Battle")
};

USTRUCT(BlueprintType)
struct FSnakeStageConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage")
	int32 RequirementScore = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage")
	float TimeLimit = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage")
	float MoveInterval = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage")
	int32 FoodScore = 10;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStageAdvanced, int32, NewStageIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllStagesCompleted);

UCLASS()
class SNAKE_API ASnakeGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintAssignable, Category = "Snake|Events")
	FOnStageAdvanced OnStageAdvanced;

	UPROPERTY(BlueprintAssignable, Category = "Snake|Events")
	FOnAllStagesCompleted OnAllStagesCompleted;
	
	ASnakeGameMode();
	
	virtual void InitGame(
		const FString& MapName,
		const FString& Options,
		FString& ErrorMessage
	) override;
	
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category = "Snake")
	void StartPlayingRun();
	
	UFUNCTION(BlueprintCallable, Category = "Snake")
	void ReturnToMainMenu();
	
	UFUNCTION(BlueprintCallable, Category = "Snake")
	void RetartRun();
	
	FString GetCurrentModeOption() const;
	
	int32 GetRequiredPlayerCount() const;
	
	int32 GetRequiredFoodCount() const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rules")
	ESnakeMode SnakeMode = ESnakeMode::Single;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stage")
	TArray<FSnakeStageConfig> Stages;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stage")
	int32 CurrentStageIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stage")
	float RemainingStageTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stage")
	int32 StageScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stage")
	int32 TotalScore = 0;
	
	int32 LastDisplayedRemainingSeconds = -1;

	
private:
	UPROPERTY(EditDefaultsOnly, Category= "Snake")
	TSubclassOf<AABoxRoverPawn> SnakePawnClass;
	
	UPROPERTY()
	TArray<TObjectPtr<AABoxRoverPawn>> SpawnedSnakePawns;
	
	UPROPERTY(EditDefaultsOnly, Category= "Snake")
	TSubclassOf<AFoodActor> FoodActorClass;
	
	UPROPERTY()
	TObjectPtr<AABoxRoverPawn> SpawnedSnakePawn;
	
	UPROPERTY()
	TArray<TObjectPtr<AFoodActor>> SpawnedFoodActors;
	
	UPROPERTY()
	TObjectPtr<AGridManager> GridManager;
	
	void EnsureLocalPlayers();
	void SpawnAllSnakes();
	void SpawnSnakeForPlayer(int32 PlayerIndex);
	void CacheGridManager();
	//void SpawnSnake();
	void SpawnFoods();
	void MoveFoodToRandomFreeCell(AFoodActor* FoodToMove);
	void MoveAllFoodsToRandomFreeCells();
	
	void StartCurrentStage();
	void AdvanceToNextStage();
	void HandleStageTimeExpired();
	bool HasMetStageRequirement() const;
	const FSnakeStageConfig* GetCurrentStageConfig() const;
	
	UFUNCTION()
	void HandleFoodConsumed(int32 PlayerIndex, int32 ScoreValue, AFoodActor* ConsumedFood);
	
	UFUNCTION()
	void HandleSnakeDeath();
	
	ASnakeGameState* GetSnakeGameState() const;
	
	
};
