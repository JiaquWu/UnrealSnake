// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridManager.h"
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
	void HandleSnakeDeath();
	
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
	void TickGridMovement(float DeltaTime);
	void UpdateDirection(ESnakeDirection NewDirection);
	void HandleDirectionChange();
	bool IsValidTurn(ESnakeDirection NewDirection) const;
	
	bool WouldHitWall(const FIntPoint& NextCell) const;
	bool WouldHitSelf(const FIntPoint& NextCell) const;
	
	// Body system
	void StartNewMoveStep();
	void FinishMoveStep();
	void UpdateBodyVisuals(float Alpha); // Alpha is 0 to 1 representing progress from current grid cell to next grid cell
	void EnsureBodySegmentMeshCount(); // Need a way to make sure we have the right number of body segment meshes to match the body segments in CurrentBodyGridPositions. This function adds or removes meshes as needed.
	void ClearBodyVisuals();
	void AddInitialBodySegments(int32 NumSegments);
	void GrowSnake(int32 Amount = 1);
	
	
	FIntPoint GetClampedStartGridPosition() const;
	FVector GetVectorFromDirection(ESnakeDirection Direction) const;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (AllowPrivateAccess = "true"))
	FIntPoint StartGridPosition = FIntPoint(10, 10);
	
	UPROPERTY(BlueprintReadOnly, Category = Input)
	TArray<FIntPoint> CurrentBodyGridPositions;
	
	UPROPERTY(BlueprintReadOnly, Category = Input)
	FIntPoint CurrentGridPosition;
	
	FIntPoint PendingNextGridPosition = FIntPoint(0, 0);
	
	FVector StepStartWorldLocation = FVector::ZeroVector;
	FVector StepTargetWorldLocation = FVector::ZeroVector;

	// Snapshot of body before starting a step. So body segements can smoothly follow.
	TArray<FIntPoint> PreviousBodyGridPositions;
	TArray<FIntPoint> TargetBodyGridPositions;
	
	float MoveInterpolationProgress = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float MoveStepTime = 0.2f; // Time it takes to move from one grid cell to the next when using grid movement.
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> BodySegmentMeshes;
	
	// Body visuals settings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> BodySegmentMeshAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> BodySegmentMaterial = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake", meta = (AllowPrivateAccess = "true"))
	int32 InitialBodyLength = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body", meta = (AllowPrivateAccess = "true"))
	FVector BodySegmentMeshScale = FVector(0.9f, 0.9f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Snake|Body", meta = (AllowPrivateAccess = "true"))
	float BodySegmentMeshZOffset = 0.f; // How much to offset the body segment meshes on the Z axis, to prevent z-fighting with the head mesh and ground.
	
	
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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bUseGridMovement = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsDead = false;
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsMovingToTarget = false;
	
	FTimerHandle ResetTimerHandle;
	
	UPROPERTY(BlueprintReadOnly)
	int32 PendingGrowth = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	FIntPoint GridDimensions;
private:
	ESnakeDirection CurrentDirection = ESnakeDirection::Right;
	ESnakeDirection RequestedDirection = ESnakeDirection::Right;
	
	TObjectPtr<AGridManager> GridManager = nullptr;
	
	float CellSize = 100.f;
	
	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (AllowPrivateAccess = "true"))
	FVector GridOrigin = FVector::ZeroVector;
	
	void CacheGridManager();
	
	void Input_TryTurnUp(const FInputActionValue& Value);
	void Input_TryTurnDown(const FInputActionValue& Value);
	void Input_TryTurnLeft(const FInputActionValue& Value);
	void Input_TryTurnRight(const FInputActionValue& Value);

	FVector GridToWorldLocation(const FIntPoint& GridPosition) const;
	
	FIntPoint DirectionToGridOffset(ESnakeDirection CurrentDirection) const;
	
	
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	TArray<FIntPoint> GetAllOccupiedGridCells() const;
};
