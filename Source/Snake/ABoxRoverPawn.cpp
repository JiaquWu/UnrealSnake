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

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

// Called when the game starts or when spawned
void AABoxRoverPawn::BeginPlay()
{
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
	UE_LOG(LogTemp, Warning, TEXT("MoveUp called"));
}

void AABoxRoverPawn::MoveDown(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("MoveDown called"));
}

void AABoxRoverPawn::MoveRight(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("MoveRight called"));
}

void AABoxRoverPawn::MoveLeft(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("MoveLeft called"));
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



