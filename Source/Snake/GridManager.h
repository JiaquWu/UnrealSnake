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
	UInstancedStaticMeshComponent* GridVisuals;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* CellMeshAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInterface> CellMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridWidth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridHeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CellSize;
	
	FVector GetCellWorldPosition(int32 CellX, int32 CellY) const;
	
	bool IsCellValid(int32 CellX, int32 CellY) const;
	
	void GenerateCells();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	// virtual void Tick(float DeltaTime) override;

};
