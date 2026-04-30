#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"

class UInstancedStaticMeshComponent;
class URectLightComponent;

UCLASS()
class SNAKE_API AGridManager : public AActor
{
	GENERATED_BODY()

public:	
	AGridManager();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layer Visual")
	TArray<TObjectPtr<UMaterialInterface>> LayerMaterials;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grid|Layer Visual")
	TObjectPtr<UMaterialInterface> DefaultFloorMaterial;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> FloorLayerInstances;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> WallLayerInstances;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UInstancedStaticMeshComponent* FloorInstances;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UInstancedStaticMeshComponent* WallInstances;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grid|Lighting")
	TObjectPtr<URectLightComponent> GridRectLight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grid|Lighting")
	float RectLightHeight = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grid|Lighting")
	float RectLightIntensity = 500.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	int32 GridWidth = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	int32 GridHeight = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	int32 GridDepth = 1;
	
	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	FIntVector GridDimensions = FIntVector(16, 16, 1);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	FVector GridOrigin = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	int32 CellSize = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid|Visual")
	bool bGenerateFloors = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Visual")
	float WallZOffset = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Visual")
	float FloorZOffset = -0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Visual")
	FVector InstanceScale = FVector(1.0f, 1.0f, 1.0f);
	
	TSet<FIntVector> BlockedCells;
	
	UMaterialInterface* GetLayerMaterial(int32 Layer) const;
	
	FVector GetCellWorldPosition(const FIntVector& GridCell) const;
	
	bool IsCellValid(const FIntVector& GridCell) const;
	
	bool IsBlocked(const FIntVector& GridCell) const;
	
	bool TryGetRandomFreeCell(FIntVector& OutCell, const TArray<FIntVector>& ForbiddenCells, int32 MaxAttempts = 200) const;
	
	void GenerateCells();
	void GenerateBlockedCells();
	void GenerateVisualInstances();
	void ClearVisualInstances();
	void UpdateGridLighting();
	
	void EnsureLayerInstanceComponents();
	void ApplyLayerMaterials();

protected:
	virtual void BeginPlay() override;
};