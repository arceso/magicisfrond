// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "CustomMovementFlags.h"


void UMainCharacterMovementComponent::BeginPlay(){
	Super::BeginPlay();
	SelfExcludeQueryParams.AddIgnoredActor(GetOwner());
	SetMovementMode(EMovementMode::MOVE_Walking);
	WRData.WallRuning = false;
	WRData.Side = ESide::NONE;
	bCanAirJump = true;
	SetPlaneConstraintNormal(FVector(0, 0, 1));
}

void UMainCharacterMovementComponent::Walking(const FVector2D& input) {
	const FRotator YawRotation(0, PawnOwner->GetController()->GetControlRotation().Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	PawnOwner->AddMovementInput(ForwardDirection, input.Y);
	PawnOwner->AddMovementInput(RightDirection, input.X);
}

void UMainCharacterMovementComponent::Falling(const FVector2D& input) {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("Falling"));
}

void UMainCharacterMovementComponent::Sliding(const FVector2D& input) {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("Sliding"));
}


void UMainCharacterMovementComponent::Crouching(const FVector2D& input) {
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("Crouching"));
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

void UMainCharacterMovementComponent::WalkJump(const FVector2D& input) {
	GetCharacterOwner()->Jump();
}
void UMainCharacterMovementComponent::FallJump(const FVector2D& input) {
	if (bCanAirJump) {
		bCanAirJump = false;
		FVector VInMDir = FVector(input.GetSafeNormal(), 1) * (Velocity.Length() * .5);
		// Magic Numbers
		VInMDir.Z += 500;
		//FVector finalV = GetControlRotation().RotateVector(VInMDir);
		GetCharacterOwner()->LaunchCharacter(VInMDir, true, true);
	}
}
void UMainCharacterMovementComponent::Wallruning(const FVector2D& input) {
	// Magic Numbers
	const FVector Start = GetActorLocation() + (GetPawnOwner()->GetActorForwardVector() * Velocity.Length() * GetWorld()->GetDeltaSeconds());
	const FVector End = Start + WRData.NormalHit * -1 * 42 * 2;

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 10, 0, 10);

	bool isWallFound = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, SelfExcludeQueryParams);
	if (isWallFound) {
		WRData.NormalHit = OutHit.ImpactNormal;
		if (input.Y >= .5f) {
			if (WRData.Side == ESide::RIGHT && input.X > .0f) Wallrun(ESide::RIGHT, OutHit);
			else if (WRData.Side == ESide::LEFT && input.X < .0f) Wallrun(ESide::LEFT, OutHit);
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
		//camera->SetCameraSide(Side == ESide::LEFT ? ESide::RIGHT : ESide::LEFT);
	}
}

void UMainCharacterMovementComponent::Wallrun(ESide side, FHitResult fhr) {
	StopMovementKeepPathing();
	float rot = MAX_ROTATION * GetWorld()->GetDeltaSeconds() * SPEED;
	if (!fhr.ImpactNormal.IsZero()) {
		const int SideValue = (side == ESide::RIGHT ? -1 : 1);
		FVector NewForward = fhr.ImpactNormal.Cross(FVector(0, 0, SideValue));
		FRotator NewRotation = NewForward.Rotation();// *MAX_ROTATION* GetWorld()->GetDeltaSeconds(); //(NewForward.Rotation()) - GetActorForwardVector().Rotation();
		FRotator RotDifference = NewRotation.Clamp() - GetOwner()->GetActorForwardVector().Rotation().Clamp();

		if (RotDifference.Yaw > 300.f) RotDifference = NewRotation.Clamp() - FRotator(0, GetOwner()->GetActorForwardVector().Rotation().Clamp().Yaw + 360, 0);
		else if (RotDifference.Yaw < -300.f) RotDifference = NewRotation.Clamp() - FRotator(0, GetOwner()->GetActorForwardVector().Rotation().Clamp().Yaw - 360, 0);


		GetOwner()->AddActorLocalRotation(RotDifference * rot);

		NewForward *= SPEED * GetWorld()->GetDeltaSeconds();

		if (fhr.Distance > MAX_DISTANCE) NewForward += fhr.ImpactNormal * -1 * (fhr.Distance - MAX_DISTANCE); // If too far get closer
		else if (fhr.Distance < MIN_DISTANCE)NewForward += fhr.ImpactNormal * (MIN_DISTANCE - fhr.Distance);  // If too close get further away

		GetOwner()->SetActorLocation(GetActorLocation() + NewForward, true);
	}
	else GetOwner()->SetActorLocation(GetActorLocation() + (GetOwner()->GetActorForwardVector() * SPEED * GetWorld()->GetDeltaSeconds()), true);
}

void UMainCharacterMovementComponent::UpdateWallrun(FVector_NetQuantizeNormal* newNormal) {
	WRData.NormalHit = *newNormal;
}

void UMainCharacterMovementComponent::EndWallrun(EEndReason reason) {
	WRData.WallRuning = false;
	WRData.Side = ESide::NONE;

	SetMovementMode(MOVE_Falling);

	bConstrainToPlane = false;
	bCanAirJump = true;
	GetPawnOwner()->GetController()->SetControlRotation(GetPawnOwner()->GetActorRotation());
	GetPawnOwner()->bUseControllerRotationYaw = true;

	if (reason == EEndReason::Jump) {}
	else if (reason == EEndReason::NoWallFound) {}
	else if (reason == EEndReason::Hit) {}
}

void UMainCharacterMovementComponent::WallrunJump(const FVector2D& input) {
	EndWallrun(EEndReason::Jump);
	GetCharacterOwner()->LaunchCharacter((
		GetCharacterOwner()->GetActorForwardVector() * JUMP_INFLUENCE.X +
		//WRData.NormalHit * JUMP_INFLUENCE.Y +
		FVector(0, 0, 1) * JUMP_INFLUENCE.Z
		) * JUMP_FORCE, true, true);
}

void UMainCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) {
	/*if (CanStartWallrun()) SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Wallruning);*/

	//Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

void UMainCharacterMovementComponent::CapsuleTouched(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, FString::Printf(TEXT("HIT %f"), (MyController->WRData.Side == ESide::LEFT ? 1.f : -1.f)));

	// DrawDebugLine(GetWorld(), GetActorLocation(), Hit.ImpactPoint, FColor::Red, false, 10.f, 5, 5.f);
}