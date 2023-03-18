// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Side.h"
#include "MainCharacterMovementComponent.generated.h"

enum class EEndReason {
	NoWallFound,
	Jump,
	Hit,
	InvalidInput
};

/**
 * 
 */
UCLASS()
class MOON_ABYSS_API UMainCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterMovement:Wallrun", meta = (AllowPrivateAccess = "true"))
	float MAX_DISTANCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterMovement:Wallrun", meta = (AllowPrivateAccess = "true"))
	float MIN_DISTANCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterMovement:Wallrun", meta = (AllowPrivateAccess = "true"))
	float SPEED;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterMovement:Wallrun", meta = (AllowPrivateAccess = "true"))
	float MAX_ROTATION;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterMovement:Wallrun", meta = (AllowPrivateAccess = "true"))
	FVector JUMP_INFLUENCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterMovement:Wallrun", meta = (AllowPrivateAccess = "true"))
	float JUMP_FORCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Wallrun, meta = (AllowPrivateAccess = "true"))
	FVector HIT_INFLUENCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Wallrun, meta = (AllowPrivateAccess = "true"))
	float HIT_FORCE;

	struct WR_DATA {
		bool WallRuning;
		ESide Side;
		FVector_NetQuantizeNormal NormalHit;
	} WRData;

	void AddInputVector(FVector WorldVector, bool bForce /* =	 false */);

	//void WR_Move(const FInputActionValue& Value);
	//void WR_dbjump(const FInputActionValue& Value);

	virtual void BeginPlay() override;
	void Move(const FVector2D& input);
	void Jump(const FVector2D& input);
	
	void Walking(const FVector2D& input);
	void WalkJump(const FVector2D& input);

	void Falling(const FVector2D& input);
	void FallJump(const FVector2D& input);

	void Sliding(const FVector2D& input);
	
	void Wallruning(const FVector2D& input);
	void EndWallrun(EEndReason reason);
	void StartWallrun(ESide Side, FVector normal);
	void Wallrun(ESide side, FHitResult fhr);
	void UpdateWallrun(FVector_NetQuantizeNormal* newNormal);
	void WallrunJump(const FVector2D& input);
	void Crouching(const FVector2D& input);
	
	void Grappling(const FVector2D& input);

	// virtual void Landed() override;
	virtual void CapsuleTouched(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



	bool bCanAirJump;
	FCollisionQueryParams SelfExcludeQueryParams;

protected:
	FHitResult OutHit;
	
};
