// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Blueprint/UserWidget.h"
#include "Side.h"
#include "PlayerCamera.h"
#include "PlayerMainCharacterController.h"
#include "MainCharacterMovementComponent.h"
#include "Moon_AbyssCharacter.generated.h"

UCLASS(config=Game)
class AMoon_AbyssCharacter : public ACharacter
{
	GENERATED_BODY()

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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
		UPlayerCamera* PlayerCamera;

	//UFUNCTION(BlueprintCallable, Category="Movement")
	virtual UMainCharacterMovementComponent* GetMovementComponent() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
		UMainCharacterMovementComponent* PlayerMovement;

	virtual void PostInitializeComponents() override;

public:
	AMoon_AbyssCharacter(const FObjectInitializer& ObjectInitializer);
	

protected:


	void StartWallrun(ESide side, FVector& fhr);
	virtual void Landed(const FHitResult& Hit) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	void updateDynamicUi();
	virtual void Tick(float DeltaSeconds) override;
	UUserWidget* EnemySelectorInstance;
	UUserWidget* CrosshairInstance;

	//UPlayerCamera* PlayerCamera;

	APlayerMainCharacterController* MyController;

	struct WR_DATA {
		bool WallRuning;
		ESide Side;
		FVector NormalHit;
	} WRData;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void BeginPlay();

};

