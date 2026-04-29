// Fill out your copyright notice in the Description page of Project Settings.


#include "SnakePlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "InputCoreTypes.h"

#include "Snake/ABoxRoverPawn.h"
#include "TimerManager.h"

class UUserWidget;

void ASnakePlayerController::BeginPlay()
{
	Super::BeginPlay();

	bAutoManageActiveCameraTarget = false;
}

void ASnakePlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();

	ApplyControlledSnakeCamera();
}

void ASnakePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	// if (WasInputKeyJustPressed(EKeys::Gamepad_DPad_Up))
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("%s received Gamepad_DPad_Up | ControlledSnake=%s"),
	// 		*GetNameSafe(this),
	// 		*GetNameSafe(ControlledSnake));
	// }
	//
	// if (WasInputKeyJustPressed(EKeys::W))
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("%s received W | ControlledSnake=%s"),
	// 		*GetNameSafe(this),
	// 		*GetNameSafe(ControlledSnake));
	// }
	
	// if (ControlledSnake && GetViewTarget() != ControlledSnake)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("%s ViewTarget changed from %s to %s, fixing."),
	// 		*GetNameSafe(this),
	// 		*GetNameSafe(GetViewTarget()),
	// 		*GetNameSafe(ControlledSnake));
	//
	// 	ApplyControlledSnakeCamera();
	// }

	// if (WasInputKeyJustPressed(EKeys::W))
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("%s received W. Pawn=%s"),
	// 		*GetNameSafe(this),
	// 		*GetNameSafe(GetPawn()));
	// }
	//
	// if (WasInputKeyJustPressed(EKeys::Up))
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("%s received Up Arrow. Pawn=%s"),
	// 		*GetNameSafe(this),
	// 		*GetNameSafe(GetPawn()));
	// }
	//
	// if (WasInputKeyJustPressed(EKeys::Gamepad_DPad_Up))
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("%s received Gamepad DPad Up. Pawn=%s"),
	// 		*GetNameSafe(this),
	// 		*GetNameSafe(GetPawn()));
	// }
}

void ASnakePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);

				UE_LOG(LogTemp, Warning, TEXT("%s added mapping context %s"),
					*GetNameSafe(this),
					*GetNameSafe(DefaultMappingContext));
			}
		}
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("%s has no EnhancedInputComponent"), *GetNameSafe(this));
		return;
	}

	if (IA_Up)
	{
		EnhancedInputComponent->BindAction(IA_Up, ETriggerEvent::Triggered, this, &ASnakePlayerController::MoveUp);
	}

	if (IA_Down)
	{
		EnhancedInputComponent->BindAction(IA_Down, ETriggerEvent::Triggered, this, &ASnakePlayerController::MoveDown);
	}

	if (IA_Left)
	{
		EnhancedInputComponent->BindAction(IA_Left, ETriggerEvent::Triggered, this, &ASnakePlayerController::MoveLeft);
	}

	if (IA_Right)
	{
		EnhancedInputComponent->BindAction(IA_Right, ETriggerEvent::Triggered, this, &ASnakePlayerController::MoveRight);
	}
	
	if (IA_VerticalUp)
	{
		EnhancedInputComponent->BindAction(IA_VerticalUp,ETriggerEvent::Triggered, this, &ASnakePlayerController::MoveVerticalUp);
	}

	if (IA_VerticalDown)
	{
		EnhancedInputComponent->BindAction(IA_VerticalDown, ETriggerEvent::Triggered, this, &ASnakePlayerController::MoveVerticalDown);
	}
}

void ASnakePlayerController::SetControlledSnake(AABoxRoverPawn* NewSnake)
{
	this->ControlledSnake = NewSnake;
	
	ApplyControlledSnakeCamera();
	GetWorldTimerManager().SetTimerForNextTick(this, &ASnakePlayerController::ApplyControlledSnakeCamera);
	
	UE_LOG(LogTemp, Warning, TEXT("%s ControlledSnake = %s ViewTarget=%s"),
		*GetNameSafe(this),
		*GetNameSafe(this->ControlledSnake),
		*GetNameSafe(GetViewTarget()));
	
	// if (ControlledSnake)
	// {
	// 	SetViewTargetWithBlend(ControlledSnake, 0.f);
	// }

	// UE_LOG(LogTemp, Warning, TEXT("%s ControlledSnake = %s ViewTarget=%s"),
	// 	*GetNameSafe(this),
	// 	*GetNameSafe(ControlledSnake),
	// 	*GetNameSafe(GetViewTarget()));
}
void ASnakePlayerController::ApplyControlledSnakeCamera()
{
	if (!ControlledSnake)
	{
		return;
	}

	SetViewTargetWithBlend(ControlledSnake, 0.f);

	UE_LOG(LogTemp, Warning, TEXT("%s ApplyCamera | ControlledSnake=%s | ViewTarget=%s"),
		*GetNameSafe(this),
		*GetNameSafe(ControlledSnake),
		*GetNameSafe(GetViewTarget()));
}


void ASnakePlayerController::MoveUp(const FInputActionValue& Value)
{
	if (!Value.Get<bool>() || !ControlledSnake)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("%s MoveUp -> %s"),
		*GetNameSafe(this),
		*GetNameSafe(ControlledSnake));

	ControlledSnake->RequestDirection(ESnakeDirection::Up);
}

void ASnakePlayerController::MoveDown(const FInputActionValue& Value)
{
	if (!Value.Get<bool>() || !ControlledSnake)
	{
		return;
	}

	ControlledSnake->RequestDirection(ESnakeDirection::Down);
}

void ASnakePlayerController::MoveLeft(const FInputActionValue& Value)
{
	if (!Value.Get<bool>() || !ControlledSnake)
	{
		return;
	}

	ControlledSnake->RequestDirection(ESnakeDirection::Left);
}

void ASnakePlayerController::MoveRight(const FInputActionValue& Value)
{
	if (!Value.Get<bool>() || !ControlledSnake)
	{
		return;
	}

	ControlledSnake->RequestDirection(ESnakeDirection::Right);
}

void ASnakePlayerController::MoveVerticalUp(const FInputActionValue& Value)
{
	if (!Value.Get<bool>() || !ControlledSnake)
	{
		return;
	}

	ControlledSnake->RequestDirection(ESnakeDirection::VerticalUp);
}

void ASnakePlayerController::MoveVerticalDown(const FInputActionValue& Value)
{
	if (!Value.Get<bool>() || !ControlledSnake)
	{
		return;
	}

	ControlledSnake->RequestDirection(ESnakeDirection::VerticalDown);
}

void ASnakePlayerController::HideAllWidgets()
{
	if (ActiveWidget)
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr;
	}
}

void ASnakePlayerController::ShowHUD()
{
	HideAllWidgets();
	if (!HUDWidgetClass)
		return;
	
	ActiveWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
	if (ActiveWidget)
		ActiveWidget->AddToViewport();
	
	FInputModeGameOnly InputMode;
	//InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void ASnakePlayerController::ShowMainMenu()
{
	HideAllWidgets();
	if (!MainMenuWidgetClass)
		return;
	
	ActiveWidget = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
	if (ActiveWidget)
		ActiveWidget->AddToViewport();
	
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void ASnakePlayerController::ShowOutro()
{
	HideAllWidgets();
	if (!OutroWidgetClass)
		return;
	
	ActiveWidget = CreateWidget<UUserWidget>(this, OutroWidgetClass);
	if (ActiveWidget)
		ActiveWidget->AddToViewport();
	
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}
