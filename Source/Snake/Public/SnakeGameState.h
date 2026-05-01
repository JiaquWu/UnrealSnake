// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SnakeGameState.generated.h"

UENUM(BlueprintType)
enum class ESnakeGameState : uint8
{
	Menu,
	Playing,
	Outro
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStageChanged, int32, NewStageIndex, int32, RequirementScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerChanged, float, RemainingTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSnakeGameStateChanged, ESnakeGameState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnBattleScoresChanged,
	int32, Player0Score,
	int32, Player1Score
);
/**
 * 
 */
UCLASS()
class SNAKE_API ASnakeGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	int32 Score = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentStageIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 RequirementScore = 0;

	UPROPERTY(BlueprintReadOnly)
	float RemainingStageTime = 0.f;

	UPROPERTY(BlueprintReadOnly)
	ESnakeGameState GameState = ESnakeGameState::Menu;

	UPROPERTY(BlueprintAssignable)
	FOnScoreChanged OnScoreChanged;

	UPROPERTY(BlueprintAssignable)
	FOnStageChanged OnStageChanged;

	UPROPERTY(BlueprintAssignable)
	FOnTimerChanged OnTimerChanged;

	UPROPERTY(BlueprintAssignable)
	FOnSnakeGameStateChanged OnGameStateChanged;
	
	UPROPERTY(BlueprintReadOnly)
	int32 Player0Score = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Player1Score = 0;

	UPROPERTY(BlueprintAssignable)
	FOnBattleScoresChanged OnBattleScoresChanged;

	void SetBattleScores(int32 NewPlayer0Score, int32 NewPlayer1Score);

	void SetScore(int32 NewScore);
	void SetStage(int32 NewStageIndex, int32 NewRequirementScore);
	void SetRemainingStageTime(float NewRemainingTime);
	void SetSnakeGameState(ESnakeGameState NewState);
};
