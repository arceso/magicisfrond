// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "ESide.h"
#include "MyCharacterCamera.generated.h"


/**
 * 
 */
UCLASS()
class UMyCharacterCamera : public UCameraComponent
{
	GENERATED_BODY()
	
	UMyCharacterCamera();
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
