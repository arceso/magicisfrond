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
	WRData.Side = ESide::NONE;
	SetActorTickEnabled(true);
}

void AMyCharacterController::BeginPlay() {
	Super::BeginPlay();
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AMyCharacterController::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
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
	//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("YES"));
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
	WRData.Side = ESide::NONE;
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
		WRData.Side = Side;
		WRData.NormalHit = normal;
		camera->SetCameraSide(Side == ESide::LEFT ? ESide::RIGHT : ESide::LEFT);
	}
}

void AMyCharacterController::WR_Movement(ESide side, FHitResult fhr) {
	GetCharacter()->GetCharacterMovement()->StopMovementKeepPathing();

	FVector NewForward = fhr.ImpactNormal.Cross(FVector(0, 0, (side == ESide::RIGHT ? -1 : 1)));

	GetCharacter()->SetActorRotation(NewForward.Rotation());
	GetCharacter()->SetActorLocation(GetCharacter()->GetActorLocation() + (NewForward.GetSafeNormal() * 50));
	// If hitLocation - GetCharacter()->GetActorLocation > un poquito
	// Then pull todwars rightvector * (side == Right? 1 : -1 )
	// Endif
}

void AMyCharacterController::UpdateWallrun(FVector_NetQuantizeNormal* newNormal) {
	WRData.NormalHit = *newNormal;
}

void AMyCharacterController::WR_Move(const FInputActionValue& Value) {
	FVector2D MovementVector = Value.Get<FVector2D>();
	const int length = 100;
	float speeb = 500.f;
	struct FHitResult OutHit;
	const FVector Start = GetCharacter()->GetActorLocation() + (GetCharacter()->GetActorForwardVector() * 10);
	const FVector End = Start + WRData.NormalHit * -1 * length;
	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), true, NULL);
	TraceParams.AddIgnoredActor(GetCharacter());
	bool isWallFound = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, TraceParams);
	//DrawDebugLine(GetWorld(), Start, End, FColor(255, 0, 0), false, -1, 0, 2);
	if (isWallFound) {
		WRData.NormalHit = OutHit.ImpactNormal;
		if (MovementVector.Y >= .5f) {
			if (WRData.Side == ESide::RIGHT && MovementVector.X > .0f) WR_Movement(ESide::RIGHT, OutHit);
			else if (WRData.Side == ESide::LEFT && MovementVector.X < .0f) WR_Movement(ESide::LEFT, OutHit);
			else EndWallrun();
		} else EndWallrun();
	} else EndWallrun();
}

void AMyCharacterController::WR_Look(const FInputActionValue& Value) {
}

void AMyCharacterController::WR_dbjump(const FInputActionValue& Value) {
	EndWallrun();
	GetCharacter()->LaunchCharacter((GetCharacter()->GetActorForwardVector() + (WRData.Side == ESide::LEFT ? 1 : -1) * GetCharacter()->GetActorRightVector()) * 1000, true, true);
}