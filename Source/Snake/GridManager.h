// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"


class UInstancedStaticMeshComponent;

UCLASS()
class SNAKE_API AGridManager : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AGridManager();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UInstancedStaticMeshComponent* FloorInstances;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UInstancedStaticMeshComponent* WallInstances;
	
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// UStaticMesh* CellMeshAsset;
	
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// TObjectPtr<UMaterialInterface> CellMaterial;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridWidth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridHeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CellSize;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGenerateFloors = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Visual")
	float WallZOffset = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Visual")
	float FloorZOffset = -0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Visual")
	FVector InstanceScale = FVector(1.0f, 1.0f, 1.0f);
	
	TSet<FIntPoint> BlockedCells;
	
	FVector GetCellWorldPosition(const FIntPoint& GridCell) const;
	
	bool IsCellValid(const FIntPoint& GridCell) const;
	
	bool IsBlocked(const FIntPoint& GridCell) const;
	
	bool TryGetRandomFreeCell(FIntPoint& OutCell, const TArray<FIntPoint>& ForbiddenCells, int32 MaxAttempts = 200) const;
	
	
	
	
	
	void GenerateCells();
	
	void GenerateBlockedCells();
	
	void GenerateVisualInstances();
	
	void ClearVisualInstances();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	// virtual void Tick(float DeltaTime) override;

};
