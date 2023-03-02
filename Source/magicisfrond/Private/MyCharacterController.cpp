// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterController.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MyCharacterCamera.h"

AMyCharacterController::AMyCharacterController() {
	WRData.NormalHit = FVector(0, 0, 0);
	WRData.WallRuning = false;
	WRData.WR_SIDE = ESide::NONE;
}

void AMyCharacterController::BeginPlay() {
	Super::BeginPlay();
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AMyCharacterController::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyCharacterController::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCharacterController::Look);
		EnhancedInputComponent->BindAction(dbjump, ETriggerEvent::Triggered, this, &AMyCharacterController::DBJump);
		EnhancedInputComponent->BindAction(WR_MoveAction, ETriggerEvent::Triggered, this, &AMyCharacterController::WR_Move);
		EnhancedInputComponent->BindAction(WR_LookAction, ETriggerEvent::Triggered, this, &AMyCharacterController::WR_Look);
		EnhancedInputComponent->BindAction(WR_dbjumpAction, ETriggerEvent::Triggered, this, &AMyCharacterController::WR_dbjump);
	}
}

void AMyCharacterController::DBJump(const FInputActionValue& Value) {
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (GetCharacter()->GetCharacterMovement()->IsFalling() && bCanAirJump) {
		bCanAirJump = false;
		FVector VInMDir = FVector(MovementVector.GetSafeNormal(), 1) * (GetCharacter()->GetMovementComponent()->Velocity.Length() * .5);
		VInMDir.Z += 1000;
		FVector finalV = GetControlRotation().RotateVector(VInMDir);
		GetCharacter()->LaunchCharacter(finalV, true, true);
	}
	else GetCharacter()->Jump();
}

void AMyCharacterController::Move(const FInputActionValue& Value) {
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (GetCharacter()->Controller != nullptr) {
		const FRotator YawRotation(0, GetControlRotation().Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		GetCharacter()->AddMovementInput(ForwardDirection, MovementVector.Y);
		GetCharacter()->AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMyCharacterController::Look(const FInputActionValue& Value) {
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	camera->HandleInput(LookAxisVector);
	AddYawInput(LookAxisVector.X);
}

void AMyCharacterController::EndWallrun() {
	WRData.WallRuning = false;
	//WRData.WR_SIDE = NONE;
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
		Subsystem->RemoveMappingContext(WallrunMappingContext);
	}
	
	GetCharacter()->GetCharacterMovement()->bConstrainToPlane = false;
	bCanAirJump = true;
	GetCharacter()->bUseControllerRotationYaw = true;
}

void AMyCharacterController::StartWallrun(ESide Side, FVector normal) {
	if (!WRData.WallRuning && GetCharacter()->GetCharacterMovement()->IsFalling()) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
			Subsystem->AddMappingContext(WallrunMappingContext, 1);
		}
		
		GetCharacter()->GetCharacterMovement()->bConstrainToPlane = true;
		GetCharacter()->bUseControllerRotationYaw = false;
		WRData.WallRuning = true;
		WRData.WR_SIDE = Side;
		WRData.NormalHit = normal;
		camera->SetCameraSide(Side == ESide::LEFT ? ESide::RIGHT : ESide::LEFT);
	}
}

void AMyCharacterController::WR_Movement(ESide side, float movement, FHitResult fhr) {
	GetCharacter()->GetCharacterMovement()->StopMovementKeepPathing();
	FRotator RotationOf90Degrees(0, side == ESide::LEFT ? -90 : 90, 0);
	FRotator LeftOrRightDirection = RotationOf90Degrees.RotateVector(fhr.Normal).Rotation();
	FRotator newRotation(0, LeftOrRightDirection.Yaw, 0);

	GetCharacter()->SetActorRotation(newRotation);
	FVector NewLoc = FRotationMatrix(fhr.Normal.Rotation()).GetScaledAxis(EAxis::Y) * movement * 20;

	if (side == ESide::LEFT) NewLoc = -NewLoc + GetCharacter()->GetActorLocation();
	else NewLoc = NewLoc + GetCharacter()->GetActorLocation();

	GetCharacter()->SetActorLocation(NewLoc); // , true, NULL, ETeleportType::TeleportPhysics);
}

void AMyCharacterController::WR_Move(const FInputActionValue& Value) {
	FVector2D MovementVector = Value.Get<FVector2D>();
	const int length = 10;
	struct FHitResult OutHit;
	const FVector Start = GetCharacter()->GetActorLocation() + GetCharacter()->GetActorForwardVector();
	const FVector End = Start + WRData.NormalHit * -1 * length;
	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), true, NULL);
	TraceParams.AddIgnoredActor(this);
	bool isWallFound = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, TraceParams);

	if (isWallFound) {
		WRData.NormalHit = OutHit.ImpactNormal;
		if (MovementVector.Y >= .5f) {
			if (WRData.WR_SIDE == ESide::RIGHT && MovementVector.X > .0f) WR_Movement(ESide::RIGHT, MovementVector.Y, OutHit);
			else if (WRData.WR_SIDE == ESide::LEFT && MovementVector.X < .0f) WR_Movement(ESide::LEFT, MovementVector.Y, OutHit);
			else EndWallrun();
		}
		else EndWallrun();

	}
	else EndWallrun();
}

void AMyCharacterController::WR_Look(const FInputActionValue& Value) {
}

void AMyCharacterController::WR_dbjump(const FInputActionValue& Value) {
	EndWallrun();
	GetCharacter()->LaunchCharacter((GetCharacter()->GetActorForwardVector() + (WRData.WR_SIDE == ESide::LEFT ? 1 : -1) * GetCharacter()->GetActorRightVector()) * 1000, true, true);
}
