#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Snake/ABoxRoverPawn.h"
#include "SnakeAIComponent.generated.h"

class ASnakeGameMode;
class AFoodActor;

UENUM(BlueprintType)
enum class ESnakeAIMode : uint8
{
	GreedySafe,
	RandomSafe
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SNAKE_API USnakeAIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USnakeAIComponent();

	void Init(AABoxRoverPawn* InControlledSnake);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category="Snake|AI")
	ESnakeAIMode AIMode = ESnakeAIMode::GreedySafe;

	UPROPERTY()
	TObjectPtr<AABoxRoverPawn> ControlledSnake;

	UPROPERTY()
	TObjectPtr<ASnakeGameMode> SnakeGameMode;

	UFUNCTION()
	void DecideNextDirection();

	AFoodActor* FindClosestFood() const;
	ESnakeDirection ChooseGreedySafeDirection(const FIntVector& TargetCell) const;
	ESnakeDirection ChooseRandomSafeDirection() const;

	TArray<ESnakeDirection> GetAllDirections() const;
	bool IsOpposite(ESnakeDirection A, ESnakeDirection B) const;
	FIntVector DirectionToOffset(ESnakeDirection Direction) const;
	int32 ManhattanDistance(const FIntVector& A, const FIntVector& B) const;
	bool HasEscapeMoveFrom(const FIntVector& Cell) const;
};