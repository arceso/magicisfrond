// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "magicisfrondCharacter.generated.h"

class UMyCharacterCamera;
class USpringArmComponent;
UCLASS(config=Game)
class AmagicisfrondCharacter : public ACharacter
{
	GENERATED_BODY()

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* WallrunMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Look Jump Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* dbjump;

	// WALLRUN

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* WR_MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* WR_LookAction;

	/** Look Jump Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* WR_dbjumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		double DistanceToCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		double DistanceSide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		double Height;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> W_EnemyTargeted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> W_Crosshair;

public:
	AmagicisfrondCharacter();
	

protected:


	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	void DBJump(const FInputActionValue& Value);
	void EndWallrun();
	void WR_Move(const FInputActionValue& Value);
	void WR_Look(const FInputActionValue& Value);
	void WR_dbjump(const FInputActionValue& Value);
	enum E_WR_Side{LEFT = 1, RIGHT = -1, NONE = 0};

	void WR_Movement(E_WR_Side side, float movement, FHitResult fhr);
	void StartWallrun(E_WR_Side Side, FVector normal);

	virtual void Landed(const FHitResult& Hit) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	void updateDynamicUi();
	virtual void Tick(float DeltaSeconds) override;
	
	UUserWidget* EnemySelectorInstance;
	UUserWidget* CrosshairInstance;

	UMyCharacterCamera* PlayerCamera;
	USpringArmComponent* CameraBoom;

	bool CanAirJump;

	float accTimeCamera;
	E_WR_Side CameraSide;

	struct WR_DATA {
		bool WallRuning = false;
		E_WR_Side WR_SIDE = NONE;
		FVector NormalHit = FVector(0,0,0);
	};
	WR_DATA WRData;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();


public:
	/** Returns CameraBoom subobject **/
	// FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	// FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

