// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCamera.h"

UPlayerCamera::UPlayerCamera() {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetComponentTickEnabled(true);
	bAutoRegister = true;
	bAutoActivate = true;

	RightCameraPosition = FVector(-400, 100, 100);
	LeftCameraPosition = FVector(-400, -100, 100);
	CurrentCameraPosition = RightCameraPosition;

	fTimeToChangeSide = 1.0f;
	fAccTime = 0.0f;

	bShouldUpdateCameraPosition = false;
}

void UPlayerCamera::BeginPlay() {
	SetRelativeLocation(RightCameraPosition);
}

void UPlayerCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bShouldUpdateCameraPosition) {
		if (fAccTime + DeltaTime > fTimeToChangeSide) fAccTime = fTimeToChangeSide;
		else fAccTime += DeltaTime;
		if (fAccTime >= fTimeToChangeSide) bShouldUpdateCameraPosition = false;

		CurrentCameraPosition = FMath::Lerp(
			CurrentCameraPosition,
			CameraSide == ESide::LEFT ? LeftCameraPosition : RightCameraPosition,
			fAccTime / fTimeToChangeSide
		);
		this->SetRelativeLocation(CurrentCameraPosition);
	}
}

void UPlayerCamera::HandleInput(FVector2D input) {
	FQuat FinalRelRotation = (this->GetRelativeRotation() + FRotator(input.Y * -1.f, 0.f, 0.f)).Quaternion();
	this->SetRelativeRotation(FinalRelRotation);
	this->SetRelativeLocation(FinalRelRotation.RotateVector(CurrentCameraPosition));
}

void UPlayerCamera::SetCameraSide(ESide Side) {
	if (Side != CameraSide) {
		CameraSide = Side;
		bShouldUpdateCameraPosition = true;
		fAccTime = 0.f;
	}
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, Side == ESide::LEFT ? TEXT("LEFT") : TEXT("RIGHT"));
}

ESide UPlayerCamera::GetCameraSide() {
	return CameraSide;
}