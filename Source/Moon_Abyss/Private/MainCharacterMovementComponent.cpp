#include "MainCharacterMovementComponent.h"
#include "MyPlayerCameraManager.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "../Moon_AbyssCharacter.h"
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
	bGrappleDeployed = false;
	bCanDoWallrun = true;

	MainCharacter = Cast<AMoon_AbyssCharacter>(GetCharacterOwner());
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
	if (input.IsNearlyZero()) {
		if (isSprinting()) Sprint(false);
	}
	else {
		const FRotator YawRotation(0, PawnOwner->GetController()->GetControlRotation().Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		PawnOwner->AddMovementInput(ForwardDirection, FMath::Clamp(input.Y, -1, 1));
		PawnOwner->AddMovementInput(RightDirection, FMath::Clamp(input.X, -1, 1));
	}
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

void UMainCharacterMovementComponent::SprintingJump(const FVector2D& input) {
	bIsSprinting = false;
	GetCharacterOwner()->Jump();
}

void UMainCharacterMovementComponent::EndSliding() {
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

	if (
		(!GetWallrunSurface(WallHit, WRData.Side)) ||
		(FMath::Abs(FVector::DotProduct(WallHit.Normal, UpdatedComponent->GetRightVector())) < 0.01)
	) {
		EndWallrun(EEndReason::NoWallFound);
		StartNewPhysics(dT, Iterations);
		return;
	}

	if (!areAllConditionsMetForWallrun(false)) {
		EndWallrun(EEndReason::InvalidInput);
		StartNewPhysics(dT, Iterations);
		return;
	}
	
	FHitResult groundCheck;
	if (GetSlideSurface(groundCheck)) {

		if (this->GetWalkableFloorAngle() >= FMath::RadiansToDegrees(FMath::Acos(groundCheck.ImpactNormal.Z))) {
			EndWallrun(EEndReason::WalkableGround);
			StartNewPhysics(dT, Iterations);
			return;
		}
	}

	Velocity += FVector::DownVector * FMath::Clamp(
		(WR_GravityForce * (1 - FVector::DotProduct(FVector::UpVector, WallHit.Normal))),
		0.f,
		99999.f
	);

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()) {
		CalcVelocity(dT, WR_Friction, false, GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(dT);

	Iterations++;
	bJustTeleported = false;

	int sideMod = 0;
	if (WRData.Side == ESide::Right) sideMod = -1;
	else if (WRData.Side == ESide::Left) sideMod = 1;

	Velocity -= WallHit.Normal * WallHit.Distance * dT * WR_PullStrength ;


	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();
	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * dT;

	Velocity += UpdatedComponent->GetForwardVector() * WR_Speed * dT;
	FVector VelPlaneDir = FVector::VectorPlaneProject(Velocity, WallHit.Normal).GetUnsafeNormal();
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
	if (side == ESide::Right) End = Start + UpdatedComponent->GetRightVector() * 200;
	else if (side == ESide::Left) End = Start + UpdatedComponent->GetRightVector() * -1 * 200;
	return GetWorld()->LineTraceSingleByProfile(Hit, Start, End, TEXT("BlockAll"), SelfExcludeQueryParams);
}

void UMainCharacterMovementComponent::SlidingJump(const FVector2D& input) {
	bIsSliding = false;
	SetMovementMode(MOVE_Falling);
	Velocity = UpdatedComponent->GetForwardVector() * Velocity;
	GetCharacterOwner()->Jump();
}

void UMainCharacterMovementComponent::PhysSlide(float dT, int32 Iterations) {
	if (dT < MIN_TICK_TIME) return;
	RestorePreAdditiveRootMotionVelocity();

	FHitResult Surface;
	bool hasSlideSurfaceBeenFound = GetSlideSurface(Surface);
	if (Velocity.SizeSquared() < Slide_MinSpeed * Slide_MinSpeed) {
		EndSliding();
		StartNewPhysics(dT, Iterations);
		return;
	}
	if (hasSlideSurfaceBeenFound)  Velocity += Slide_GravityForce * FVector::DownVector * dT;
	else Velocity += FVector::UpVector * GetGravityZ() * dT;

	if (!Acceleration.IsZero() && FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > .5f) {
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
	}

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()) {
		if (hasSlideSurfaceBeenFound) CalcVelocity(dT, Slide_Friction, true, GetMaxBrakingDeceleration());
		else CalcVelocity(dT, 0.f, false, 0.f);
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
	} else SafeMoveUpdatedComponent(Adjusted, FRotationMatrix::MakeFromZX(FVector::UpVector, GetPawnOwner()->GetActorForwardVector()).ToQuat(), true, Hit);

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
	
	if (CanUncrouch()) {
		FVector launchVector = Velocity;
		launchVector.Z += 3000.f;
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
	
	if (isWallruning()) WallrunJump(input);
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
		GetCharacterOwner()->GetCapsuleComponent()->GetRelativeLocation().Z + GetCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / -2
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
			GetCharacterOwner()->GetCapsuleComponent()->GetRelativeLocation().Z + GetCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()  - GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		));
		GetCharacterOwner()->GetCapsuleComponent()->SetCapsuleHalfHeight(GetCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
			
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
	GetCharacterOwner()->GetCapsuleComponent()->SetCapsuleHalfHeight(GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2);
	Velocity += Velocity.GetSafeNormal2D() * Slide_EnterImpulse;
	SetMovementMode(MOVE_Custom, CMOVE_Sliding);
}

void UMainCharacterMovementComponent::StartAirSlide() {
	GetCharacterOwner()->GetCapsuleComponent()->SetCapsuleHalfHeight(GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2);
	Velocity += Velocity.GetSafeNormal2D() * SlideAirHorizontalBoost;
	Velocity.Z += SlideAirHeightLost;
	SetMovementMode(MOVE_Custom, CMOVE_Sliding);
}

void UMainCharacterMovementComponent::FallJump(const FVector2D& input) {
	if (bCanAirJump) {
		bCanAirJump = false;
		FVector VInMDir = GetCharacterOwner()->GetActorForwardVector().Rotation().RotateVector(FVector(input.GetSafeNormal(), 0) * (Velocity.Length()));
		
		VInMDir.SetComponentForAxis(EAxis::Z, AirJumpHeightGain);
		PendingLaunchVelocity = VInMDir;
	}
}

void UMainCharacterMovementComponent::StartWallrun(ESide Side, FVector normal) {
	if (bCanDoWallrun) {
		if (areAllConditionsMetForWallrun(true)) {
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, Acceleration.GetUnsafeNormal2D().ToString());
			if (isSprinting()) Sprint(false);
			SetMovementMode(MOVE_Custom, CMOVE_Wallruning);
			GetPawnOwner()->bUseControllerRotationYaw = false;
			WRData.WallRuning = true;
			WRData.Side = Side;
			WRData.NormalHit = normal;
		}
	}
}

void UMainCharacterMovementComponent::EndWallrun(EEndReason reason, const FHitResult* Hit) {
	FRotator JumpAngle;
	if (reason == EEndReason::NoWallFound) {
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("End Reason: NoWallFound"));
	}
	else if (reason == EEndReason::InvalidInput) {
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("End Reason: InvalidInput"));

	}
	else if (reason == EEndReason::Jump) {
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("End Reason: Jump"));

		if (WRData.Side == ESide::Left) JumpAngle = FRotator(0, WallrunJumpAngle, 0);
		else if (WRData.Side == ESide::Right) JumpAngle = FRotator(0, WallrunJumpAngle * -1, 0);
		FVector LaunchV = (WRData.NormalHit * WR_EndJumpAwayForce + FVector::UpVector * WR_JumpUpForce);
		GetCharacterOwner()->LaunchCharacter(LaunchV, false, false);
	}
	else if (reason == EEndReason::Hit) {
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("End Reason: Hit"));

		if (Hit) {
			if (FVector::DotProduct(Hit->Normal, UpdatedComponent->GetForwardVector()) < 0.1) SetMovementMode(MOVE_Walking);
			else GetCharacterOwner()->LaunchCharacter(Hit->Normal * WR_EndHitYawForce + FVector::UpVector * WR_EndHitUpForce, true, true);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(WallRunCDHandle, FTimerDelegate::CreateLambda([&] { this->bCanDoWallrun = true; }), CoolDownBetweenWallruns, false);

	bCanDoWallrun = false;
	bCanAirJump = true;
	SetMovementMode(MOVE_Falling);
	GetPawnOwner()->GetController()->SetControlRotation(GetCharacterOwner()->GetActorRotation());
	GetPawnOwner()->bUseControllerRotationYaw = true;
	WRData.WallRuning = false;
	WRData.Side = ESide::None;
}

void UMainCharacterMovementComponent::WallrunJump(const FVector2D& input) { 
	EndWallrun(EEndReason::Jump); 
}

void UMainCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) {
	/*if (CanStartWallrun()) SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Wallruning);*/

	
}

void UMainCharacterMovementComponent::CapsuleTouched(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	

	
}


void UMainCharacterMovementComponent::StartGrapple() {
	if (!bGrappleDeployed) {
		if (MainCharacter->isTargetUIOnScreen) {
			DoGrapple(MainCharacter->OutHit.GetActor()->GetActorLocation());
		} else {
			FHitResult fhr;
			APlayerCameraManager* camRef = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
			if (GetWorld()->LineTraceSingleByChannel( 
				fhr,
				camRef->GetCameraLocation(),
				camRef->GetCameraLocation() + camRef->GetCameraRotation().Vector() * 5000,
				ECC_WorldStatic,
				SelfExcludeQueryParams
			))  DoGrapple(fhr.Location);
		}
	}
}

bool UMainCharacterMovementComponent::areAllConditionsMetForWallrun(bool isStart) {
	return(
		WRData.WallRuning != isStart &&
		Velocity.SizeSquared2D() > WR_MinSpeed * WR_MinSpeed &&
		FMath::Abs(Acceleration.X) > 0.5f &&
		!isWalking() &&
		!IsMovingOnGround()
	);
}

void UMainCharacterMovementComponent::DoGrapple(const FVector& atLocation) {
	bGrappleDeployed = true;
	GrappleLocation = atLocation;
	DrawDebugLine(GetWorld(), GrappleLocation, UpdatedComponent->GetComponentLocation(), FColor::Red, false, 15.f, 0, 5);
}

void UMainCharacterMovementComponent::ContinueGrapple() {
	if (bGrappleDeployed && !isGrappling()) {
		Velocity += (GrappleLocation - UpdatedComponent->GetComponentLocation()).GetUnsafeNormal() * GrappleStartBoost;
		MaxRadius = FVector::Distance(GrappleLocation, UpdatedComponent->GetComponentLocation());
		SetMovementMode(MOVE_Custom, CMOVE_Grappling);
	}
}

void UMainCharacterMovementComponent::EndGrapple() {
	if (isGrappling()) {
		SetMovementMode(MOVE_Falling);
		GetPawnOwner()->GetController()->SetControlRotation(GetCharacterOwner()->GetActorRotation());
		MaxRadius = 0.f;
	}
}

void UMainCharacterMovementComponent::RetractGrapple() {
	if (isGrappling()) EndGrapple();
	bGrappleDeployed = false;
	GrappleLocation = FVector::ZeroVector;
	if (IsMovingOnGround())SetMovementMode(MOVE_Walking);
}

void UMainCharacterMovementComponent::PhysGrapple(float dT, int32 Iterations) {
	if (dT < MIN_TICK_TIME) return;
	FVector ToGrappleVector = GrappleLocation - UpdatedComponent->GetComponentLocation();
	if (FVector::DotProduct(Velocity.GetUnsafeNormal(), ToGrappleVector.GetUnsafeNormal()) < 0) 
		Velocity = FVector::VectorPlaneProject(Velocity, ToGrappleVector.GetUnsafeNormal() * -1);

	Velocity += FVector::DownVector * Grapple_GravityForce;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();
	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * dT;

	// #todo Sweep!!!
	if (float fqDiffRad = MaxRadius - ToGrappleVector.SquaredLength() > 0)
		GetPawnOwner()->SetActorLocation(GetPawnOwner()->GetActorLocation() + ToGrappleVector.GetUnsafeNormal() * fqDiffRad, false);

	MoveUpdatedComponent(
		Adjusted, 
		FRotationMatrix::MakeFromZX(FVector::UpVector, Velocity.GetUnsafeNormal2D()).Rotator(), 
		true, 
		&Hit
	);

	Iterations++;
	bJustTeleported = false;

	// #todo Sweep!!!
	if (float fqDiffRad = MaxRadius - ToGrappleVector.SquaredLength() > 0)
		GetPawnOwner()->SetActorLocation(GetPawnOwner()->GetActorLocation() + ToGrappleVector.GetUnsafeNormal() * fqDiffRad, false);

	if (Hit.Time < 1.f) {
		HandleImpact(Hit, dT, Adjusted);
		SlideAlongSurface(Adjusted, 1.f - Hit.Time, Hit.Normal, Hit, true);
		EndGrapple();
	} else {
		if (MaxRadius > ToGrappleVector.SquaredLength()) MaxRadius = ToGrappleVector.SquaredLength();
	}
}