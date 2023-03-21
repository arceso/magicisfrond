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


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Wallrun", meta = (AllowPrivateAccess = "true"))
	float MAX_DISTANCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Wallrun", meta = (AllowPrivateAccess = "true"))
	float MIN_DISTANCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Wallrun", meta = (AllowPrivateAccess = "true"))
	float SPEED;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Wallrun", meta = (AllowPrivateAccess = "true"))
	float MAX_ROTATION;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Wallrun", meta = (AllowPrivateAccess = "true"))
	FVector JUMP_INFLUENCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Wallrun", meta = (AllowPrivateAccess = "true"))
	float JUMP_FORCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Wallrun", meta = (AllowPrivateAccess = "true"))
	FVector HIT_INFLUENCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Wallrun", meta = (AllowPrivateAccess = "true"))
	float HIT_FORCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Wallrun", meta = (AllowPrivateAccess = "true"))
	float EndWallLaunchForce;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Jump", meta = (AllowPrivateAccess = "true"))
	float AirJumpHeightGain;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Jump", meta = (AllowPrivateAccess = "true"))
	float WallrunJumpAngle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Jump", meta = (AllowPrivateAccess = "true"))
	float AirInfluenceControl;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Slide", meta = (AllowPrivateAccess = "true"))
	float fGroundFrictionBase;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Slide", meta = (AllowPrivateAccess = "true"))
	float fGroundFrictionReduced;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Slide", meta = (AllowPrivateAccess = "true"))
	float fGroundSlideForce;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Slide", meta = (AllowPrivateAccess = "true"))
	float fBrackingForce;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Slide", meta = (AllowPrivateAccess = "true"))
	float fBrackingForceReduced;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Slide", meta = (AllowPrivateAccess = "true"))
	float fAirSlideForce;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Sprint", meta = (AllowPrivateAccess = "true"))
	float fSprintMaxSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Walk", meta = (AllowPrivateAccess = "true"))
	float fWalkMaxSpeed;


	float
		Slide_MinSpeed = 350,
		Slide_EnterImpulse = 2500,
		Slide_GravityForce = 10000,
		Slide_Friction = .4;


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
	
	bool isSliding() const;
	bool isWallruning() const;
	bool isCrouching() const;
	bool isGrappling() const;
	bool isWalking() const;
	bool isFalling() const;
	bool isSprinting() const;

	void Walking(const FVector2D& input);
	void WalkJump(const FVector2D& input);

	void Sprint(bool bStart);
	void Sprinting(const FVector2D& input);

	void Crouch(bool bStart);
	void Crouching(const FVector2D& input);
	void StartCrouching();
	void EndCrouching();
	void CrouchingJump(const FVector2D& input);


	void StartGroundSlide();
	void StartAirSlide();
	void Sliding(const FVector2D& input);
	void SlidingJump(const FVector2D& input);
	void PhysSlide(float deltaT, int32 Iterations);
	bool GetSlideSurface(FHitResult& Hit);

	void Falling(const FVector2D& input);
	void FallJump(const FVector2D& input);

	void Wallruning(const FVector2D& input);
	void EndWallrun(EEndReason reason);
	void StartWallrun(ESide Side, FVector normal);
	void Wallrun(ESide side, FHitResult fhr);
	void UpdateWallrun(FVector_NetQuantizeNormal* newNormal);
	void WallrunJump(const FVector2D& input);

	void Grappling(const FVector2D& input);
	void GrapplingJump(const FVector2D& input);

	// virtual void Landed() override;
	virtual void PhysCustom(float dT, int32 iterations) override;
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;
	virtual void CapsuleTouched(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	bool 
		bCanAirJump,
		bIsSprinting,
		bIsSliding,
		bIsCrouching;

	FCollisionQueryParams SelfExcludeQueryParams;

protected:
	FHitResult OutHit;
	
private:
	void SprintingJump(const FVector2D& input);
	void EndSliding();
};
