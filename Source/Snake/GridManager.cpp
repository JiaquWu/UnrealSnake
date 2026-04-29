#include "GridManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/RectLightComponent.h"

AGridManager::AGridManager()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	
	FloorInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorInstances"));
	FloorInstances->SetupAttachment(RootComponent);
	FloorInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	WallInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallInstances"));
	WallInstances->SetupAttachment(RootComponent);
	WallInstances->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	GridRectLight = CreateDefaultSubobject<URectLightComponent>(TEXT("GridRectLight"));
	GridRectLight->SetupAttachment(RootComponent);
	GridRectLight->SetIntensity(RectLightIntensity);
	GridRectLight->SetCastShadows(false);
}

void AGridManager::BeginPlay()
{
	Super::BeginPlay();

	GridDimensions = FIntVector(GridWidth, GridHeight, GridDepth);

	GenerateCells();
	GenerateVisualInstances();
	UpdateGridLighting();
}

FVector AGridManager::GetCellWorldPosition(const FIntVector& GridCell) const
{
	const FVector OriginPosition = GetActorLocation();

	const float X = GridCell.X * CellSize + OriginPosition.X + CellSize * 0.5f;
	const float Y = GridCell.Y * CellSize + OriginPosition.Y + CellSize * 0.5f;

	const float Z = GridCell.Z * CellSize + OriginPosition.Z;
	
	return FVector(X, Y, Z);
}

bool AGridManager::IsCellValid(const FIntVector& GridCell) const
{
	return GridCell.X >= 0
		&& GridCell.X < GridWidth
		&& GridCell.Y >= 0
		&& GridCell.Y < GridHeight
		&& GridCell.Z >= 0
		&& GridCell.Z < GridDepth;
}

bool AGridManager::IsBlocked(const FIntVector& GridCell) const
{
	return BlockedCells.Contains(GridCell);
}

bool AGridManager::TryGetRandomFreeCell(FIntVector& OutCell, const TArray<FIntVector>& ForbiddenCells, int32 MaxAttempts) const
{
	TSet<FIntVector> ForbiddenSet(ForbiddenCells);

	for (int32 i = 0; i < MaxAttempts; i++)
	{
		const FIntVector CandidateCell(
			FMath::RandRange(1, GridWidth - 2),
			FMath::RandRange(1, GridHeight - 2),
			FMath::RandRange(0, GridDepth - 1)
		);
		
		if (!IsCellValid(CandidateCell))
		{
			continue;
		}

		if (IsBlocked(CandidateCell))
		{
			continue;
		}
		
		if (ForbiddenSet.Contains(CandidateCell))
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
	BlockedCells.Empty();

	GridDimensions = FIntVector(GridWidth, GridHeight, GridDepth);

	GenerateBlockedCells();
}

void AGridManager::GenerateBlockedCells()
{
	for (int32 Z = 0; Z < GridDepth; Z++)
	{
		for (int32 X = 0; X < GridWidth; X++)
		{
			BlockedCells.Add(FIntVector(X, 0, Z));
			BlockedCells.Add(FIntVector(X, GridHeight - 1, Z));
		}

		for (int32 Y = 0; Y < GridHeight; Y++)
		{
			BlockedCells.Add(FIntVector(0, Y, Z));
			BlockedCells.Add(FIntVector(GridWidth - 1, Y, Z));
		}
	}
}

void AGridManager::GenerateVisualInstances()
{
	ClearVisualInstances();

	if (!FloorInstances || !WallInstances)
	{
		return;
	}

	int32 FloorCount = 0;
	int32 WallCount = 0;

	for (int32 CellZ = 0; CellZ < GridDepth; CellZ++)
	{
		for (int32 CellX = 0; CellX < GridWidth; CellX++)
		{
			for (int32 CellY = 0; CellY < GridHeight; CellY++)
			{
				const FIntVector Cell(CellX, CellY, CellZ);
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
	}

	UE_LOG(LogTemp, Warning, TEXT("Generated floors: %d, walls: %d, blocked cells: %d, depth: %d"),
		FloorCount,
		WallCount,
		BlockedCells.Num(),
		GridDepth);
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

void AGridManager::UpdateGridLighting()
{
	if (!GridRectLight)
	{
		return;
	}

	const float GridWorldWidth = GridWidth * CellSize;
	const float GridWorldHeight = GridHeight * CellSize;
	const float GridWorldDepth = FMath::Max(1, GridDepth) * CellSize;

	const FVector GridCenterLocal(
		GridWorldWidth * 0.5f,
		GridWorldHeight * 0.5f,
		GridWorldDepth + RectLightHeight
	);

	GridRectLight->SetRelativeLocation(GridCenterLocal);
	GridRectLight->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	GridRectLight->SetSourceWidth(GridWorldWidth);
	GridRectLight->SetSourceHeight(GridWorldHeight);

	GridRectLight->SetAttenuationRadius(FMath::Max3(GridWorldWidth, GridWorldHeight, GridWorldDepth) * 1.5f);
	GridRectLight->SetIntensity(RectLightIntensity);
	GridRectLight->SetCastShadows(false);
}