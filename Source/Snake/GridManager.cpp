// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Input/HittestGrid.h"

// Sets default values
AGridManager::AGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GridVisuals = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Visual Mesh"));
	
}

// Called when the game starts or when spawned
void AGridManager::BeginPlay()
{
	Super::BeginPlay();
	GenerateCells();
}

// Called every frame
// void AGridManager::Tick(float DeltaTime)
// {
// 	Super::Tick(DeltaTime);
//
// }

FVector AGridManager::GetCellWorldPosition(int32 CellX, int32 CellY) const
{
	const FVector OriginPosition = GetActorLocation();
	const float X = CellX * CellSize + OriginPosition.X + CellSize * 0.5f;
	const float Y = CellY * CellSize + OriginPosition.Y + CellSize * 0.5f;
	
	FVector Result = FVector(X, Y, OriginPosition.Z);
	return Result;

}

bool AGridManager::IsCellValid(int32 CellX, int32 CellY) const
{
	return CellX >= 0
	&& CellX < GridWidth 
	&&  CellY >= 0
	&& CellY < GridHeight;
	
}

void AGridManager::GenerateCells()
{
	if (GridVisuals == nullptr || CellMeshAsset == nullptr)
		return;
	GridVisuals->SetStaticMesh(CellMeshAsset);
	GridVisuals->ClearInstances();
	
	if (CellMaterial)
	{
		GridVisuals->SetMaterial(0, CellMaterial);
	}
	
	for (int32 CellX = 0; CellX < GridWidth; CellX++)
	{
		for (int32 CellY = 0; CellY < GridHeight; CellY++)
		{
			const FVector CellPosition = GetCellWorldPosition(CellX, CellY);
			const FTransform InstanceTransform(FRotator::ZeroRotator, CellPosition, FVector(1.f));
			
			
			GridVisuals->AddInstance(InstanceTransform);
		}
	}
	
	
}