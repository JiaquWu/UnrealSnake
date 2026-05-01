#include "SnakeAIComponent.h"

#include "FoodActor.h"
#include "SnakeGameMode.h"
#include "Kismet/GameplayStatics.h"

USnakeAIComponent::USnakeAIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USnakeAIComponent::BeginPlay()
{
	Super::BeginPlay();

	SnakeGameMode = Cast<ASnakeGameMode>(UGameplayStatics::GetGameMode(this));
}

void USnakeAIComponent::Init(AABoxRoverPawn* InControlledSnake)
{
	ControlledSnake = InControlledSnake;

	if (ControlledSnake)
	{
		ControlledSnake->OnBeforeMoveStep.AddDynamic(this, &USnakeAIComponent::DecideNextDirection);
	}
}

void USnakeAIComponent::DecideNextDirection()
{
	if (!ControlledSnake)
	{
		return;
	}

	switch (AIMode)
	{
	case ESnakeAIMode::GreedySafe:
	{
		AFoodActor* TargetFood = FindClosestFood();
		if (!TargetFood)
		{
			return;
		}

		const ESnakeDirection Direction =
			ChooseGreedySafeDirection(TargetFood->GetFoodGridPosition());

		ControlledSnake->RequestDirection(Direction);
		break;
	}

	case ESnakeAIMode::RandomSafe:
	{
		const ESnakeDirection Direction = ChooseRandomSafeDirection();
		ControlledSnake->RequestDirection(Direction);
		break;
	}
	}
}

AFoodActor* USnakeAIComponent::FindClosestFood() const
{
	if (!SnakeGameMode || !ControlledSnake)
	{
		return nullptr;
	}

	AFoodActor* ClosestFood = nullptr;
	int32 BestDistance = MAX_int32;

	const FIntVector SnakeCell = ControlledSnake->GetCurrentGridPosition();

	for (AFoodActor* Food : SnakeGameMode->GetSpawnedFoodActors())
	{
		if (!Food || !Food->IsFoodActive())
		{
			continue;
		}

		const int32 Distance = ManhattanDistance(SnakeCell, Food->GetFoodGridPosition());

		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			ClosestFood = Food;
		}
	}

	return ClosestFood;
}

ESnakeDirection USnakeAIComponent::ChooseGreedySafeDirection(const FIntVector& TargetCell) const
{
	if (!ControlledSnake)
	{
		return ESnakeDirection::Right;
	}

	const FIntVector CurrentCell = ControlledSnake->GetCurrentGridPosition();
	const ESnakeDirection CurrentDirection = ControlledSnake->GetCurrentDirection();

	TArray<ESnakeDirection> BestDirections;
	int32 BestScore = MAX_int32;

	for (ESnakeDirection Direction : GetAllDirections())
	{
		if (IsOpposite(Direction, CurrentDirection))
		{
			continue;
		}

		const FIntVector NextCell = CurrentCell + DirectionToOffset(Direction);

		if (!ControlledSnake->CanMoveToCell(NextCell))
		{
			continue;
		}

		int32 Score = ManhattanDistance(NextCell, TargetCell);

		if (!HasEscapeMoveFrom(NextCell))
		{
			Score += 1000;
		}

		if (Score < BestScore)
		{
			BestScore = Score;
			BestDirections.Empty();
			BestDirections.Add(Direction);
		}
		else if (Score == BestScore)
		{
			BestDirections.Add(Direction);
		}
	}

	if (BestDirections.Num() > 0)
	{
		return BestDirections[FMath::RandRange(0, BestDirections.Num() - 1)];
	}

	return ChooseRandomSafeDirection();
}

ESnakeDirection USnakeAIComponent::ChooseRandomSafeDirection() const
{
	if (!ControlledSnake)
	{
		return ESnakeDirection::Right;
	}

	const FIntVector CurrentCell = ControlledSnake->GetCurrentGridPosition();
	const ESnakeDirection CurrentDirection = ControlledSnake->GetCurrentDirection();

	TArray<ESnakeDirection> SafeDirections;

	for (ESnakeDirection Direction : GetAllDirections())
	{
		if (IsOpposite(Direction, CurrentDirection))
		{
			continue;
		}

		const FIntVector NextCell = CurrentCell + DirectionToOffset(Direction);

		if (ControlledSnake->CanMoveToCell(NextCell))
		{
			SafeDirections.Add(Direction);
		}
	}

	if (SafeDirections.Num() > 0)
	{
		return SafeDirections[FMath::RandRange(0, SafeDirections.Num() - 1)];
	}

	return CurrentDirection;
}

bool USnakeAIComponent::HasEscapeMoveFrom(const FIntVector& Cell) const
{
	if (!ControlledSnake)
	{
		return false;
	}

	const ESnakeDirection CurrentDirection = ControlledSnake->GetCurrentDirection();
	int32 SafeMoveCount = 0;

	for (ESnakeDirection Direction : GetAllDirections())
	{
		if (IsOpposite(Direction, CurrentDirection))
		{
			continue;
		}

		const FIntVector CandidateCell = Cell + DirectionToOffset(Direction);

		if (ControlledSnake->CanMoveToCell(CandidateCell))
		{
			SafeMoveCount++;
		}
	}

	return SafeMoveCount > 0;
}

TArray<ESnakeDirection> USnakeAIComponent::GetAllDirections() const
{
	return {
		ESnakeDirection::Up,
		ESnakeDirection::Down,
		ESnakeDirection::Left,
		ESnakeDirection::Right,
		ESnakeDirection::VerticalUp,
		ESnakeDirection::VerticalDown
	};
}

bool USnakeAIComponent::IsOpposite(ESnakeDirection A, ESnakeDirection B) const
{
	return (A == ESnakeDirection::Up && B == ESnakeDirection::Down)
		|| (A == ESnakeDirection::Down && B == ESnakeDirection::Up)
		|| (A == ESnakeDirection::Left && B == ESnakeDirection::Right)
		|| (A == ESnakeDirection::Right && B == ESnakeDirection::Left)
		|| (A == ESnakeDirection::VerticalUp && B == ESnakeDirection::VerticalDown)
		|| (A == ESnakeDirection::VerticalDown && B == ESnakeDirection::VerticalUp);
}

FIntVector USnakeAIComponent::DirectionToOffset(ESnakeDirection Direction) const
{
	switch (Direction)
	{
	case ESnakeDirection::Up:
		return FIntVector(1, 0, 0);

	case ESnakeDirection::Down:
		return FIntVector(-1, 0, 0);

	case ESnakeDirection::Left:
		return FIntVector(0, -1, 0);

	case ESnakeDirection::Right:
		return FIntVector(0, 1, 0);

	case ESnakeDirection::VerticalUp:
		return FIntVector(0, 0, 1);

	case ESnakeDirection::VerticalDown:
		return FIntVector(0, 0, -1);

	default:
		return FIntVector::ZeroValue;
	}
}

int32 USnakeAIComponent::ManhattanDistance(const FIntVector& A, const FIntVector& B) const
{
	return FMath::Abs(A.X - B.X)
		+ FMath::Abs(A.Y - B.Y)
		+ FMath::Abs(A.Z - B.Z);
}