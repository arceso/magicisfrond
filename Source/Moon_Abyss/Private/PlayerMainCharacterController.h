// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Side.h"
#include "InputActionValue.h"
#include "MainCharacterMovementComponent.h"
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

	//Grapple stuff
	void Grapple(const FInputActionValue& Value);

	// Tech stuff
	APlayerMainCharacterController();
	void BeginPlay()override;
	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);
	UPlayerCamera* camera;
	UMainCharacterMovementComponent* CMC;


	/** INPUTS **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = InputActions, meta = (AllowPrivateAccess = "true"))
		class UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = InputActions, meta = (AllowPrivateAccess = "true"))
		class UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = InputActions, meta = (AllowPrivateAccess = "true"))
		class UInputAction* dbjump;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = InputActions, meta = (AllowPrivateAccess = "true"))
		class UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = InputActions, meta = (AllowPrivateAccess = "true"))
		class UInputAction* GrappleAction;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = InputMappings, meta = (AllowPrivateAccess = "true"))
		class UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = InputMappings, meta = (AllowPrivateAccess = "true"))
		class UInputMappingContext* GrappleMappingContext;



private:
	float ACC;
};
