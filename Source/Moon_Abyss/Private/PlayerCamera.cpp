// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCamera.h"
#include "GameFramework/SpringArmComponent.h"

UPlayerCamera::UPlayerCamera() {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetComponentTickEnabled(true);
	bAutoRegister = true;
	bAutoActivate = true;

	RightCameraPosition = FVector(-50, 100, 100);
	LeftCameraPosition = FVector(-50, -100, 100);
	CurrentCameraPosition = RightCameraPosition;
	fTimeToChangeSide = 1.0f;
	fAccTimeToChangeSide = 0.0f;
	fTimeToChangeHeight = 1.f;
	fAccTimeToChangeHeight = 0.0f;
	bShouldUpdateCameraPosition = false;

	SweepShape.SetSphere(15.f);
}

void UPlayerCamera::BeginPlay() {
	Super::BeginPlay();
}

void UPlayerCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bShouldUpdateCameraPosition) {
		if (fAccTimeToChangeSide + DeltaTime > fTimeToChangeSide) fAccTimeToChangeSide = fTimeToChangeSide;
		else fAccTimeToChangeSide += DeltaTime;
		if (fAccTimeToChangeSide >= fTimeToChangeSide) bShouldUpdateCameraPosition = false;

		TargetCameraPosition = FMath::Lerp(
			TargetCameraPosition,
			CameraSide == ESide::LEFT ? LeftCameraPosition : RightCameraPosition,
			fAccTimeToChangeSide / fTimeToChangeSide
		);

		// Line trace
		FHitResult TraceHit;
		FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), false, GetOwner());
		if (GetWorld()->LineTraceSingleByChannel(TraceHit, GetOwner()->GetActorLocation(), GetOwner()->GetTransform().TransformPosition(TargetCameraPosition), ECollisionChannel::ECC_WorldStatic, TraceParams)) {
			TargetCameraPosition = TraceHit.Location - GetOwner()->GetActorLocation();
			//TargetCameraPosition.SetComponentForAxis(EAxis::Z, 400 + TargetCameraPosition.X);
		}
	}

	CurrentCameraPosition = TargetCameraPosition;
	SetRelativeLocation(CurrentCameraPosition);
	SetRelativeRotation(TargetCameraRotation);
}

void UPlayerCamera::HandleInput(FVector2D input) {
	bShouldUpdateCameraPosition = true;
	TargetCameraRotation = (GetRelativeRotation() + FRotator(input.Y * -1.f, 0.f, 0.f));
}

void UPlayerCamera::SetCameraSide(ESide Side) {
	if (Side != CameraSide) {
		CameraSide = Side;
		bShouldUpdateCameraPosition = true;
		fAccTimeToChangeSide = 0.f;
	}
}

ESide UPlayerCamera::GetCameraSide() {
	return CameraSide;
}