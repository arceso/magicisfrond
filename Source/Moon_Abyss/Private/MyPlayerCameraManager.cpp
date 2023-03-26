// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerCameraManager.h"

#include "GameFramework/SpringArmComponent.h"
#include "Moon_Abyss/Moon_AbyssCharacter.h"
#include "MainCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

AMyPlayerCameraManager::AMyPlayerCameraManager() {
	ViewRollMin = 0.f;
	ViewRollMax = 0.f;
}
void AMyPlayerCameraManager::BeginPlay() {

	POVLastLocation = GetOwningPlayerController()->GetPawn()->GetTargetLocation();
}


void AMyPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float dT)
{
	Super::UpdateViewTarget(OutVT, dT);

	if (AMoon_AbyssCharacter* MyCharacter = Cast<AMoon_AbyssCharacter>(GetOwningPlayerController()->GetPawn())) {
		UMainCharacterMovementComponent* CMC = Cast<UMainCharacterMovementComponent>(MyCharacter->GetCustomMovementComponent());
		FVector TargetCrouchOffset(
			0,
			0,
			CMC->GetCrouchedHalfHeight() - MyCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		);
		
		const FVector ArmMovementStep = (OutVT.POV.Location - POVLastLocation) * (1.f / dT);
		FVector LerpTarget = POVLastLocation;
		float RemainingTime = dT;
		while (RemainingTime > UE_KINDA_SMALL_NUMBER) {
			const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
			LerpTarget += ArmMovementStep * LerpAmount;
			RemainingTime -= LerpAmount;
			OutVT.POV.Location = FMath::VInterpTo(POVLastLocation, LerpTarget, LerpAmount, CameraLagSpeed);

			POVLastLocation = OutVT.POV.Location;
		}
		OutVT.POV.Rotation = (MyCharacter->GetActorLocation() - OutVT.POV.Location).Rotation();

		FVector Offset = FMath::Lerp(FVector::ZeroVector, TargetCrouchOffset, FMath::Clamp(CrouchBlendTime / CrouchBlendDuration, 0.f, 1.f));
		if (CMC->isCrouching()) {
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime + dT, 0.f, CrouchBlendDuration);
			Offset -= TargetCrouchOffset;
		} else CrouchBlendTime = FMath::Clamp(CrouchBlendTime - dT, 0.f, CrouchBlendDuration);

		if (CMC->IsMovingOnGround()) OutVT.POV.Location += Offset;
		POVLastLocation = OutVT.POV.Location;
	}
}
