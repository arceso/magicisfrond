// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Side.h"
#include "Components/CapsuleComponent.h"
#include "CollisionShape.h"
#include "PlayerCamera.generated.h"

enum class ECameraMode: int {
	Center = 0,
	OffSet,
	CloseUp,
	Scripted,
	MAX
};

USTRUCT(BlueprintType)
struct FCameraPosition {
	GENERATED_USTRUCT_BODY()

public:
	FCameraPosition() {};
	FCameraPosition(FName name):PositionName(name) {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Position", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
		FVector Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Lerp Timmings", meta = (AllowPrivateAccess = "true"))
		float fTimeToLerp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Lerp Timmings", meta = (AllowPrivateAccess = "true"))
		float fAccTimeToLerp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Position Name")
		FName PositionName;
		
	
	FVector GetPosition(ESide side) {
		if (side == ESide::Left) return FVector(Position.X, Position.Y * -1, Position.Z);
		else return Position;
	}
};

/**
 *
 */
UCLASS()
class UPlayerCamera : public UCameraComponent {

	GENERATED_BODY()

public:
	using onTickCallBack = void (UPlayerCamera::*)(float);
	UPlayerCamera();
	void HandleInput(FVector2D input);
	void SetCameraSide(ESide Side);
	ESide GetCameraSide();
	void SetCameraMode(ECameraMode newMode);
	ECameraMode GetCameraMode();

	FCollisionShape SweepShape;
	
	UPROPERTY(EditAnywhere, Category = CameraPositions, meta = (AllowPrivateAccess = "true", TitleProperty = "ItemName", ArrayClamp = "static_cast<int>(ECameraMode::MAX)"))
		TArray<struct FCameraPosition> CameraPositions;

private:
	ESide CameraSide;
	ECameraMode mMode;

	USceneComponent* LocalTransform;

	FVector CurrentCameraPosition,
		TargetCameraPosition;

	FRotator CurrentCameraRotation,
		TargetCameraRotation;

	float
		fAccTimeToChangeSide,
		fAccTimeToChangeHeight,
		fAccTimeToChangeModeScripted;

	FVector OldLocation;

	onTickCallBack modeTransitionToFunction;
	void baseTransition(const float& deltaT, float& Acc, float& Time, const FVector& vec);

	bool bCameraSideUpdated,
		bCameraUpdated;

	void CameraSideChanged(ESide oldSide, ESide newSide);
	void CameraModeChanged(ECameraMode oldMode, ECameraMode newMode);
	void SetToValidTargetPosition();
	void SetRelPosToProperSide(float DeltaT);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};

