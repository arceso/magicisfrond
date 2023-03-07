#include "Moon_AbyssCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerMainCharacterController.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCamera.h"
#include "PlayerMainCharacterController.h"
#include "Side.h"
#include "Blueprint/UserWidget.h"

//////////////////////////////////////////////////////////////////////////
// AMoon_AbyssCharacter

AMoon_AbyssCharacter::AMoon_AbyssCharacter() {

	// Magic Numbers
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 0.f);

	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->AirControl = 90000.f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	PlayerCamera = CreateDefaultSubobject<UPlayerCamera>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->bUsePawnControlRotation = false;
}


void AMoon_AbyssCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	((APlayerMainCharacterController*)Controller)->SetupPlayerInputComponent(PlayerInputComponent);
}
void AMoon_AbyssCharacter::BeginPlay() {
	Super::BeginPlay();
	MyController = (APlayerMainCharacterController*)Controller;
	MyController->camera = PlayerCamera;
	MyController->WRData.WallRuning = false;
	MyController->WRData.NormalHit = FVector::ZeroVector;
	MyController->WRData.Side = ESide::NONE;

	// Magic Numbers
	PlayerCamera->SetRelativeLocation(FVector(-400, 100, 100));



	EnemySelectorInstance = CreateWidget<UUserWidget>(GetWorld(), W_EnemyTargeted);
	EnemySelectorInstance->AddToViewport();
	EnemySelectorInstance->SetVisibility(ESlateVisibility::Hidden);

	CrosshairInstance = CreateWidget<UUserWidget>(GetWorld(), W_Crosshair);
	CrosshairInstance->AddToViewport();
	UGameViewportClient* Viewport = GetWorld()->GetGameViewport();
	FIntPoint ViewSize = Viewport->Viewport->GetSizeXY();


	//PlayerCamera->SetRelativeLocation(FVector(-400, DistanceSide * 2, Height));

	// Magic Numbers
	CrosshairInstance->SetRenderScale(FVector2D(.033, .05));
	GetMovementComponent()->SetPlaneConstraintNormal(FVector(0, 0, 1));
}

//////////////////////////////////////////////////////////////////////////
// Input


void AMoon_AbyssCharacter::Landed(const FHitResult& Hit) {
	MyController->bCanAirJump = true;
}

void AMoon_AbyssCharacter::NotifyHit(
	class UPrimitiveComponent* MyComp,
	AActor* Other,
	class UPrimitiveComponent* OtherComp,
	bool bSelfMoved,
	FVector HitLocation,
	FVector HitNormal,
	FVector NormalImpulse,
	const FHitResult& Hit
) {
	GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("HIT"));
	//GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Cyan, HitNormal.ToString());
	if (!MyController->WRData.WallRuning) {
		if (GetActorRightVector().Dot(HitNormal) < -0.50f) {
			MyController->StartWallrun(ESide::RIGHT, HitNormal);
			MyController->WRData.Side = ESide::RIGHT;
			MyController->WRData.WallRuning = true;
			MyController->WRData.NormalHit = HitNormal;

		}
		else if (GetActorRightVector().Dot(HitNormal) > 0.5f) {
			MyController->StartWallrun(ESide::LEFT, HitNormal);
			MyController->WRData.Side = ESide::LEFT;
			MyController->WRData.WallRuning = true;
			MyController->WRData.NormalHit = HitNormal;
		}
	}
	else {
		// Magic Numbers
		MyController->EndWallrun();
		LaunchCharacter(
			(HitNormal + WRData.NormalHit).GetSafeNormal() * 2000,
			true,
			true
		);
		MyController->WRData.Side = ESide::NONE;
		MyController->WRData.WallRuning = false;
		MyController->WRData.NormalHit = FVector::ZeroVector;
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TEXT("YES"));
	}
}

void AMoon_AbyssCharacter::updateDynamicUi() {
	const int length = 5000;
	struct FHitResult OutHit;
	APlayerCameraManager* camManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	const FVector Start = camManager->GetCameraLocation();
	const FVector End = Start + camManager->GetCameraRotation().Vector() * length;
	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), true, NULL);
	TraceParams.AddIgnoredActor(this);
	bool Target = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, TraceParams);

	if (Target && OutHit.GetActor()->ActorHasTag(FName("Enemy"))) {
		FVector2D ScreenLocation;
		if (UGameplayStatics::ProjectWorldToScreen(GetLocalViewingPlayerController(), OutHit.GetActor()->GetActorLocation(), ScreenLocation, false)) {
			if (EnemySelectorInstance->GetVisibility() == ESlateVisibility::Hidden) {
				EnemySelectorInstance->SetVisibility(ESlateVisibility::Visible);
			}
			FVector2D ESSize = EnemySelectorInstance->GetCachedGeometry().GetLocalSize();
			EnemySelectorInstance->SetPositionInViewport(ScreenLocation - (ESSize / 2));
		}
	}
	else if (EnemySelectorInstance->GetVisibility() == ESlateVisibility::Visible) {
		EnemySelectorInstance->SetVisibility(ESlateVisibility::Hidden);
	}
}
void AMoon_AbyssCharacter::Wallrun(float deltaT) {
	//if (MyController->WRData.WallRuning) {
	//	struct FHitResult OutHit;
	//	const FVector Start = GetActorLocation();
	//	const FVector End = Start + GetActorRightVector() * (MyController->WRData.Side == ESide::RIGHT ? 1 : -1) * 100;
	//	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), true, NULL);
	//	TraceParams.AddIgnoredActor(this);
	//	bool Target = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, TraceParams);
	//	MyController->UpdateWallrun(&(OutHit.ImpactNormal));
	//}
}

void AMoon_AbyssCharacter::Tick(float DeltaSeconds) {
	updateDynamicUi();
}