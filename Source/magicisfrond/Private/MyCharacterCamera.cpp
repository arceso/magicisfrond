// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterCamera.h"

UMyCharacterCamera::UMyCharacterCamera() {
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

void UMyCharacterCamera::BeginPlay() {
	SetRelativeLocation(RightCameraPosition);
}

void UMyCharacterCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
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

void UMyCharacterCamera::HandleInput(FVector2D input) {
	FQuat FinalRelRotation = (this->GetRelativeRotation() + FRotator(input.Y * -1.f, 0.f, 0.f)).Quaternion();
	this->SetRelativeRotation(FinalRelRotation);
	this->SetRelativeLocation(FinalRelRotation.RotateVector(CurrentCameraPosition));
}	

void UMyCharacterCamera::SetCameraSide(ESide Side) {
	if (Side != CameraSide) {
		CameraSide = Side;
		bShouldUpdateCameraPosition = true;
		fAccTime = 0.f;
	}
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, Side == ESide::LEFT ? TEXT("LEFT") : TEXT("RIGHT"));
}

ESide UMyCharacterCamera::GetCameraSide() {
	return CameraSide;
}