// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerMainCharacterController.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerCamera.h"

APlayerMainCharacterController::APlayerMainCharacterController() {
	bSprint = false;
}

void APlayerMainCharacterController::BeginPlay() {
	Super::BeginPlay();
	CMC = reinterpret_cast<UMainCharacterMovementComponent*>(GetCharacter()->GetCharacterMovement());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void APlayerMainCharacterController::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerMainCharacterController::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerMainCharacterController::Look);
		EnhancedInputComponent->BindAction(dbjump, ETriggerEvent::Triggered, this, &APlayerMainCharacterController::DBJump);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &APlayerMainCharacterController::Crouch);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &APlayerMainCharacterController::Sprint);
		EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Triggered, this, &APlayerMainCharacterController::Grapple);
	}
}

void APlayerMainCharacterController::DBJump(const FInputActionValue& Value) {
	CMC->Jump(Value.Get<FVector2D>());
}

void APlayerMainCharacterController::Move(const FInputActionValue& Value) {
	CMC->Move(Value.Get<FVector2D>());
}

void APlayerMainCharacterController::Look(const FInputActionValue& Value) {
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	camera->HandleInput(LookAxisVector);
	AddYawInput(LookAxisVector.X);
}

void APlayerMainCharacterController::Crouch(const FInputActionValue& Value) {
	CMC->Crouch(Value.Get<bool>());
}

void APlayerMainCharacterController::Sprint(const FInputActionValue& Value) {
	if (Value.Get<bool>()) CMC->Sprint(true);
}

void APlayerMainCharacterController::Grapple(const FInputActionValue& Value) {
	CMC->StartGrapple();
}