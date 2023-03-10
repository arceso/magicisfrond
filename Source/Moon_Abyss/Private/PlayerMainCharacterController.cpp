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
	WRData.NormalHit = FVector(0, 0, 0);
	WRData.WallRuning = false;
	WRData.Side = ESide::NONE;
	SetActorTickEnabled(true);
	bCanAirJump = true;
}

void APlayerMainCharacterController::BeginPlay() {
	Super::BeginPlay();
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void APlayerMainCharacterController::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerMainCharacterController::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerMainCharacterController::Look);
		EnhancedInputComponent->BindAction(dbjump, ETriggerEvent::Triggered, this, &APlayerMainCharacterController::DBJump);
		EnhancedInputComponent->BindAction(WR_MoveAction, ETriggerEvent::Triggered, this, &APlayerMainCharacterController::WR_Move);
		EnhancedInputComponent->BindAction(WR_LookAction, ETriggerEvent::Triggered, this, &APlayerMainCharacterController::WR_Look);
		EnhancedInputComponent->BindAction(WR_dbjumpAction, ETriggerEvent::Triggered, this, &APlayerMainCharacterController::WR_dbjump);
	}
}

void APlayerMainCharacterController::DBJump(const FInputActionValue& Value) {
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, bCanAirJump ? TEXT("YES") : TEXT("NO"));
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (GetCharacter()->CanJump()) GetCharacter()->Jump();
	else if (bCanAirJump) {
		bCanAirJump = false;
		FVector VInMDir = FVector(MovementVector.GetSafeNormal(), 1) * (GetCharacter()->GetMovementComponent()->Velocity.Length() * .5);
		// Magic Numbers
		VInMDir.Z += 500;
		FVector finalV = GetControlRotation().RotateVector(VInMDir);
		GetCharacter()->LaunchCharacter(finalV, true, true);
	}
}

void APlayerMainCharacterController::Move(const FInputActionValue& Value) {
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (GetCharacter()->Controller != nullptr) {
		const FRotator YawRotation(0, GetControlRotation().Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		GetCharacter()->AddMovementInput(ForwardDirection, MovementVector.Y);
		GetCharacter()->AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerMainCharacterController::Look(const FInputActionValue& Value) {
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	camera->HandleInput(LookAxisVector);
	AddYawInput(LookAxisVector.X);
}

void APlayerMainCharacterController::EndWallrun() {
	WRData.WallRuning = false;
	WRData.Side = ESide::NONE;
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
		Subsystem->RemoveMappingContext(WallrunMappingContext);
	}

	GetCharacter()->GetCharacterMovement()->bConstrainToPlane = false;
	bCanAirJump = true;
	SetControlRotation(GetCharacter()->GetActorRotation());
	GetCharacter()->bUseControllerRotationYaw = true;
}

void APlayerMainCharacterController::StartWallrun(ESide Side, FVector normal) {
	if (!WRData.WallRuning && GetCharacter()->GetCharacterMovement()->IsFalling()) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
			Subsystem->AddMappingContext(WallrunMappingContext, 1);
		}

		// If not set, beautiful things happens. Should look into it.
		GetCharacter()->GetCharacterMovement()->bConstrainToPlane = true;
		GetCharacter()->bUseControllerRotationYaw = false;
		WRData.WallRuning = true;
		WRData.Side = Side;
		WRData.NormalHit = normal;
		camera->SetCameraSide(Side == ESide::LEFT ? ESide::RIGHT : ESide::LEFT);
	}
}

// Move to movement component :)
void APlayerMainCharacterController::WR_Movement(ESide side, FHitResult fhr) {
	GetCharacter()->GetCharacterMovement()->StopMovementKeepPathing();
	float rot = MAX_ROTATION * GetWorld()->GetDeltaSeconds() * SPEED;
	if (!fhr.ImpactNormal.IsZero()) {
		const int SideValue = (side == ESide::RIGHT ? -1 : 1);
		FVector NewForward = fhr.ImpactNormal.Cross(FVector(0, 0, SideValue));
		FRotator NewRotation = NewForward.Rotation();// *MAX_ROTATION* GetWorld()->GetDeltaSeconds(); //(NewForward.Rotation()) - GetCharacter()->GetActorForwardVector().Rotation();
		FRotator RotDifference = NewRotation.Clamp() - GetCharacter()->GetActorForwardVector().Rotation().Clamp();

		if (RotDifference.Yaw > 300.f) RotDifference = NewRotation.Clamp() - FRotator(0, GetCharacter()->GetActorForwardVector().Rotation().Clamp().Yaw + 360,0);
		else if (RotDifference.Yaw < -300.f) RotDifference = NewRotation.Clamp() - FRotator(0, GetCharacter()->GetActorForwardVector().Rotation().Clamp().Yaw - 360, 0);
		
		GetCharacter()->AddActorLocalRotation(RotDifference * rot);

		NewForward *= SPEED * GetWorld()->GetDeltaSeconds();

		if (fhr.Distance > MAX_DISTANCE) NewForward += fhr.ImpactNormal * -1 * (fhr.Distance - MAX_DISTANCE); // If too far get closer
		else if (fhr.Distance < MIN_DISTANCE)NewForward += fhr.ImpactNormal * (MIN_DISTANCE - fhr.Distance);  // If too close get further away

		GetCharacter()->SetActorLocation(GetCharacter()->GetActorLocation() + NewForward, true);
	}
	else GetCharacter()->SetActorLocation(GetCharacter()->GetActorLocation() + (GetCharacter()->GetActorForwardVector() * SPEED * GetWorld()->GetDeltaSeconds()), true);
}

void APlayerMainCharacterController::UpdateWallrun(FVector_NetQuantizeNormal* newNormal) {
	WRData.NormalHit = *newNormal;
}

void APlayerMainCharacterController::WR_Move(const FInputActionValue& Value) {
	FVector2D MovementVector = Value.Get<FVector2D>();

	FHitResult OutHit;

	// Magic Numbers
	const FVector Start = GetCharacter()->GetActorLocation() + (GetCharacter()->GetActorForwardVector() * SPEED * GetWorld()->GetDeltaSeconds());
	const FVector End = Start + WRData.NormalHit * -1 * MAX_DISTANCE * 2;

	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), true, NULL);
	TraceParams.AddIgnoredActor(GetCharacter());
	bool isWallFound = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, TraceParams);
	if (isWallFound) {
		WRData.NormalHit = OutHit.ImpactNormal;
		if (MovementVector.Y >= .5f) {
			if (WRData.Side == ESide::RIGHT && MovementVector.X > .0f) WR_Movement(ESide::RIGHT, OutHit);
			else if (WRData.Side == ESide::LEFT && MovementVector.X < .0f) WR_Movement(ESide::LEFT, OutHit);
			else EndWallrun();
		}
		else EndWallrun();
	}
	else EndWallrun();
}

void APlayerMainCharacterController::WR_Look(const FInputActionValue& Value) {
}

void APlayerMainCharacterController::WR_dbjump(const FInputActionValue& Value) {
	EndWallrun();

	// Magic Numbers
	GetCharacter()->LaunchCharacter((
		GetCharacter()->GetActorForwardVector() * JUMP_INFLUENCE.X +
		WRData.NormalHit * JUMP_INFLUENCE.Y +
		FVector(0, 0, 1) * JUMP_INFLUENCE.Z
	) * JUMP_FORCE, true, true);
}