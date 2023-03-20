// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCamera.h"
#include "GameFramework/SpringArmComponent.h"

UPlayerCamera::UPlayerCamera() {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetComponentTickEnabled(true);
	bAutoRegister = true;
	bAutoActivate = true;

	//fTimeToChangeSide = 1.0f;
	//fAccTimeToChangeSide = 0.0f;
	//fAccTimeToChangeHeight = 0.0f;
	bCameraUpdated = false;
	bCameraUpdated = false;
	SweepShape.SetSphere(15.f);
	mMode = ECameraMode::Center;

	CameraPositions.Add(FCameraPosition("Center"));
	CameraPositions.Add(FCameraPosition("Offset"));
	CameraPositions.Add(FCameraPosition("CloseUp"));
	CameraPositions.Add(FCameraPosition("Scripted"));
}

void UPlayerCamera::BeginPlay() {
	Super::BeginPlay();
	if (!CameraPositions.IsEmpty()) {
		TargetCameraPosition = CameraPositions[static_cast<int>(ECameraMode::Center)].Position;
	}
	CurrentCameraPosition = TargetCameraPosition;
}

void UPlayerCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!CameraPositions.IsEmpty()) {
		FCameraPosition& Pos = CameraPositions[static_cast<int>(mMode)];
		if (bCameraUpdated) baseTransition(DeltaTime, Pos.fAccTimeToLerp, Pos.fTimeToLerp, Pos.GetPosition(CameraSide));
	}
	SetToValidTargetPosition();

	SetRelativeLocation(TargetCameraPosition);
	SetRelativeRotation(TargetCameraRotation);
}

void UPlayerCamera::SetRelPosToProperSide(float DeltaT) {
	if (fAccTimeToChangeSide + DeltaT > 1.f) fAccTimeToChangeSide = 1.f;
	else fAccTimeToChangeSide += DeltaT;
	if (fAccTimeToChangeSide >= 1.f) bCameraUpdated = false;

	TargetCameraPosition.Y = FMath::Lerp(
		TargetCameraPosition.Y, 
		CurrentCameraPosition.Y,
		fAccTimeToChangeSide / 1.f
	);
}

void UPlayerCamera::SetToValidTargetPosition() {
	constexpr float BIAS_DISTANCE = 20.f;
	FHitResult TraceHit;
	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), false, GetOwner());
	if (GetWorld()->LineTraceSingleByChannel(TraceHit, GetOwner()->GetActorLocation(), GetOwner()->GetTransform().TransformPosition(TargetCameraPosition), ECollisionChannel::ECC_WorldStatic, TraceParams)) {
		TargetCameraPosition.X = ((TraceHit.Location - GetOwner()->GetActorLocation()).Length() - BIAS_DISTANCE) * -1;
	}
}

void UPlayerCamera::HandleInput(FVector2D input) {
	bCameraUpdated = true;
	TargetCameraRotation = (GetRelativeRotation() + FRotator(input.Y * -1.f, 0.f, 0.f));
}

void UPlayerCamera::CameraSideChanged(ESide oldSide, ESide newSide) {
	bCameraUpdated = true;
	CurrentCameraPosition.Y *= -1;
	CameraPositions[static_cast<int>(mMode)].fAccTimeToLerp = 0.f;
}

void UPlayerCamera::SetCameraSide(ESide Side) {
	if (Side != CameraSide) CameraSideChanged(CameraSide, CameraSide = Side);
}

ESide UPlayerCamera::GetCameraSide() { 
	return CameraSide; 
}

// Called when CameraMode is updated
void UPlayerCamera::CameraModeChanged(ECameraMode oldMode, ECameraMode newMode) {
	bCameraUpdated = true;
	//fAccTimeToChangeModeCenter = 0.f;
	CameraPositions[static_cast<int>(mMode)].fAccTimeToLerp = 0.f;
	CurrentCameraPosition = CameraPositions[static_cast<int>(mMode)].GetPosition(CameraSide);
	if (newMode == ECameraMode::Center) SetCameraSide(ESide::Center);
	else if (newMode == ECameraMode::CloseUp && (oldMode == ECameraMode::Center || oldMode == ECameraMode::Scripted)) SetCameraSide(ESide::Right);
	else if (newMode == ECameraMode::OffSet && (oldMode == ECameraMode::Center || oldMode == ECameraMode::Scripted)) SetCameraSide(ESide::Right);
	else if (newMode == ECameraMode::Scripted) SetCameraSide(ESide::None);
}

void UPlayerCamera::SetCameraMode(ECameraMode newMode) { 
	if (mMode != newMode) CameraModeChanged(mMode, mMode = newMode);
}

ECameraMode UPlayerCamera::GetCameraMode() { 
	return mMode; 
}

void UPlayerCamera::baseTransition(const float& deltaT, float& Acc, float& Time, const FVector& vec) {
	if (Acc + deltaT > Time) Acc = Time;
	else Acc += deltaT;
	if (Acc >= Time) bCameraUpdated = false;
	TargetCameraPosition = FMath::Lerp(
		TargetCameraPosition,
		vec,
		Acc / Time
	);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TargetCameraPosition.ToString());
}
