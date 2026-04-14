// Fill out your copyright notice in the Description page of Project Settings.


#include "ABoxRoverPawn.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
AABoxRoverPawn::AABoxRoverPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));
	RootComponent = CollisionSphere;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual Mesh"));
	VisualMesh->SetupAttachment(RootComponent);

	// SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	// SpringArm->SetupAttachment(RootComponent);
	
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
	
	RequestedDirection = CurrentDirection;
	UpdateDirection(CurrentDirection);	
	
	Super::BeginPlay();

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
}

// Called every frame
void AABoxRoverPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (AController* CurrentController  = GetController())
	{
		GEngine->AddOnScreenDebugMessage(
			3, 0.0f, FColor::Cyan,
			FString::Printf(TEXT("Controller: %s"), *CurrentController ->GetName())
		);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			3, 0.0f, FColor::Red,
			TEXT("Controller: NULL")
		);
	}
	// 前后移动
	if (!FMath::IsNearlyZero(MoveInput))
	{
		const FVector MoveDelta = GetActorForwardVector() * MoveInput * MoveSpeed * DeltaTime;
		AddActorWorldOffset(MoveDelta, true);
	}

	// 左右转向
	if (!FMath::IsNearlyZero(TurnInput))
	{
		const FRotator TurnDelta(0.0f, TurnInput * TurnSpeed * DeltaTime, 0.0f);
		AddActorLocalRotation(TurnDelta);
	}
	
	// if (bUseGridMovement)
	// {
	// 	// Interpolate towards the target
	// 	TickGridMovement(DeltaTime);
	// }
	// else
	// {
	// 	HandleDirectionChange();
	// 	TickFreeMovement(DeltaTime);
	// }
	HandleDirectionChange();
	TickFreeMovement(DeltaTime);
	
	if (GEngine)
	{
		TArray<UCameraComponent*> Cameras;
		GetComponents<UCameraComponent>(Cameras);

		for (int32 i = 0; i < Cameras.Num(); ++i)
		{
			if (Cameras[i])
			{
				UE_LOG(LogTemp, Warning, TEXT("Camera %d: %s Active=%s WorldRot=%s"),
					i,
					*Cameras[i]->GetName(),
					Cameras[i]->IsActive() ? TEXT("true") : TEXT("false"),
					*Cameras[i]->GetComponentRotation().ToString());
			}
		}
		
		
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (AActor* ViewTarget = PC->GetViewTarget())
			{
				GEngine->AddOnScreenDebugMessage(
					40, 0.f, FColor::White,
					FString::Printf(TEXT("ViewTarget: %s"), *ViewTarget->GetName())
				);
			}
		}
		
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			GEngine->AddOnScreenDebugMessage(
				20, 0.f, FColor::White,
				FString::Printf(TEXT("Control Rot: %s"), *PC->GetControlRotation().ToString())
			);

			if (PC->PlayerCameraManager)
			{
				GEngine->AddOnScreenDebugMessage(
					21, 0.f, FColor::Silver,
					FString::Printf(TEXT("CameraManager Rot: %s"), *PC->PlayerCameraManager->GetCameraRotation().ToString())
				);
			}
			
			if (SpringArm)
			{
				const FTransform SocketTransform = SpringArm->GetSocketTransform(USpringArmComponent::SocketName, RTS_World);

				GEngine->AddOnScreenDebugMessage(
					30, 0.f, FColor::Magenta,
					FString::Printf(TEXT("SpringArm Socket Rot: %s"), *SocketTransform.GetRotation().Rotator().ToString())
				);
			}
		}
		
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
	}
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



