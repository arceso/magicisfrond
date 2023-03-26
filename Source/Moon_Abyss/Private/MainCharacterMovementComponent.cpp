// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterMovementComponent.h"
#include "MyPlayerCameraManager.h"
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
	AirControl = 1;
	fGroundFrictionBase = 8.f;
	fGroundFrictionReduced = .5f;
	GroundFriction = fGroundFrictionBase;
	fBrackingForce = 5.f;
	fBrackingForceReduced = 0.5;
	BrakingFriction = fBrackingForce;
	bWantToEndCrouch = false;
}

void UMainCharacterMovementComponent::PhysCustom(float dT, int32 iterations) {
	Super::PhysCustom(dT, iterations);

	switch (CustomMovementMode) {
	case CMOVE_Sliding:
		PhysSlide(dT, iterations);
		break;
	case CMOVE_Wallruning:
		PhysWallrun(dT, iterations);
		break;
	case CMOVE_Grappling:
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("CustomPhys"));
		PhysGrapple(dT, iterations);
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
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2 * FVector::DownVector;
	
	return GetWorld()->LineTraceSingleByProfile(Hit, Start, End, TEXT("BlockAll"), SelfExcludeQueryParams);

}

bool UMainCharacterMovementComponent::GetGroundSurface(FHitResult& Hit) {
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
	
	if (MovementMode == MOVE_Falling) Velocity += FVector::UpVector * GetGravityZ() * DeltaTime;
	if (bWantToEndCrouch && CanUncrouch()) {
		if (isCrouching()) EndCrouching();
		else if (isSliding()) EndSliding();
	}
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

	GetCharacterOwner()->GetCapsuleComponent()->SetCapsuleHalfHeight(GetCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	if (CanUncrouch()) {
		SetMovementMode(MOVE_Walking); 
	}
	else {
		bWantToEndCrouch = true;
		StartCrouching();
	}
}

bool UMainCharacterMovementComponent::CanUncrouch() {
	DrawDebugCapsule(GetWorld(), UpdatedComponent->GetComponentLocation() + FVector::UpVector * GetCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), GetCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), GetCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleRadius(), UpdatedComponent->GetComponentQuat(), FColor::Blue, false, 10.f, 0, 1.f);
	return !OverlapTest(UpdatedComponent->GetComponentLocation() + FVector::UpVector * (GetCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), UpdatedComponent->GetComponentQuat(), ECollisionChannel::ECC_WorldStatic, GetCharacterOwner()->GetCapsuleComponent()->GetCollisionShape(), GetCharacterOwner());
}

void UMainCharacterMovementComponent::PhysWallrun(float dT, int32 Iterations) {
	if (dT < MIN_TICK_TIME) return;
	RestorePreAdditiveRootMotionVelocity();
	FHitResult WallHit;

	if (!GetWallrunSurface(WallHit, WRData.Side)) {
		EndWallrun(EEndReason::NoWallFound);
		StartNewPhysics(dT, Iterations);
		return;
	}

	if (FMath::Abs(FVector::DotProduct(WallHit.Normal, UpdatedComponent->GetRightVector())) < 0.01) {
		EndWallrun(EEndReason::NoWallFound);
		StartNewPhysics(dT, Iterations);
		return;
	}

	//if (WallHit.Distance < 80.f) GetPawnOwner()->SetActorLocation(GetPawnOwner()->GetActorLocation() + (WallHit.Normal * (80.f - WallHit.Distance)));

	if (Velocity.SizeSquared2D() < WR_MinSpeed * WR_MinSpeed) {
		EndWallrun(EEndReason::InvalidInput);
		StartNewPhysics(dT, Iterations);
		return;
	}

	//float angleOfSurface = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FVector::UpVector, WallHit.Normal) / (FVector::UpVector.Size() * WallHit.Normal.Size())));
	FHitResult groundCheck;
	if (GetSlideSurface(groundCheck)) {

		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString::Printf(TEXT("Angle: %f."), FMath::RadiansToDegrees(FMath::Acos(groundCheck.ImpactNormal.Z))));
		if (this->GetWalkableFloorAngle() >= FMath::RadiansToDegrees(FMath::Acos(groundCheck.ImpactNormal.Z))) {
			EndWallrun(EEndReason::WalkableGround);
			StartNewPhysics(dT, Iterations);
			return;
		}
	}

	Velocity += FVector::DownVector * (WR_GravityForce * (1 - FVector::DotProduct(FVector::UpVector, WallHit.Normal)));
	
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()) {
		CalcVelocity(dT, WR_Friction, false, GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(dT);

	Iterations++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();
	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * dT;

	FVector VelPlaneDir = FVector::VectorPlaneProject(Velocity, WallHit.Normal).GetUnsafeNormal();
	int sideMod = 0;
	if (WRData.Side == ESide::Right) sideMod = -1;
	else if (WRData.Side == ESide::Left) sideMod = 1;
		WRData.Side == ESide::Right ? -1 : 1;
	FQuat NewRotation = FRotationMatrix::MakeFromZY(FVector::UpVector, WallHit.Normal * sideMod).ToQuat();

	SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);

	if (Hit.Time < 1.f) {
		HandleImpact(Hit, dT, Adjusted);
		SlideAlongSurface(Adjusted, 1.f - Hit.Time, Hit.Normal, Hit, true);
	}

	FHitResult newSurfaceHit;
	
	if (!GetWallrunSurface(newSurfaceHit, WRData.Side)) {
		EndWallrun(EEndReason::NoWallFound);
	} else if (Velocity.SizeSquared2D() < WR_MinSpeed * WR_MinSpeed) {
		EndWallrun(EEndReason::InvalidInput);
	} else WRData.NormalHit = newSurfaceHit.Normal;
}

bool UMainCharacterMovementComponent::GetWallrunSurface(FHitResult& Hit, ESide side) {
	FVector Start = UpdatedComponent->GetComponentLocation() + UpdatedComponent->GetForwardVector() * 10;
	FVector End;
	if (side == ESide::Right) End = Start + UpdatedComponent->GetRightVector() * 200;//CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius() * 2;
	else if (side == ESide::Left) End = Start + UpdatedComponent->GetRightVector() * -1 * 200;//CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius() * 2;
	//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 5.f, 0, 4.f);
	return GetWorld()->LineTraceSingleByProfile(Hit, Start, End, TEXT("BlockAll"), SelfExcludeQueryParams);
	//return GetWorld()->SweepSingleByProfile(Hit, Start, End, UpdatedComponent->GetComponentRotation().Quaternion(), TEXT("BlockAll"), GetCharacterOwner()->GetCapsuleComponent()->GetCollisionShape(), SelfExcludeQueryParams);
}

void UMainCharacterMovementComponent::SlidingJump(const FVector2D& input) {
	bIsSliding = false;
	SetMovementMode(MOVE_Falling);
	GetCharacterOwner()->Jump();
}

void UMainCharacterMovementComponent::PhysSlide(float dT, int32 Iterations) {
	if (dT < MIN_TICK_TIME) return;
	RestorePreAdditiveRootMotionVelocity();

	FHitResult Surface;
	GetSlideSurface(Surface);
	if (Velocity.SizeSquared() < Slide_MinSpeed * Slide_MinSpeed) {
		EndSliding();
		StartNewPhysics(dT, Iterations);
		return;
	}

	Velocity += Slide_GravityForce * FVector::DownVector * dT;

	if (FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > .5f) {
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
	} else Acceleration = FVector::ZeroVector;

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()) {
		CalcVelocity(dT, Slide_Friction, true, GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(dT);

	Iterations++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();
	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * dT;
	
	if (Surface.bBlockingHit) {
		FVector VelPlaneDir = FVector::VectorPlaneProject(Velocity, Surface.Normal).GetSafeNormal();
		FQuat NewRotation = FRotationMatrix::MakeFromXZ(VelPlaneDir, Surface.Normal).ToQuat();
		SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);
	}
	else {
		SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentRotation(), true, Hit);
	}

	if (Hit.Time < 1.f) {
		HandleImpact(Hit, dT, Adjusted);
		SlideAlongSurface(Adjusted, 1.f - Hit.Time, Hit.Normal, Hit, true);
	}
	FHitResult newSurfaceHit;
	GetSlideSurface(newSurfaceHit);
	if (Velocity.SizeSquared() < Slide_MinSpeed * Slide_MinSpeed) EndSliding();
	
	if(!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()) {
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / dT;
	}
}

void UMainCharacterMovementComponent::CrouchingJump(const FVector2D& input) {
	//Magic Numbers
	if (CanUncrouch()) {
		FVector launchVector = Velocity;
		launchVector.Z += 75000.f;
		GetCharacterOwner()->LaunchCharacter(launchVector, true, true);
	}
}

void UMainCharacterMovementComponent::GrapplingJump(const FVector2D& input)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("IMPLEMENT GrapplingJump, DUMBASS!"));
}

void UMainCharacterMovementComponent::Move(const FVector2D& input) {
	if (isWalking()) Walking(input);
	else if (isFalling()) Falling(input);
	else if (isSprinting()) Sprinting(input);
	else if (isWallruning()) Wallruning(input);
	else if (isCrouching()) Crouching(input);
	else if (isGrappling()) Grappling(input);
	//else if (isSliding()) Sliding(input);
}

void UMainCharacterMovementComponent::Wallruning(const FVector2D& input) {
	if (input.IsNearlyZero()) {
		if (isSprinting()) Sprint(false);
	}
	else {
		PawnOwner->AddMovementInput(UpdatedComponent->GetForwardVector(), input.Y);
		PawnOwner->AddMovementInput(UpdatedComponent->GetRightVector(), input.X);
	}
}

void UMainCharacterMovementComponent::Jump(const FVector2D& input) {
	
	if (isWallruning()) {
		WallrunJump(input);
	}
	else if (isSprinting()) SprintingJump(input);
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
bool UMainCharacterMovementComponent::isCrouching() const { return bIsCrouching; }
bool UMainCharacterMovementComponent::isGrappling() const { return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Grappling; }
bool UMainCharacterMovementComponent::isSprinting() const { return MovementMode == MOVE_Walking && bIsSprinting; }
bool UMainCharacterMovementComponent::isWalking() const { return MovementMode == MOVE_Walking && !isSprinting() && !isCrouching(); }
bool UMainCharacterMovementComponent::isFalling() const { return MovementMode == MOVE_Falling; }

void UMainCharacterMovementComponent::WalkJump(const FVector2D& input) {
	if(isSprinting()) Sprint(false);
	GetCharacterOwner()->Jump();
}

void UMainCharacterMovementComponent::StartCrouching() {
	GetCharacterOwner()->GetCapsuleComponent()->SetRelativeLocation(FVector(
		GetCharacterOwner()->GetCapsuleComponent()->GetRelativeLocation().X,		
		GetCharacterOwner()->GetCapsuleComponent()->GetRelativeLocation().Y,
		GetCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / -2
	));
	GetCharacterOwner()->GetCapsuleComponent()->SetCapsuleHalfHeight(GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2);
	MaxWalkSpeed /= 2.f;
	bIsCrouching = true;
}

void UMainCharacterMovementComponent::EndCrouching() {
	if (!CanUncrouch()) bWantToEndCrouch = true;
	else {
		bWantToEndCrouch = false;
		bIsCrouching = false;
		GetCharacterOwner()->GetCapsuleComponent()->SetRelativeLocation(FVector(
			GetCharacterOwner()->GetCapsuleComponent()->GetRelativeLocation().X,
			GetCharacterOwner()->GetCapsuleComponent()->GetRelativeLocation().Y,
			GetCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()  - GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		));
		GetCharacterOwner()->GetCapsuleComponent()->SetCapsuleHalfHeight(GetCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
			//SetCapsuleHalfHeight(GetCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		MaxWalkSpeed *= 2.f;
		Super::UnCrouch(true);
		CharacterOwner->bIsCrouched = false;
	}
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
	//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Purple, TEXT("Ground Slide started"));
	//GroundFriction = fGroundFrictionReduced;
	//BrakingFriction = fBrackingForceReduced;
	//PendingLaunchVelocity = GetCharacterOwner()->GetActorForwardVector() * fGroundSlideForce;
	//bIsSliding = true;
	GetCharacterOwner()->GetCapsuleComponent()->SetCapsuleHalfHeight(GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2);
	Velocity += Velocity.GetSafeNormal2D() * Slide_EnterImpulse;
	SetMovementMode(MOVE_Custom, CMOVE_Sliding);
}

void UMainCharacterMovementComponent::StartAirSlide() {
	////GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Purple, TEXT("Air Slide started"));
	//FVector ForwardAndSightlyDownwards = Velocity * UpdatedComponent->GetForwardVector();
	//float ZVelocity = ForwardAndSightlyDownwards.Z;
	//ForwardAndSightlyDownwards.Z = 0;
	//ForwardAndSightlyDownwards *= fAirSlideForce;
	// 
	//ForwardAndSightlyDownwards.Z = ZVelocity - 750.f;

	////GroundFriction = fGroundFrictionReduced;
	////BrakingFriction = fBrackingForceReduced;
	////PendingLaunchVelocity = GetCharacterOwner()->GetActorForwardVector() * fGroundSlideForce;
	////bIsSliding = true;
	//Velocity += Velocity.GetSafeNormal2D() * ForwardAndSightlyDownwards;
	//Velocity.SetComponentForAxis(EAxis::Z, Velocity.Z - 750.f);
	GetCharacterOwner()->GetCapsuleComponent()->SetCapsuleHalfHeight(GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2);
	Velocity += Velocity.GetSafeNormal2D() * Slide_EnterImpulse;
	SetMovementMode(MOVE_Custom, CMOVE_Sliding);
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

void UMainCharacterMovementComponent::StartWallrun(ESide Side, FVector normal) {
	//GEngine->AddOnScreenDebugMessage(-1,5,FColor::Red,TEXT("StartWallrun"));

	if (
		!WRData.WallRuning && 
		Velocity.SizeSquared2D() > WR_MinSpeed * WR_MinSpeed &&
		!isWalking() &&
		!IsMovingOnGround()
	) {
		if(isSprinting()) Sprint(false);
		
		SetMovementMode(MOVE_Custom, CMOVE_Wallruning);
		GetPawnOwner()->bUseControllerRotationYaw = false;
		WRData.WallRuning = true;
		WRData.Side = Side;
		WRData.NormalHit = normal;
		Velocity += Velocity.GetSafeNormal2D() * WR_EnterImpulse;
	}
}

void UMainCharacterMovementComponent::EndWallrun(EEndReason reason, const FHitResult* Hit) {
	bCanAirJump = true;

	//GetPawnOwner()->GetController()->SetControlRotation(GetPawnOwner()->GetActorRotation());
	SetMovementMode(MOVE_Falling);
	GetPawnOwner()->bUseControllerRotationYaw = true;

	FRotator JumpAngle;

	switch (reason) {
	case EEndReason::NoWallFound:
	case EEndReason::InvalidInput:
		// Do nothing
		break;
	case EEndReason::Jump:
		if (WRData.Side == ESide::Left) JumpAngle = FRotator(0, WallrunJumpAngle, 0);
		else if (WRData.Side == ESide::Right) JumpAngle = FRotator(0, WallrunJumpAngle * -1, 0);
		GetCharacterOwner()->LaunchCharacter((WRData.NormalHit * WR_EndJumpAwayForce + FVector::UpVector * WR_JumpUpForce), true, true);
		break;
	case EEndReason::Hit:
		if (Hit) {
			if (FVector::DotProduct(Hit->Normal, UpdatedComponent->GetForwardVector()) < 0.1) SetMovementMode(MOVE_Walking);
			else GetCharacterOwner()->LaunchCharacter(Hit->Normal * WR_EndHitYawForce + FVector::UpVector * WR_EndHitUpForce, true, true);
		}
		break;
	default:
		break;
	};
	WRData.WallRuning = false;
	WRData.Side = ESide::None;
}

void UMainCharacterMovementComponent::WallrunJump(const FVector2D& input) { 
	EndWallrun(EEndReason::Jump); 
}

void UMainCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) {
	/*if (CanStartWallrun()) SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Wallruning);*/

	//Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

void UMainCharacterMovementComponent::CapsuleTouched(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("HIT %f"));

	// DrawDebugLine(GetWorld(), GetActorLocation(), Hit.ImpactPoint, FColor::Red, false, 10.f, 5, 5.f);
}


void UMainCharacterMovementComponent::StartGrapple() {
	FHitResult fhr;

	APlayerCameraManager* camRef = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;


	if (GetWorld()->LineTraceSingleByChannel(
		fhr,
		camRef->GetCameraLocation(),
		camRef->GetCameraLocation() + camRef->GetCameraRotation().Vector() * 5000,
		ECC_WorldStatic,
		SelfExcludeQueryParams
	)) {
		DrawDebugLine(GetWorld(), fhr.Location, UpdatedComponent->GetComponentLocation(), FColor::Red, false, 15.f, 0, 5);
		GrappleLocation = fhr.Location;

		Velocity += (fhr.Location - UpdatedComponent->GetComponentLocation()).GetUnsafeNormal() * 500;

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("StartGrapple"));
		SetMovementMode(MOVE_Custom, CMOVE_Grappling);
	}
}

void UMainCharacterMovementComponent::EndGrapple() {
	SetMovementMode(MOVE_Falling);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("End Grapple"));
}

void UMainCharacterMovementComponent::PhysGrapple(float dT, int32 Iterations) {
	//EndGrapple();
	if (dT < MIN_TICK_TIME) return;
	RestorePreAdditiveRootMotionVelocity();

	constexpr float grappleSpeed = 2500;

	FVector ToGrappleVector = GrappleLocation - UpdatedComponent->GetComponentLocation();

	float LookAndGrappleSimmilarity = FVector::DotProduct(UpdatedComponent->GetForwardVector(), ToGrappleVector.GetUnsafeNormal());

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("LookAndGrappleSimmilarity: %f"), LookAndGrappleSimmilarity));

	if(LookAndGrappleSimmilarity < 0) {
		EndGrapple();
		StartNewPhysics(dT, Iterations);
		return;
	} else {
		Velocity += FMath::VInterpTo(
			UpdatedComponent->GetForwardVector(), 
			ToGrappleVector.GetUnsafeNormal(), 
			LookAndGrappleSimmilarity, 1.f
		) * grappleSpeed * dT;
	}

	//Velocity += Slide_GravityForce * FVector::DownVector * dT;

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()) {
		CalcVelocity(dT, Slide_Friction, true, GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(dT);

	Iterations++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();
	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * dT;

	
	//FVector VelPlaneDir = FVector::VectorPlaneProject(Velocity, Surface.Normal).GetSafeNormal();
	//FQuat NewRotation = FRotationMatrix::MakeFromXZ(VelPlaneDir, Surface.Normal).ToQuat();
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentRotation(), true, Hit);
	
}