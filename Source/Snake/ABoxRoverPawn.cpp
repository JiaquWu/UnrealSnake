// Fill out your copyright notice in the Description page of Project Settings.


#include "ABoxRoverPawn.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Private/FoodActor.h"


#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet//GameplayStatics.h"


// Sets default values
AABoxRoverPawn::AABoxRoverPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = CollisionSphere;
	CollisionSphere->SetSphereRadius(32.f);
	CollisionSphere->SetCollisionProfileName(TEXT("Pawn")); // Set the collision profile to "Pawn", which is a predefined profile that allows it to collide with the world and other pawns, but not block the camera or other things. This is important for our snake pawn, because we want it to be able to collide with the ground and other objects, but we don't want it to block the camera or cause weird physics interactions with the box mesh.

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual Mesh"));
	VisualMesh->SetupAttachment(RootComponent);

	// SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	// SpringArm->SetupAttachment(RootComponent);
	// VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SnakeHeadMesh"));
	// VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetRelativeLocation(FVector::ZeroVector); // Center the mesh on the root component (collision sphere)
	// We don't want the mesh to collide with the ground or other objects, because we're going to use a separate collision sphere for that, and we want the mesh to just be a visual representation of the snake's head without affecting physics or collisions. So we set it to NoCollision, which means it won't generate any collision events or block anything. We also set SimulatePhysics to false, because we don't want the mesh to be affected by physics forces or gravity, since we're going to move it manually in the Tick function based on player input:
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetSimulatePhysics(false);

	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 800.f;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->SetAbsolute(false, true, false);   // 位置不绝对，旋转绝对，缩放不绝对
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;
	SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	SpringArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	
	bUseControllerRotationYaw = false;
}

// Called when the game starts or when spawned
void AABoxRoverPawn::BeginPlay()
{
	Super::BeginPlay();
	
	if (!GridManager)
	{
		CacheGridManager();
		if (!GridManager)
		{
			UE_LOG(LogTemp, Error, TEXT("SnakePawn could not find GridManager in world."))
		}
		else
		{
			GridDimensions = GridManager->GridDimensions;
			CellSize = GridManager->CellSize;
		}
	}
	
	RequestedDirection = CurrentDirection;
	UpdateDirection(CurrentDirection);	
	
	

	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		UE_LOG(LogTemp, Warning, TEXT("FirstPlayerController valid"));

		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			UE_LOG(LogTemp, Warning, TEXT("LocalPlayer valid"));

			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				UE_LOG(LogTemp, Warning, TEXT("EnhancedInput subsystem valid"));

				if (DefaultMappingContext)
				{
					UE_LOG(LogTemp, Warning, TEXT("Adding Mapping Context"));
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("DefaultMappingContext is NULL"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Subsystem is NULL"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("LocalPlayer is NULL"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FirstPlayerController is NULL"));
	}
	
	if (Camera)
	{
		UE_LOG(LogTemp, Warning, TEXT("Camera bUsePawnControlRotation: %s"),
			Camera->bUsePawnControlRotation ? TEXT("true") : TEXT("false"));
	}

	if (SpringArm)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpringArm bUsePawnControlRotation: %s"),
			SpringArm->bUsePawnControlRotation ? TEXT("true") : TEXT("false"));
	}
	
	if (bUseGridMovement)
	{
		CurrentGridPosition = GetClampedStartGridPosition();
		PendingNextGridPosition = CurrentGridPosition;
		
		const FVector StartLocation = GridToWorldLocation(CurrentGridPosition);
		//SetActorLocation(StartLocation);

		StepStartWorldLocation = StartLocation;
		StepTargetWorldLocation = StartLocation;
		MoveInterpolationProgress = 0.f;
		bIsMovingToTarget = false;

		CurrentBodyGridPositions.Empty();
		PreviousBodyGridPositions.Empty();
		AddInitialBodySegments(InitialBodyLength);
		EnsureBodySegmentMeshCount();
		UpdateBodyVisuals(0.f);
	}
	else
	{
		FVector StartLocation = GetActorLocation();
		StartLocation.Z += 15.f;
		SetActorLocation(StartLocation);
	}
	
	
}


// Called every frame
void AABoxRoverPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// if (AController* CurrentController  = GetController())
	// {
	// 	GEngine->AddOnScreenDebugMessage(
	// 		3, 0.0f, FColor::Cyan,
	// 		FString::Printf(TEXT("Controller: %s"), *CurrentController ->GetName())
	// 	);
	// }
	// else
	// {
	// 	GEngine->AddOnScreenDebugMessage(
	// 		3, 0.0f, FColor::Red,
	// 		TEXT("Controller: NULL")
	// 	);
	// }
	if (!FMath::IsNearlyZero(MoveInput))
	{
		const FVector MoveDelta = GetActorForwardVector() * MoveInput * MoveSpeed * DeltaTime;
		AddActorWorldOffset(MoveDelta, true);
	}

	if (!FMath::IsNearlyZero(TurnInput))
	{
		const FRotator TurnDelta(0.0f, TurnInput * TurnSpeed * DeltaTime, 0.0f);
		AddActorLocalRotation(TurnDelta);
	}
	
	if (bUseGridMovement)
	{
		// Interpolate towards the target
		TickGridMovement(DeltaTime);
	}
	else
	{
		HandleDirectionChange();
		TickFreeMovement(DeltaTime);
	}
	// HandleDirectionChange();
	// TickFreeMovement(DeltaTime);
	
	// if (GEngine)
	// {
	// 	TArray<UCameraComponent*> Cameras;
	// 	GetComponents<UCameraComponent>(Cameras);
	//
	// 	for (int32 i = 0; i < Cameras.Num(); ++i)
	// 	{
	// 		if (Cameras[i])
	// 		{
	// 			UE_LOG(LogTemp, Warning, TEXT("Camera %d: %s Active=%s WorldRot=%s"),
	// 				i,
	// 				*Cameras[i]->GetName(),
	// 				Cameras[i]->IsActive() ? TEXT("true") : TEXT("false"),
	// 				*Cameras[i]->GetComponentRotation().ToString());
	// 		}
	// 	}
	// 	
	// 	
	// 	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	// 	{
	// 		if (AActor* ViewTarget = PC->GetViewTarget())
	// 		{
	// 			GEngine->AddOnScreenDebugMessage(
	// 				40, 0.f, FColor::White,
	// 				FString::Printf(TEXT("ViewTarget: %s"), *ViewTarget->GetName())
	// 			);
	// 		}
	// 	}
	// 	
	// 	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	// 	{
	// 		GEngine->AddOnScreenDebugMessage(
	// 			20, 0.f, FColor::White,
	// 			FString::Printf(TEXT("Control Rot: %s"), *PC->GetControlRotation().ToString())
	// 		);
	//
	// 		if (PC->PlayerCameraManager)
	// 		{
	// 			GEngine->AddOnScreenDebugMessage(
	// 				21, 0.f, FColor::Silver,
	// 				FString::Printf(TEXT("CameraManager Rot: %s"), *PC->PlayerCameraManager->GetCameraRotation().ToString())
	// 			);
	// 		}
	// 		
	// 		if (SpringArm)
	// 		{
	// 			const FTransform SocketTransform = SpringArm->GetSocketTransform(USpringArmComponent::SocketName, RTS_World);
	//
	// 			GEngine->AddOnScreenDebugMessage(
	// 				30, 0.f, FColor::Magenta,
	// 				FString::Printf(TEXT("SpringArm Socket Rot: %s"), *SocketTransform.GetRotation().Rotator().ToString())
	// 			);
	// 		}
	// 	}
		
		// GEngine->AddOnScreenDebugMessage(
		// 	10, 0.f, FColor::Green,
		// 	FString::Printf(TEXT("SpringArm World Rot: %s"), *SpringArm->GetComponentRotation().ToString())
		// );
		//
		// GEngine->AddOnScreenDebugMessage(
		// 	11, 0.f, FColor::Yellow,
		// 	FString::Printf(TEXT("SpringArm Relative Rot: %s"), *SpringArm->GetRelativeRotation().ToString())
		// );
		//
		// GEngine->AddOnScreenDebugMessage(
		// 	12, 0.f, FColor::Cyan,
		// 	FString::Printf(TEXT("Camera World Rot: %s"), *Camera->GetComponentRotation().ToString())
		// );
		//
		// GEngine->AddOnScreenDebugMessage(
		// 	13, 0.f, FColor::Red,
		// 	FString::Printf(TEXT("Camera Relative Rot: %s"), *Camera->GetRelativeRotation().ToString())
		// );
		//
		// if (APlayerController* PC = Cast<APlayerController>(GetController()))
		// {
		// 	GEngine->AddOnScreenDebugMessage(
		// 		20, 0.f, FColor::White,
		// 		FString::Printf(TEXT("Control Rot: %s"), *PC->GetControlRotation().ToString())
		// 	);
		// }
	//}
	// 屏幕 debug
	// if (GEngine)
	// {
	// 	GEngine->AddOnScreenDebugMessage(
	// 		1,
	// 		0.0f,
	// 		FColor::Green,
	// 		FString::Printf(TEXT("MoveInput: %.2f"), MoveInput)
	// 	);
	//
	// 	GEngine->AddOnScreenDebugMessage(
	// 		2,
	// 		0.0f,
	// 		FColor::Yellow,
	// 		FString::Printf(TEXT("TurnInput: %.2f"), TurnInput)
	// 	); 
	// }
}

void AABoxRoverPawn::UpdateDirection(ESnakeDirection NewDirection)
{
	switch (NewDirection)
	{
	case ESnakeDirection::Up:	SetActorRotation(FRotator(0.f, 0.f, 0.f));
		break;
	case ESnakeDirection::Down:	SetActorRotation(FRotator(0.f, 180.f, 0.f));
		break;
	case ESnakeDirection::Left:	SetActorRotation(FRotator(0.f, -90.f, 0.f));
		break;
	case ESnakeDirection::Right:SetActorRotation(FRotator(0.f, 90.f, 0.f));
		break;
	}
}

void AABoxRoverPawn::HandleDirectionChange()
{
	if (CurrentDirection != RequestedDirection && IsValidTurn(RequestedDirection))
	{
		CurrentDirection = RequestedDirection;
		UpdateDirection(CurrentDirection);
		UE_LOG(LogTemp, Warning, TEXT("Direction changed to: %s"), *UEnum::GetValueAsString(CurrentDirection));
	}
}

bool AABoxRoverPawn::IsValidTurn(ESnakeDirection NewDirection) const
{
	// A turn is valid if it's not the same as the current direction, and it's not directly opposite to the current direction (e.g. if we're moving up, we can't turn down)
	if (NewDirection == CurrentDirection)
	{
		return false;
	}
	switch (CurrentDirection)
	{
	case ESnakeDirection::Up:
		return NewDirection != ESnakeDirection::Down;
	case ESnakeDirection::Down:
		return NewDirection != ESnakeDirection::Up;
	case ESnakeDirection::Left:
		return NewDirection != ESnakeDirection::Right;
	case ESnakeDirection::Right:
		return NewDirection != ESnakeDirection::Left;
	default:
		return false; // Should never happen, but we return false just in case
	}
}

FVector AABoxRoverPawn::GetVectorFromDirection(ESnakeDirection Direction) const
{
	switch (Direction)
	{
	case ESnakeDirection::Up:     return FVector::ForwardVector;  // +X
	case ESnakeDirection::Down:   return -FVector::ForwardVector; // -X
	case ESnakeDirection::Left:   return -FVector::RightVector;   // -Y
	case ESnakeDirection::Right:  return FVector::RightVector;    // +Y
	default:                    return FVector::ZeroVector;     // Should never happen, but we return zero just in case
	}
}

void AABoxRoverPawn::TickFreeMovement(float DeltaTime)
{
	// Is this going to work? 
	FVector MovementVector = GetVectorFromDirection(CurrentDirection);
	FVector DesiredOffset = MovementVector * MoveSpeed * DeltaTime;
	AddActorWorldOffset(DesiredOffset, true);
}

void AABoxRoverPawn::TickGridMovement(float DeltaTime)
{
	if (MoveStepTime <= 0.f)
	{
		return;
	}

	if (!bIsMovingToTarget)
	{
		// If at target, check for direction change and update target location
		StartNewMoveStep();
		if (!bIsMovingToTarget)
		{
			return; // If we failed to start a new move step (e.g. because we hit a wall), we exit early and don't try to interpolate or update the location
		}
	}

	MoveInterpolationProgress += DeltaTime / MoveStepTime;
	const float Alpha = FMath::Clamp(MoveInterpolationProgress, 0.f, 1.f);

	// Use Lerp for constant speed across the cell
	const FVector NewHeadLocation = FMath::Lerp(StepStartWorldLocation, StepTargetWorldLocation, Alpha);
	SetActorLocation(NewHeadLocation, false);

	UpdateBodyVisuals(Alpha); // Since we have now moved the head, we also want to update the body segment locations to follow the head smoothly, so we call UpdateBodyVisuals with the current interpolation alpha to position the body segments correctly between their previous and current grid positions.

	if (Alpha >= 1.f)
	{
		// We've reached the target grid location
		FinishMoveStep();
	}
}

void AABoxRoverPawn::StartNewMoveStep()
{
	HandleDirectionChange();

	const FIntPoint GridOffset = DirectionToGridOffset(CurrentDirection);
	PendingNextGridPosition = CurrentGridPosition + GridOffset;

	if (WouldHitWall(PendingNextGridPosition) || WouldHitSelf(PendingNextGridPosition))
	{
		HandleSnakeDeath();
		return;
	}

	// Cache the START and the END for this specific step
	PreviousBodyGridPositions = CurrentBodyGridPositions;
	TargetBodyGridPositions.Empty(CurrentBodyGridPositions.Num());
	
	if (CurrentBodyGridPositions.Num() > 0)
	{
		// Segment 0's target is always the current Head position
		TargetBodyGridPositions.Add(CurrentGridPosition);
		
		// Every other segment follows the previous segment's current position
		for (int32 i = 1; i < CurrentBodyGridPositions.Num(); i++)
		{
			TargetBodyGridPositions.Add(CurrentBodyGridPositions[i - 1]);
		}
	}
	
	StepStartWorldLocation = GridToWorldLocation(CurrentGridPosition);
	StepTargetWorldLocation = GridToWorldLocation(PendingNextGridPosition);
	MoveInterpolationProgress = 0.f; // Since we're just starting to move towards the new target, we reset the interpolation progress to 0
	bIsMovingToTarget = true;
}

FVector AABoxRoverPawn::GridToWorldLocation(const FIntPoint& GridPosition) const
{
	if (!GridManager)
	{
		return FVector::ZeroVector;
	}

	return GridManager->GetCellWorldPosition(GridPosition);
}

void AABoxRoverPawn::FinishMoveStep()
{
	// We've reached the target grid location, so we update our current grid position to the pending next grid position, and we can also update the body positions now that we've officially moved into the new cell:
	const FIntPoint OldHeadGridPosition = CurrentGridPosition;
	CurrentGridPosition = PendingNextGridPosition;

	// Body follows where the head / previous segment used to be
	if (CurrentBodyGridPositions.Num() > 0 || PendingGrowth > 0)
	{
		CurrentBodyGridPositions.Insert(OldHeadGridPosition, 0);
		if (PendingGrowth > 0)
		{
			PendingGrowth--;
		}
		else
		{
			CurrentBodyGridPositions.RemoveAt(CurrentBodyGridPositions.Num() - 1);
		}
	}
	
	SetActorLocation(StepTargetWorldLocation, false);
	UpdateBodyVisuals(1.f);
	bIsMovingToTarget = false;
}

void AABoxRoverPawn::UpdateBodyVisuals(float Alpha)
{
	EnsureBodySegmentMeshCount();

	for (int32 i = 0; i < BodySegmentMeshes.Num(); ++i)
	{
		if (!BodySegmentMeshes[i]) continue;

		// Guaranteed targets from the start of the move
		FIntPoint StartCell = PreviousBodyGridPositions.IsValidIndex(i) ?
		PreviousBodyGridPositions[i] : CurrentBodyGridPositions[i];
		
		// Target cell = where this segment should be after the current step
		FIntPoint TargetCell = TargetBodyGridPositions.IsValidIndex(i) ?
		TargetBodyGridPositions[i] : StartCell;

		const FVector StartWorldLocation = GridToWorldLocation(StartCell);
		const FVector TargetWorldLocation = GridToWorldLocation(TargetCell);
		
		// This Lerp is now stable
		const FVector NewWorldLocation = FMath::Lerp(StartWorldLocation, TargetWorldLocation, Alpha);

		BodySegmentMeshes[i]->SetWorldLocation(NewWorldLocation);
	}
}

void AABoxRoverPawn::EnsureBodySegmentMeshCount()
{
	// Add missing segments
	while (BodySegmentMeshes.Num() < CurrentBodyGridPositions.Num())
	{
		const int32 SegmentIndex = BodySegmentMeshes.Num();

		FName ComponentName = *FString::Printf(TEXT("BodySegmentMesh_%d"), SegmentIndex); // This is a unique name for the new mesh component, based on the current number of body segment meshes. This is important for Unreal Engine's component system, because each component needs to have a unique name within the actor, and by using the index we ensure that each new body segment mesh gets a unique name like "BodySegmentMesh_0", "BodySegmentMesh_1", etc.
		// Why is ComponentName a pointer? Because the constructor for UStaticMeshComponent takes a FName for the component name, and FName can be implicitly converted to a const FName& (which is what the constructor expects), so we can just pass ComponentName directly to the constructor. We also use NewObject to create the component, which is the recommended way to create components at runtime in Unreal Engine, because it properly initializes the component and registers it with the actor.
		UStaticMeshComponent* NewSegmentMesh = NewObject<UStaticMeshComponent>(this, ComponentName); // "this" refers to the ASnakePawn instance. We create a new UStaticMeshComponent as a child of the snake pawn, and we give it a unique name based on the current number of body segment meshes.

		if (!NewSegmentMesh)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create body segment mesh component"));
			return;
		}

		NewSegmentMesh->SetupAttachment(RootComponent);
		NewSegmentMesh->RegisterComponent(); // We need to register the component so that it becomes part of the actor and is rendered in the world. This is important for our snake pawn, because we want the new body segment mesh to appear in the game when it's created, and registering it ensures that it will be visible and properly attached to the snake pawn.
		NewSegmentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		NewSegmentMesh->SetSimulatePhysics(false);
		NewSegmentMesh->SetRelativeScale3D(BodySegmentMeshScale);

		if (BodySegmentMeshAsset)
		{
			NewSegmentMesh->SetStaticMesh(BodySegmentMeshAsset);
		}

		if (BodySegmentMaterial)
		{
			NewSegmentMesh->SetMaterial(0, BodySegmentMaterial); // 0 is the material index, which means we're setting the first material slot of the mesh.
		}

		BodySegmentMeshes.Add(NewSegmentMesh);
	}

	// Remove extra segment meshes.
	while (BodySegmentMeshes.Num() > CurrentBodyGridPositions.Num())
	{
		if (UStaticMeshComponent* LastSegment = BodySegmentMeshes.Last())
		{
			LastSegment->DestroyComponent();
		}
		BodySegmentMeshes.Pop();
	}
}


FIntPoint AABoxRoverPawn::DirectionToGridOffset(ESnakeDirection Direction) const
{
	switch (Direction)
	{
		
		case ESnakeDirection::Up:
		return FIntPoint(1,0);
		case ESnakeDirection::Down:
		return FIntPoint(-1,0);
		case ESnakeDirection::Left:
		return FIntPoint(0,-1);
		case ESnakeDirection::Right:
		return FIntPoint(0,1);
		default:return FIntPoint(0,0);
	}
}

void AABoxRoverPawn::AddInitialBodySegments(int32 NumSegments)
{
	CurrentBodyGridPositions.Empty();

	const FIntPoint BackwardOffset = DirectionToGridOffset(CurrentDirection) * -1; // We want to add body segments in the opposite direction of the current movement, so we multiply the grid offset by -1 to get the backward offset.
	FIntPoint NextBodyCell = CurrentGridPosition + BackwardOffset;

	for (int32 i = 0; i < NumSegments; ++i)
	{
		CurrentBodyGridPositions.Add(NextBodyCell);
		NextBodyCell += BackwardOffset; // Move the next body cell further back for the next segment
	}
}

FIntPoint AABoxRoverPawn::GetClampedStartGridPosition() const
{
	if (!GridManager)
	{
		return FIntPoint(1, 1);
	}

	const FVector Origin = GridManager->GridOrigin;
	const FVector WorldLocation = GetActorLocation();

	const int32 GridX = FMath::FloorToInt((WorldLocation.X - Origin.X) / CellSize);
	const int32 GridY = FMath::FloorToInt((WorldLocation.Y - Origin.Y) / CellSize);

	return FIntPoint(
		FMath::Clamp(GridX, 1, GridManager->GridWidth - 2),
		FMath::Clamp(GridY, 1, GridManager->GridHeight - 2)
	);
}

bool AABoxRoverPawn::WouldHitWall(const FIntPoint& NextCell) const
{
	if (!GridManager) return false;
	// This assumes our border cells are walls.
	return GridManager->IsBlocked(NextCell); // IsWallCell?
	//return NextCell.X <= 0 || NextCell.X >= GridDimensions.X - 1
	//	|| NextCell.Y <= 0 || NextCell.Y >= GridDimensions.Y - 1;
}

bool AABoxRoverPawn::WouldHitSelf(const FIntPoint& NextCell) const
{
	if (CurrentBodyGridPositions.Num() == 0)
	{
		return false;
	}

	// Special case:
	// Moving into the current tail cell is allowed if the tail will move away this step.
	const bool bTailWillStayThisStep = (PendingGrowth > 0);

	for (int32 i = 0; i < CurrentBodyGridPositions.Num(); ++i)
	{
		const bool bIsTail = (i == CurrentBodyGridPositions.Num() - 1);
		if (!bTailWillStayThisStep && bIsTail)
		{
			continue;
		}  

		if (CurrentBodyGridPositions[i] == NextCell)
		{
			return true;
		}
	}

	return false;
}

// Called to bind functionality to input
void AABoxRoverPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent called"));

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent valid"));
		
		if (IA_Up)
		{
			EnhancedInputComponent->BindAction(IA_Up, ETriggerEvent::Triggered, this, &AABoxRoverPawn::MoveUp);
			EnhancedInputComponent->BindAction(IA_Up, ETriggerEvent::Completed, this, &AABoxRoverPawn::MoveUp);
		}
		
		if (IA_Down)
		{
			EnhancedInputComponent->BindAction(IA_Down, ETriggerEvent::Triggered, this, &AABoxRoverPawn::MoveDown);
			EnhancedInputComponent->BindAction(IA_Down, ETriggerEvent::Completed, this, &AABoxRoverPawn::MoveDown);
		}
		
		if (IA_Left)
		{
			EnhancedInputComponent->BindAction(IA_Left, ETriggerEvent::Triggered, this, &AABoxRoverPawn::MoveLeft);
			EnhancedInputComponent->BindAction(IA_Left, ETriggerEvent::Completed, this, &AABoxRoverPawn::MoveLeft);
		}
		
		if (IA_Right)
		{
			EnhancedInputComponent->BindAction(IA_Right, ETriggerEvent::Triggered, this, &AABoxRoverPawn::MoveRight);
			EnhancedInputComponent->BindAction(IA_Right, ETriggerEvent::Completed, this, &AABoxRoverPawn::MoveRight);
		}
		// if (IA_Move)
		// {
		// 	UE_LOG(LogTemp, Warning, TEXT("Binding IA_Move"));
		// 	EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AABoxRoverPawn::Move);
		// 	EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Completed, this, &AABoxRoverPawn::Move);
		// }
		// else
		// {
		// 	UE_LOG(LogTemp, Error, TEXT("IA_Move is NULL"));
		// }
		//
		// if (IA_Turn)
		// {
		// 	UE_LOG(LogTemp, Warning, TEXT("Binding IA_Turn"));
		// 	EnhancedInputComponent->BindAction(IA_Turn, ETriggerEvent::Triggered, this, &AABoxRoverPawn::Turn);
		// 	EnhancedInputComponent->BindAction(IA_Turn, ETriggerEvent::Completed, this, &AABoxRoverPawn::Turn);
		// }
		// else
		// {
		// 	UE_LOG(LogTemp, Error, TEXT("IA_Turn is NULL"));
		// }
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EnhancedInputComponent cast failed"));
	}
}

void AABoxRoverPawn::MoveUp(const FInputActionValue& Value)
{
	//UE_LOG(LogTemp, Warning, TEXT("MoveUp called"));
	bool bPressed = Value.Get<bool>();
	if (bPressed)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveUp called"));
		RequestedDirection = ESnakeDirection::Up;
	}
}

void AABoxRoverPawn::MoveDown(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("MoveDown called"));
	bool bPressed = Value.Get<bool>();
	if (bPressed)
	{
		RequestedDirection = ESnakeDirection::Down;
	}
}

void AABoxRoverPawn::MoveRight(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("MoveRight called"));
	bool bPressed = Value.Get<bool>();
	if (bPressed)
	{
		RequestedDirection = ESnakeDirection::Right;
	}
}

void AABoxRoverPawn::MoveLeft(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("MoveLeft called"));
	bool bPressed = Value.Get<bool>();
	if (bPressed)
	{
		RequestedDirection = ESnakeDirection::Left;
	}
}

void AABoxRoverPawn::Input_TryTurnUp(const FInputActionValue& Value)
{
	// bool bPressed = Value.Get<bool>();
	// if (bPressed)
	// {
	//
	// 	RequestedDirection = ESnakeDirection::Up;
	// }
}

void AABoxRoverPawn::Input_TryTurnDown(const FInputActionValue& Value)
{
	// bool bPressed = Value.Get<bool>();
	// if (bPressed)
	// {
	// 	RequestedDirection = ESnakeDirection::Down;
	// }
}

void AABoxRoverPawn::Input_TryTurnLeft(const FInputActionValue& Value)
{
	// bool bPressed = Value.Get<bool>();
	// if (bPressed)
	// {
	// 	RequestedDirection = ESnakeDirection::Left;
	// }
}

void AABoxRoverPawn::Input_TryTurnRight(const FInputActionValue& Value)
{
	// bool bPressed = Value.Get<bool>();
	// if (bPressed)
	// {
	// 	RequestedDirection = ESnakeDirection::Right;
	// }
}

// void AABoxRoverPawn::Move(const FInputActionValue& Value)
// {
// 	MoveInput = Value.Get<float>();
// 	UE_LOG(LogTemp, Warning, TEXT("Move called: %f"), MoveInput);
// }
//
// void AABoxRoverPawn::Turn(const FInputActionValue& Value)
// {
// 	TurnInput = Value.Get<float>();
// 	UE_LOG(LogTemp, Warning, TEXT("Turn called: %f"), TurnInput);
// }

void AABoxRoverPawn::GrowSnake(int32 Growth)
{
	PendingGrowth += FMath::Max(Growth, 0);
}

void AABoxRoverPawn::HandleFoodOverlap(AFoodActor* FoodActor)
{
	if (bIsDead || !FoodActor)
		return;
	
	GrowSnake(1);
	FoodActor->DeactivateFood();
	OnFoodConsumed.Broadcast(10);
	
	//FoodActor->SetActorLocation(FoodActor->GetActorLocation());
}

void AABoxRoverPawn::HandleSnakeDeath()
{
	// UE_LOG(LogTemp, Warning, TEXT("HandleSnakeDeath entered on %s, bIsDead=%s"),
	// 	*GetName(),
	// 	bIsDead ? TEXT("true") : TEXT("false"));
	if (bIsDead)
		return;
	
	bIsDead = true;
	
	
	UE_LOG(LogTemp, Warning, TEXT("Pawn HandleSnakeDeath on %s"), *GetName());
	
	OnSnakeDied.Broadcast();
	
	
}

TArray<FIntPoint> AABoxRoverPawn::GetAllOccupiedGridCells() const
{
	TArray<FIntPoint> Occupied = CurrentBodyGridPositions;
	Occupied.Insert(CurrentGridPosition,0);
	
	return  Occupied;
}

void AABoxRoverPawn::ClearBodyVisuals()
{
	for (UStaticMeshComponent* SegmentMesh : BodySegmentMeshes)
	{
		if (SegmentMesh)
		{
			SegmentMesh->DestroyComponent();
		}
	}
	BodySegmentMeshes.Empty();
}

void AABoxRoverPawn::CacheGridManager()
{
	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	
}

