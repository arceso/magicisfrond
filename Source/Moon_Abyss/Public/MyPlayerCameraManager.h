// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "MyPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class MOON_ABYSS_API AMyPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	float CrouchBlendDuration = 1.0f;
	UPROPERTY(EditDefaultsOnly)
	float CameraLagMaxTimeStep = 0.05;
	UPROPERTY(EditDefaultsOnly)
	float CameraLagSpeed = 2.f;

	float CrouchBlendTime;

public:
	AMyPlayerCameraManager();
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float dT) override;
	
	virtual void BeginPlay() override;

private:
	FVector POVLastLocation;
	FRotator POVLastRotation;
	FVector ACC_Dist;
};
