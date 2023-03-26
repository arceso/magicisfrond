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


#include "Components/TextRenderComponent.h"

#include "Moon_AbyssCharacter.generated.h"
UCLASS(config=Game)
class AMoon_AbyssCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> W_EnemyTargeted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> W_Crosshair;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
		UPlayerCamera* PlayerCamera;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* TriggerCapsule;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	UTextRenderComponent* StateText;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION() 
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	//UFUNCTION(BlueprintCallable, Category="Movement")
	virtual UMainCharacterMovementComponent* GetMovementComponent() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
		UMainCharacterMovementComponent* PlayerMovement;

	virtual void PostInitializeComponents() override;

public:
	AMoon_AbyssCharacter(const FObjectInitializer& ObjectInitializer);
	UMainCharacterMovementComponent* GetCustomMovementComponent();
	
protected:
	void SetUpWallrun(ESide side, FVector& fhr);
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

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void BeginPlay();

};

