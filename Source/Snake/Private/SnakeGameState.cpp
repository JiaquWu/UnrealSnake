// Fill out your copyright notice in the Description page of Project Settings.


#include "SnakeGameState.h"

void ASnakeGameState::SetScore(int32 NewScore)
{
	if (Score == NewScore)
	{
		return;
	}

	Score = NewScore;
	OnScoreChanged.Broadcast(Score);
}

void ASnakeGameState::SetStage(int32 NewStageIndex, int32 NewRequirementScore)
{
	CurrentStageIndex = NewStageIndex;
	RequirementScore = NewRequirementScore;

	OnStageChanged.Broadcast(CurrentStageIndex, RequirementScore);
}

void ASnakeGameState::SetRemainingStageTime(float NewRemainingTime)
{
	RemainingStageTime = NewRemainingTime;
	OnTimerChanged.Broadcast(RemainingStageTime);
}

void ASnakeGameState::SetSnakeGameState(ESnakeGameState NewState)
{
	if (GameState == NewState)
	{
		return;
	}

	GameState = NewState;
	OnGameStateChanged.Broadcast(GameState);
}

void ASnakeGameState::SetBattleScores(int32 NewPlayer0Score, int32 NewPlayer1Score)
{
	Player0Score = NewPlayer0Score;
	Player1Score = NewPlayer1Score;

	OnBattleScoresChanged.Broadcast(Player0Score, Player1Score);
}