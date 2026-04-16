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

/**
 * 
 */
UCLASS()
class SNAKE_API ASnakeGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, Category= "Snake")
	int32 Score = 0;
	
	UPROPERTY(BlueprintReadOnly, Category= "Snake")
	ESnakeGameState GameState = ESnakeGameState::Playing;
};
