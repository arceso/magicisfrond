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

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CameraCapsule"));
	CapsuleComp->SetupAttachment(this, TEXT("CameraCapsule"));
	

}

void UPlayerCamera::BeginPlay() {
	SetRelativeLocation(RightCameraPosition, true);
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

		FHitResult OutHit;
		this->SetRelativeRotation(TargetCameraRotation);
		this->SetRelativeLocation(TargetCameraRotation.RotateVector(CurrentCameraPosition), true, &OutHit, ETeleportType::None);
		if (OutHit.bBlockingHit) {
			this->SetRelativeLocation(OutHit.Location);
			bShouldUpdateCameraPosition = false;
		}

	}
}

void UPlayerCamera::HandleInput(FVector2D input) {
	bShouldUpdateCameraPosition = true;
	TargetCameraRotation = (this->GetRelativeRotation() + FRotator(input.Y * -1.f, 0.f, 0.f));
}

void UPlayerCamera::SetCameraSide(ESide Side) {
	if (Side != CameraSide) {
		CameraSide = Side;
		bShouldUpdateCameraPosition = true;
		fAccTime = 0.f;
	}
}

ESide UPlayerCamera::GetCameraSide() {
	return CameraSide;
}