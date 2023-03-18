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

APlayerMainCharacterController::APlayerMainCharacterController() {}

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

void APlayerMainCharacterController::Grapple(const FInputActionValue& Value) {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("Graple time!") );
	FHitResult OutHit;
	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), false, GetCharacter());
	if (GetWorld()->LineTraceSingleByChannel(
		OutHit,
		camera->GetComponentLocation(),
		camera->GetComponentLocation() + camera->GetForwardVector() * 5000,
		ECC_WorldStatic,
		TraceParams
	)) {
		DrawDebugLine(GetWorld(), OutHit.Location, GetCharacter()->GetActorLocation(), FColor::Red, false, 15.f, 0, 5);
		GetCharacter()->GetMovementComponent();
	}
}