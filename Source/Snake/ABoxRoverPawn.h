// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "ABoxRoverPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFoodConsumed, int32, Score);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSnakeDied);

class UInputMappingContext;
class UStaticMeshComponent;
class USphereComponent;
class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class AFoodActor;

struct FInputActionValue;


UENUM(BlueprintType)
enum class ESnakeDirection : uint8
{
	Up,
	Down,
	Left,
	Right
};


UCLASS()
class SNAKE_API AABoxRoverPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AABoxRoverPawn();
	
	UFUNCTION()
	void HandleFoodOverlap(AFoodActor* FoodActor);
	
	UFUNCTION()
	void HandleFoodDeath();
	
	UPROPERTY(BlueprintAssignable, Category="Snake|Events")
	FOnFoodConsumed OnFoodConsumed;
	
	UPROPERTY(BlueprintAssignable, Category="Snake|Events")
	FOnSnakeDied OnSnakeDied;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// void Move(const FInputActionValue& Value);
	// void Turn(const FInputActionValue& Value);
	
	void MoveUp(const FInputActionValue& Value);
	void MoveDown(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void MoveLeft(const FInputActionValue& Value);
	
	void TickFreeMovement(float DeltaTime);
	void UpdateDirection(ESnakeDirection NewDirection);
	void HandleDirectionChange();
	bool IsValidTurn(ESnakeDirection NewDirection) const;
	
	void GrowSnake(int32 Growth);
	
	FVector GetVectorFromDirection(ESnakeDirection Direction) const;
	
	UPROPERTY(BlueprintReadOnly, Category = Input)
	TArray<FIntPoint> CurrentBodyGridPositions;
	
	UPROPERTY(BlueprintReadOnly, Category = Input)
	TArray<FIntPoint> CurrentGridPosition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* IA_Up;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* IA_Down;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* IA_Left;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* IA_Right;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MoveSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float TurnSpeed = 45.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	float MoveInput = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	float TurnInput = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDead = false;
	
	UPROPERTY(BlueprintReadOnly)
	int32 PendingGrowth = 0;
private:
	ESnakeDirection CurrentDirection = ESnakeDirection::Right;
	ESnakeDirection RequestedDirection = ESnakeDirection::Right;
	
	void Input_TryTurnUp(const FInputActionValue& Value);
	void Input_TryTurnDown(const FInputActionValue& Value);
	void Input_TryTurnLeft(const FInputActionValue& Value);
	void Input_TryTurnRight(const FInputActionValue& Value);

	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	TArray<FIntPoint> GetAllOccupiedGridCells() const;
};
