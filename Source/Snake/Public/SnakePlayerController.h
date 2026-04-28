// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SnakePlayerController.generated.h"

class UUserWidget;
class UInputMappingContext;
class UInputAction;
class AABoxRoverPawn;
struct FInputActionValue;
/**
 * 
 */
UCLASS()
class SNAKE_API ASnakePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION(BlueprintCallable)
	void ShowHUD();
	
	UFUNCTION(BlueprintCallable)
	void ShowMainMenu();
	
	UFUNCTION(BlueprintCallable)
	void ShowOutro();
	
	UFUNCTION(BlueprintCallable)
	void HideAllWidgets();
	
	virtual void SetupInputComponent() override;

	void SetControlledSnake(AABoxRoverPawn* NewSnake);
	
	void ApplyControlledSnakeCamera();
	
protected:
	
	virtual void BeginPlayingState() override;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> IA_Up;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> IA_Down;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> IA_Left;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> IA_Right;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> OutroWidgetClass;
	
private:
	
	UPROPERTY()
	TObjectPtr<UUserWidget> ActiveWidget;
	
	UPROPERTY()
	TObjectPtr<AABoxRoverPawn> ControlledSnake;
	
	void MoveUp(const FInputActionValue& Value);
	void MoveDown(const FInputActionValue& Value);
	void MoveLeft(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
};
