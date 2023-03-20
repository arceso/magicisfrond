// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "CustomMovementFlags.h"


void UMainCharacterMovementComponent::BeginPlay(){
	Super::BeginPlay();
	SelfExcludeQueryParams.AddIgnoredActor(GetOwner());
	SetMovementMode(EMovementMode::MOVE_Walking);
	WRData.WallRuning = false;
	WRData.Side = ESide::None;
	bCanAirJump = true;
	bIsSprinting = false;
	AirJumpHeightGain = 500.f;
	EndWallLaunchForce = 1000.f;
	SetPlaneConstraintNormal(FVector(0, 0, 1));
	AirControl = 1;
}

void UMainCharacterMovementComponent::Walking(const FVector2D& input) {
	const FRotator YawRotation(0, PawnOwner->GetController()->GetControlRotation().Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	PawnOwner->AddMovementInput(ForwardDirection, input.Y);
	PawnOwner->AddMovementInput(RightDirection, input.X);
}

void UMainCharacterMovementComponent::Falling(const FVector2D& input) {
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("Falling"));
	const FRotator YawRotation(0, PawnOwner->GetController()->GetControlRotation().Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	PawnOwner->AddMovementInput(ForwardDirection, input.Y * AirInfluenceControl);
	PawnOwner->AddMovementInput(RightDirection, input.X * AirInfluenceControl);
}

void UMainCharacterMovementComponent::Sliding(const FVector2D& input) {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("Sliding"));
}


void UMainCharacterMovementComponent::Crouching(const FVector2D& input) {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("Crouching"));
}

void UMainCharacterMovementComponent::Sprinting(const FVector2D& input)
{

}

void UMainCharacterMovementComponent::Grappling(const FVector2D& input) {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("Grappling"));
}


void UMainCharacterMovementComponent::AddInputVector(FVector WorldAccel, bool bForce /*=false*/) {
	if (PawnOwner && !WorldAccel.IsZero()) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, WorldAccel.ToString());
		PawnOwner->Internal_AddMovementInput(WorldAccel, bForce);
	}
}


void UMainCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
};


void UMainCharacterMovementComponent::Move(const FVector2D& input) {
	if (MovementMode == MOVE_Custom) {
		if (CustomMovementMode == CMOVE_Sliding) Sliding(input);
		if (CustomMovementMode == CMOVE_Wallruning) Wallruning(input);
		if (CustomMovementMode == CMOVE_Crouching) Crouching(input);
		if (CustomMovementMode == CMOVE_Grappling) Grappling(input);
	} else {
		if (MovementMode == MOVE_Walking) Walking(input);
		if (MovementMode == MOVE_Falling) Falling(input);
	}
}

void UMainCharacterMovementComponent::Jump(const FVector2D& input) {
	if (MovementMode == MOVE_Custom) {
		//if (CustomMovementMode == CMOVE_Sliding) SlidingJump(input);
		if (CustomMovementMode == CMOVE_Wallruning) WallrunJump(input);
		//if (CustomMovementMode == CMOVE_Crouching) CrouchingJump(input);
		//if (CustomMovementMode == CMOVE_Grappling) GrapplingJump(input);
	}
	else {
		if (MovementMode == MOVE_Walking) WalkJump(input);
		if (MovementMode == MOVE_Falling) FallJump(input);
	}
}

/// STATE GETTERS
bool UMainCharacterMovementComponent::isSliding() { return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Sliding; }
bool UMainCharacterMovementComponent::isWallruning() { return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Wallruning; }
bool UMainCharacterMovementComponent::isCrouching() { return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Crouching; }
bool UMainCharacterMovementComponent::isGrappling() { return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Grappling; }
bool UMainCharacterMovementComponent::isWalking() { return MovementMode == MOVE_Walking; }
bool UMainCharacterMovementComponent::isFalling() { return MovementMode == MOVE_Falling; }
bool UMainCharacterMovementComponent::isSprinting() { return bIsSprinting; }

void UMainCharacterMovementComponent::WalkJump(const FVector2D& input) {
	GetCharacterOwner()->Jump();
}

void UMainCharacterMovementComponent::Sprint(bool bStart) {
	bIsSprinting = bStart;
	if (bStart) this->MaxWalkSpeed += 1000;
	else this->MaxWalkSpeed -= 1000;
}

void UMainCharacterMovementComponent::FallJump(const FVector2D& input) {
	if (bCanAirJump) {
		bCanAirJump = false;
		FVector VInMDir = GetCharacterOwner()->GetActorForwardVector().Rotation().RotateVector(FVector(input.GetSafeNormal(), 0) * (Velocity.Length()));
		// Magic Numbers
		VInMDir.Z += AirJumpHeightGain;
		//FVector finalV = GetControlRotation().RotateVector(VInMDir);
		GetCharacterOwner()->LaunchCharacter(VInMDir, true, true);
	}
}
void UMainCharacterMovementComponent::Wallruning(const FVector2D& input) {
	// Magic Numbers
	const FVector Start = GetActorLocation() + (GetPawnOwner()->GetActorForwardVector() * Velocity.Length() * GetWorld()->GetDeltaSeconds());
	const FVector End = Start + WRData.NormalHit * -1 * 100 * 2; // Magic number for the time being

	bool isWallFound = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, SelfExcludeQueryParams);
	if (isWallFound) {
		WRData.NormalHit = OutHit.ImpactNormal;
		if (input.Y >= .5f) {
			if (WRData.Side == ESide::Right && input.X > .0f) Wallrun(ESide::Right, OutHit);
			else if (WRData.Side == ESide::Left && input.X < .0f) Wallrun(ESide::Left, OutHit);
			else EndWallrun(EEndReason::InvalidInput);
		}
		else EndWallrun(EEndReason::InvalidInput);
	}
	else EndWallrun(EEndReason::NoWallFound);
}


///////////////////////////////////////////////////////////////
/// WALLRUN!!
/// 

void UMainCharacterMovementComponent::StartWallrun(ESide Side, FVector normal) {
	if (!WRData.WallRuning && IsFalling()) {
		// If not set, beautiful things happens. Should look into it.
		SetMovementMode(MOVE_Custom, CMOVE_Wallruning);
		bConstrainToPlane = true;
		GetPawnOwner()->bUseControllerRotationYaw = false;
		WRData.WallRuning = true;
		WRData.Side = Side;
		WRData.NormalHit = normal;
		GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Cyan, TEXT("PAesequefunshiona"));
	}
}

void UMainCharacterMovementComponent::Wallrun(ESide side, FHitResult fhr) {
	StopMovementKeepPathing();
	float rot = MAX_ROTATION * GetWorld()->GetDeltaSeconds() * SPEED;
	if (!fhr.ImpactNormal.IsZero()) {
		int SideValue = 0;
		if (side == ESide::Right) SideValue = -1;
		else if (side == ESide::Left) SideValue = 1;
		FVector NewForward = fhr.ImpactNormal.Cross(FVector(0, 0, SideValue));
		FRotator NewRotation = NewForward.Rotation();
		FRotator RotDifference = NewRotation.Clamp() - GetOwner()->GetActorForwardVector().Rotation().Clamp();

		if (RotDifference.Yaw > 300.f) RotDifference = NewRotation.Clamp() - FRotator(0, GetOwner()->GetActorForwardVector().Rotation().Clamp().Yaw + 360, 0);
		else if (RotDifference.Yaw < -300.f) RotDifference = NewRotation.Clamp() - FRotator(0, GetOwner()->GetActorForwardVector().Rotation().Clamp().Yaw - 360, 0);

		GetOwner()->AddActorLocalRotation(RotDifference * rot);

		NewForward *= SPEED * GetWorld()->GetDeltaSeconds();

		if (fhr.Distance > MAX_DISTANCE) NewForward += fhr.ImpactNormal * -1 * (fhr.Distance - MAX_DISTANCE); // If too far get closer
		else if (fhr.Distance < MIN_DISTANCE)NewForward += fhr.ImpactNormal * (MIN_DISTANCE - fhr.Distance);  // If too close get further away

		GetOwner()->SetActorLocation(GetActorLocation() + NewForward, true);
	} else GetOwner()->SetActorLocation(GetActorLocation() + (GetOwner()->GetActorForwardVector() * SPEED * GetWorld()->GetDeltaSeconds()), true);
}

void UMainCharacterMovementComponent::UpdateWallrun(FVector_NetQuantizeNormal* newNormal) {
	WRData.NormalHit = *newNormal;
}

void UMainCharacterMovementComponent::EndWallrun(EEndReason reason) {
	WRData.WallRuning = false;
	WRData.Side = ESide::None;

	SetMovementMode(MOVE_Falling);

	bConstrainToPlane = false;
	bCanAirJump = true;
	GetPawnOwner()->GetController()->SetControlRotation(GetPawnOwner()->GetActorRotation());
	GetPawnOwner()->bUseControllerRotationYaw = true;

	if (reason == EEndReason::Jump) {
		// Pos has saltao, paco.
	}
	else if (reason == EEndReason::NoWallFound) {
		FVector VInMDir = GetCharacterOwner()->GetActorForwardVector() * EndWallLaunchForce;
		GetCharacterOwner()->LaunchCharacter(VInMDir, true, true);
	}
	else if (reason == EEndReason::Hit) {
		GetCharacterOwner()->LaunchCharacter(FVector(0,0,2), true, true);
	}
}

void UMainCharacterMovementComponent::WallrunJump(const FVector2D& input) {
	FRotator DeltaDegrees(0, WRData.Side == ESide::Left ? WallrunJumpAngle : WallrunJumpAngle * -1, 0);
	//DeltaDegrees += GetCharacterOwner()->GetActorForwardVector().Rotation();
	EndWallrun(EEndReason::Jump);
	FVector WallJumpDirection = DeltaDegrees.RotateVector(GetCharacterOwner()->GetActorForwardVector()) * 1000;
	WallJumpDirection.Z += AirJumpHeightGain;
	GetCharacterOwner()->LaunchCharacter(WallJumpDirection, true, true);
}

void UMainCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) {
	/*if (CanStartWallrun()) SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Wallruning);*/

	//Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

void UMainCharacterMovementComponent::CapsuleTouched(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, FString::Printf(TEXT("HIT %f"), (MyController->WRData.Side == ESide::LEFT ? 1.f : -1.f)));

	// DrawDebugLine(GetWorld(), GetActorLocation(), Hit.ImpactPoint, FColor::Red, false, 10.f, 5, 5.f);
}