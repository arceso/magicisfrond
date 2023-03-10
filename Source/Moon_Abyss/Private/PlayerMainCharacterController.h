// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Side.h"
#include "InputActionValue.h"
#include "PlayerMainCharacterController.generated.h"

class UPlayerCamera;

UCLASS()
class APlayerMainCharacterController : public APlayerController
{
	GENERATED_BODY()


public:
	// Peasant stuff
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void DBJump(const FInputActionValue& Value);

	// Wall runners stuff
	void WR_Move(const FInputActionValue& Value);
	void WR_Look(const FInputActionValue& Value);
	void WR_dbjump(const FInputActionValue& Value);
	void WR_Movement(ESide side, FHitResult fhr);
	void StartWallrun(ESide Side, FVector normal);
	void EndWallrun();
	void UpdateWallrun(FVector_NetQuantizeNormal* newNormal);

	// Tech stuff
	APlayerMainCharacterController();
	void BeginPlay()override;
	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);
	UPlayerCamera* camera;

	bool bCanAirJump;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Wallrun, meta = (AllowPrivateAccess = "true"))
	float MAX_DISTANCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Wallrun, meta = (AllowPrivateAccess = "true"))
	float MIN_DISTANCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Wallrun, meta = (AllowPrivateAccess = "true"))
	float SPEED;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Wallrun, meta = (AllowPrivateAccess = "true"))
	float MAX_ROTATION;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Wallrun, meta = (AllowPrivateAccess = "true"))
	FVector JUMP_INFLUENCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Wallrun, meta = (AllowPrivateAccess = "true"))
	float JUMP_FORCE;

	// :WARNING: This is being used on character.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Wallrun, meta = (AllowPrivateAccess = "true"))
	FVector HIT_INFLUENCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Wallrun, meta = (AllowPrivateAccess = "true"))
	float HIT_FORCE;

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

	struct WR_DATA {
		bool WallRuning;
		ESide Side;
		FVector_NetQuantizeNormal NormalHit;
	} WRData;

private:
	float ACC;
};
