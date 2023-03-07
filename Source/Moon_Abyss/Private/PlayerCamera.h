// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Side.h"
#include "PlayerCamera.generated.h"


/**
 *
 */
UCLASS()
class UPlayerCamera : public UCameraComponent
{
	GENERATED_BODY()

		UPlayerCamera();
public:
	void HandleInput(FVector2D input);
	void SetCameraSide(ESide Side);
	ESide GetCameraSide();

private:
	ESide CameraSide;

	FVector RightCameraPosition,
		LeftCameraPosition,
		CurrentCameraPosition;

	float fTimeToChangeSide,
		fAccTime;
	bool bShouldUpdateCameraPosition;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
