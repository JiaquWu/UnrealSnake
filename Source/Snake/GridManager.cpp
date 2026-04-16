// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Input/HittestGrid.h"

// Sets default values
AGridManager::AGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	
	FloorInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorInstances"));
	FloorInstances->SetupAttachment(RootComponent);
	FloorInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	WallInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallInstances"));
	WallInstances->SetupAttachment(RootComponent);
	WallInstances->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	
}

// Called when the game starts or when spawned
void AGridManager::BeginPlay()
{
	Super::BeginPlay();
	GenerateCells();
	GenerateVisualInstances();
}

// Called every frame
// void AGridManager::Tick(float DeltaTime)
// {
// 	Super::Tick(DeltaTime);
//
// }

FVector AGridManager::GetCellWorldPosition(const FIntPoint& GridCell) const
{
	const FVector OriginPosition = GetActorLocation();
	const float X = GridCell.X * CellSize + OriginPosition.X + CellSize * 0.5f;
	const float Y = GridCell.Y * CellSize + OriginPosition.Y + CellSize * 0.5f;
	
	FVector Result = FVector(X, Y, OriginPosition.Z);
	return Result;

}

bool AGridManager::IsCellValid(const FIntPoint& GridCell) const
{
	return GridCell.X >= 0
	&& GridCell.X < GridWidth 
	&&  GridCell.Y >= 0
	&& GridCell.Y < GridHeight;
	
}

bool AGridManager::IsBlocked(const FIntPoint& GridCell) const
{
	return BlockedCells.Contains(GridCell);
}

bool AGridManager::TryGetRandomFreeCell(FIntPoint& OutCell, const TArray<FIntPoint>& ForbiddenCells, int32 MaxAttempts) const
{
	TSet<FIntPoint> ForbiddenSet(ForbiddenCells);
	for (int32 i = 0; i < MaxAttempts; i++)
	{
		const FIntPoint CandidateCell
		(FMath::RandRange(1, GridWidth-2), 
		FMath::RandRange(1, GridHeight-2));
		
		if (ForbiddenSet.Contains(CandidateCell) || ForbiddenCells.Contains(CandidateCell))
		{
			continue;
		}
		
		OutCell = CandidateCell;
		return true;
	}
	
	return false;
}

void AGridManager::GenerateCells()
{
	// if (FloorInstances == nullptr || CellMeshAsset == nullptr)
	// 	return;
	//FloorInstances->SetStaticMesh(CellMeshAsset);
	//FloorInstances->ClearInstances();
	
	// if (CellMaterial)
	// {
	// 	FloorInstances->SetMaterial(0, CellMaterial);
	// }
	
	for (int x = 0; x < GridWidth; x++)
	{
		BlockedCells.Add(FIntPoint(x,0));
		BlockedCells.Add(FIntPoint(x,GridHeight-1));
	}
	for (int y = 0; y < GridHeight; y++)
	{
		BlockedCells.Add(FIntPoint(0,y));
		BlockedCells.Add(FIntPoint(GridWidth-1,y));
	}
	
	
	
	
	
	
}

void AGridManager::GenerateVisualInstances()
{
	ClearVisualInstances();

	if (!FloorInstances || !WallInstances)
		return;

	int32 FloorCount = 0;
	int32 WallCount = 0;

	for (int32 CellX = 0; CellX < GridWidth; CellX++)
	{
		for (int32 CellY = 0; CellY < GridHeight; CellY++)
		{
			const FIntPoint Cell(CellX, CellY);
			const bool bBlocked = BlockedCells.Contains(Cell);
			const FVector BaseWorld = GetCellWorldPosition(Cell);

			if (bBlocked)
			{
				const FVector WallLocation = BaseWorld + FVector(0, 0, WallZOffset);
				const FTransform WallTransform(FRotator::ZeroRotator, WallLocation, InstanceScale);
				WallInstances->AddInstance(WallTransform);
				WallCount++;
			}
			else if (bGenerateFloors)
			{
				const FVector FloorLocation = BaseWorld + FVector(0, 0, FloorZOffset);
				const FTransform FloorTransform(FRotator::ZeroRotator, FloorLocation, InstanceScale);
				FloorInstances->AddInstance(FloorTransform);
				FloorCount++;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Generated floors: %d, walls: %d, blocked cells: %d"),
		FloorCount,
		WallCount,
		BlockedCells.Num());
}

void AGridManager::ClearVisualInstances()
{
	if (FloorInstances)
	{
		FloorInstances->ClearInstances();
	}
	
	if (WallInstances)
	{
		WallInstances->ClearInstances();
	}
	
}
