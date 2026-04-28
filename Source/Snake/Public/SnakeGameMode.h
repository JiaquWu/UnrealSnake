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

UCLASS()
class SNAKE_API ASnakeGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
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
	TObjectPtr<AFoodActor> SpawnedFoodActor;
	
	UPROPERTY()
	TObjectPtr<AGridManager> GridManager;
	
	void EnsureLocalPlayers();
	void SpawnAllSnakes();
	void SpawnSnakeForPlayer(int32 PlayerIndex);
	void CacheGridManager();
	//void SpawnSnake();
	void SpawnFood();
	void MoveFoodToRandomFreeCell();
	
	void StartCurrentStage();
	void AdvanceToNextStage();
	void HandleStageTimeExpired();
	bool HasMetStageRequirement() const;
	const FSnakeStageConfig* GetCurrentStageConfig() const;
	
	UFUNCTION()
	void HandleFoodConsumed(int32 ScoreValue);
	
	UFUNCTION()
	void HandleSnakeDeath();
	
	ASnakeGameState* GetSnakeGameState() const;
	
	
};
