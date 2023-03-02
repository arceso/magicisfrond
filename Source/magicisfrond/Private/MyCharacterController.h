// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ESide.h"
#include "InputActionValue.h"
#include "MyCharacterController.generated.h"

class UMyCharacterCamera;

UCLASS()
class AMyCharacterController : public APlayerController
{
	GENERATED_BODY()


public:
	// Tech stuff
	AMyCharacterController();
	void BeginPlay()override;
	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);
	UMyCharacterCamera* camera;

	// Peasant stuff
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void DBJump(const FInputActionValue& Value);

	// Wall runners stuff
	void WR_Move(const FInputActionValue& Value);
	void WR_Look(const FInputActionValue& Value);
	void WR_dbjump(const FInputActionValue& Value);
	void WR_Movement(ESide side, float movement, FHitResult fhr);
	void StartWallrun(ESide Side, FVector normal);
	void EndWallrun();

	// Tech stuff pt2
	/** INPUTS **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* dbjump;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* WR_MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* WR_LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* WR_dbjumpAction;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputMappingContext* WallrunMappingContext;

	bool bCanAirJump;
private:

	struct WR_DATA {
		bool WallRuning;
		ESide WR_SIDE;
		FVector NormalHit;
	};

	WR_DATA WRData;
};
