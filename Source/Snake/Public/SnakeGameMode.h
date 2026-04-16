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
UCLASS()
class SNAKE_API ASnakeGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable, Category = "Snake")
	void StartPlayingRun();
	
	UFUNCTION(BlueprintCallable, Category = "Snake")
	void ReturnToMainMenu();
	
	UFUNCTION(BlueprintCallable, Category = "Snake")
	void RetartRun();
	
private:
	UPROPERTY(EditDefaultsOnly, Category= "Snake")
	TSubclassOf<AABoxRoverPawn> SnakePawnClass;
	
	UPROPERTY(EditDefaultsOnly, Category= "Snake")
	TSubclassOf<AFoodActor> FoodActorClass;
	
	UPROPERTY()
	TObjectPtr<AABoxRoverPawn> SpawnedSnakePawn;
	
	UPROPERTY()
	TObjectPtr<AFoodActor> SpawnedFoodActor;
	
	UPROPERTY()
	TObjectPtr<AGridManager> GridManager;
	
	void CacheGridManager();
	void SpawnSnake();
	void SpawnFood();
	void MoveFoodToRandomFreeCell();
	
	UFUNCTION()
	void HandleFoodConsumed(int32 ScoreValue);
	
	UFUNCTION()
	void HandleSnakeDeath();
	
	ASnakeGameState* GetSnakeGameState() const;
	
	
};
