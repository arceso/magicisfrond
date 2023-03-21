// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
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
	fGroundFrictionBase = 8.f;
	fGroundFrictionReduced = .5f;
	GroundFriction = fGroundFrictionBase;
	fBrackingForce = 5.f;
	fBrackingForceReduced = 0.5;
	BrakingFriction = fBrackingForce;

}

void UMainCharacterMovementComponent::PhysCustom(float dT, int32 iterations) {
	Super::PhysCustom(dT, iterations);

	switch (CustomMovementMode) {
	case CMOVE_Sliding:
		PhysSlide(dT, iterations);
		break;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"));
		break;
	}
}


bool UMainCharacterMovementComponent::IsMovingOnGround() const {
	return Super::IsMovingOnGround() || isSliding();
}

bool UMainCharacterMovementComponent::CanCrouchInCurrentState() const {
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

void UMainCharacterMovementComponent::Walking(const FVector2D& input) {
	if (input.IsNearlyZero()) {
		if (isSprinting()) Sprint(false);
	} else {
		const FRotator YawRotation(0, PawnOwner->GetController()->GetControlRotation().Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		PawnOwner->AddMovementInput(ForwardDirection, input.Y);
		PawnOwner->AddMovementInput(RightDirection, input.X);
	}
}

bool UMainCharacterMovementComponent::GetSlideSurface(FHitResult& Hit){
	FVector
		Start = UpdatedComponent->GetComponentLocation(),
		End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2 * FVector::DownVector;
	return GetWorld()->LineTraceSingleByProfile(Hit, Start, End, TEXT("BlockAll"), SelfExcludeQueryParams);
}

void UMainCharacterMovementComponent::Falling(const FVector2D& input) {
	////if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("Falling"));
	if (isSprinting()) Sprint(false);
	const FRotator YawRotation(0, PawnOwner->GetController()->GetControlRotation().Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	PawnOwner->AddMovementInput(ForwardDirection, input.Y * AirInfluenceControl);
	PawnOwner->AddMovementInput(RightDirection, input.X * AirInfluenceControl);
}

void UMainCharacterMovementComponent::Sliding(const FVector2D& input) {
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, TEXT("SLIDING"));
	//Walking(input);
}


void UMainCharacterMovementComponent::Crouching(const FVector2D& input) {
	Walking(input);
}

void UMainCharacterMovementComponent::Sprinting(const FVector2D& input) {

	Walking(input);

}

void UMainCharacterMovementComponent::Grappling(const FVector2D& input) {
	Walking(input);
}


void UMainCharacterMovementComponent::AddInputVector(FVector WorldAccel, bool bForce /*=false*/) {
	if (PawnOwner && !WorldAccel.IsZero()) {
		PawnOwner->Internal_AddMovementInput(WorldAccel, bForce);
	}
}


void UMainCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (MovementMode == MOVE_Falling) Velocity.Z += this->GetGravityZ() * DeltaTime;
};


void UMainCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) {
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	PreviousMovementMode;
}

void UMainCharacterMovementComponent::SprintingJump(const FVector2D& input)
{
	bIsSprinting = false;
	GetCharacterOwner()->Jump();
}

void UMainCharacterMovementComponent::EndSliding() {
	//bIsSliding = false;
	//if (GroundFriction == fGroundFrictionReduced) GroundFriction = fGroundFrictionBase;
	//if (BrakingFriction == fBrackingForceReduced) BrakingFriction = fBrackingForce;

	FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, true, Hit);
	SetMovementMode(MOVE_Walking);
}

void UMainCharacterMovementComponent::SlidingJump(const FVector2D& input) {
	bIsSliding = false;
	GetCharacterOwner()->Jump();
}

void UMainCharacterMovementComponent::PhysSlide(float deltaT, int32 Iterations)
{
	if (deltaT < MIN_TICK_TIME) return;
	RestorePreAdditiveRootMotionVelocity(); // If animations add movement to root component or smth like that. mb coment it out.

	FHitResult Surface;
	if (!GetSlideSurface(Surface) || Velocity.SizeSquared() < Slide_MinSpeed * Slide_MinSpeed) {
		EndSliding();
		StartNewPhysics(deltaT, Iterations);
		return;
	}

	Velocity += Slide_GravityForce * FVector::DownVector * deltaT;

	if (FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > .5f) {
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
	} else Acceleration = FVector::ZeroVector;

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()) {
		CalcVelocity(deltaT, Slide_Friction, true, GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(deltaT);

	Iterations++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();
	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * deltaT;
	FVector VelPlaneDir = FVector::VectorPlaneProject(Velocity, Surface.Normal).GetSafeNormal();
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(VelPlaneDir, Surface.Normal).ToQuat();
	SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);

	if (Hit.Time < 1.f) {
		HandleImpact(Hit, deltaT, Adjusted);
		SlideAlongSurface(Adjusted, 1.f - Hit.Time, Hit.Normal, Hit, true);
	}
	FHitResult newSurfaceHit;
	if (!GetSlideSurface(newSurfaceHit) || Velocity.SizeSquared() < Slide_MinSpeed * Slide_MinSpeed) EndSliding();
	
	if(!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()) {
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaT;
	}
}

void UMainCharacterMovementComponent::CrouchingJump(const FVector2D& input) {
	FVector launchVector = Velocity;
	launchVector.Z += 750.f;
	PendingLaunchVelocity = launchVector;
}

void UMainCharacterMovementComponent::GrapplingJump(const FVector2D& input)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void UMainCharacterMovementComponent::Move(const FVector2D& input) {
	if (isWalking()) Walking(input);
	else if (isFalling()) Falling(input);
	else if (isSprinting()) Sprinting(input);
	else if (isWallruning()) Wallruning(input);
	else if (isCrouching()) Crouching(input);
	else if (isGrappling()) Grappling(input);
	else if (isSliding()) Sliding(input);
}

void UMainCharacterMovementComponent::Jump(const FVector2D& input) {
	
		if (isWallruning()) WallrunJump(input);
		if (isSprinting()) SprintingJump(input);
		else if (isSliding()) SlidingJump(input);
		else if (isCrouching()) CrouchingJump(input);
		else if (isGrappling()) GrapplingJump(input);
		else if (isWalking()) WalkJump(input);
		else if (isFalling()) FallJump(input);
}

void UMainCharacterMovementComponent::Crouch(bool bStart) {
	if (!isSliding()) {
		if (bStart) {
			if (isSprinting()) StartGroundSlide();
			else if (isFalling()) StartAirSlide();
			else if (isWallruning()) { EndWallrun(EEndReason::Jump); StartAirSlide(); }
			else if (isWalking()) StartCrouching();
		} else if (isCrouching()) EndCrouching();
	} else if (!bStart) EndSliding();
}

/// STATE GETTERS
bool UMainCharacterMovementComponent::isSliding() const { return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Sliding; }
bool UMainCharacterMovementComponent::isWallruning() const { return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Wallruning; }
bool UMainCharacterMovementComponent::isCrouching() const { return MovementMode == MOVE_Walking && bIsCrouching; }
bool UMainCharacterMovementComponent::isGrappling() const { return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Grappling; }
bool UMainCharacterMovementComponent::isSprinting() const { return MovementMode == MOVE_Walking && bIsSprinting; }
bool UMainCharacterMovementComponent::isWalking() const { return MovementMode == MOVE_Walking && !bIsSprinting && !bIsSliding && !bIsCrouching; }
bool UMainCharacterMovementComponent::isFalling() const { return MovementMode == MOVE_Falling && !bIsSliding; }

void UMainCharacterMovementComponent::WalkJump(const FVector2D& input) {
	if(isSprinting()) Sprint(false);
	GetCharacterOwner()->Jump();
}

void UMainCharacterMovementComponent::StartCrouching() {
	GetCharacterOwner()->GetCapsuleComponent()->SetCapsuleHalfHeight(GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2);
	MaxWalkSpeed /= 2.f;
	bIsCrouching = true;
}

void UMainCharacterMovementComponent::EndCrouching() {
	bIsCrouching = false;
	GetCharacterOwner()->GetCapsuleComponent()->SetCapsuleHalfHeight(GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2);
	MaxWalkSpeed *= 2.f;
}

void UMainCharacterMovementComponent::Sprint(bool bStart) {
	if (isWalking() && Velocity.IsNearlyZero()) return;
	if (isSprinting()) {
		bIsSprinting = false;
		MaxWalkSpeed = fWalkMaxSpeed;
	} else if (isWalking() || isCrouching()) {
		bIsSprinting = true;
		MaxWalkSpeed = fSprintMaxSpeed;
	} 
}

void UMainCharacterMovementComponent::StartGroundSlide() {
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Purple, TEXT("Slide started"));
	//GroundFriction = fGroundFrictionReduced;
	//BrakingFriction = fBrackingForceReduced;
	//PendingLaunchVelocity = GetCharacterOwner()->GetActorForwardVector() * fGroundSlideForce;
	//bIsSliding = true;
	Velocity += Velocity.GetSafeNormal2D() * Slide_EnterImpulse;
	SetMovementMode(MOVE_Custom, CMOVE_Sliding);
}

void UMainCharacterMovementComponent::StartAirSlide() {
	bIsSliding = true;
	FVector ForwardAndSightlyDownwards = Velocity;
	float ZVelocity = ForwardAndSightlyDownwards.Z;
	ForwardAndSightlyDownwards.Z = 0;
	ForwardAndSightlyDownwards *= fAirSlideForce;
	ForwardAndSightlyDownwards.Z = ZVelocity - 750.f;

	PendingLaunchVelocity = ForwardAndSightlyDownwards;
}

void UMainCharacterMovementComponent::FallJump(const FVector2D& input) {
	if (bCanAirJump) {
		bCanAirJump = false;
		FVector VInMDir = GetCharacterOwner()->GetActorForwardVector().Rotation().RotateVector(FVector(input.GetSafeNormal(), 0) * (Velocity.Length()));
		// Magic Numbers
		VInMDir.Z += AirJumpHeightGain;
		PendingLaunchVelocity = VInMDir;
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
		if(isSprinting()) Sprint(false);
		// If not set, beautiful things happens. Should look into it.
		SetMovementMode(MOVE_Custom, CMOVE_Wallruning);
		bConstrainToPlane = true;
		GetPawnOwner()->bUseControllerRotationYaw = false;
		WRData.WallRuning = true;
		WRData.Side = Side;
		WRData.NormalHit = normal;
		//GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Cyan, TEXT("PAesequefunshiona"));
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
		// Pos has saltao, que quers que te diga, Novita.
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
	////GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, FString::Printf(TEXT("HIT %f"), (MyController->WRData.Side == ESide::LEFT ? 1.f : -1.f)));

	// DrawDebugLine(GetWorld(), GetActorLocation(), Hit.ImpactPoint, FColor::Red, false, 10.f, 5, 5.f);
}