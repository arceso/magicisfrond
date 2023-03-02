#include "magicisfrondCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MyCharacterController.h"
#include "Kismet/GameplayStatics.h"
#include "MyCharacterCamera.h"
#include "ESide.h"

//////////////////////////////////////////////////////////////////////////
// AmagicisfrondCharacter

AmagicisfrondCharacter::AmagicisfrondCharacter() {
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f ,0.f, 0.f);

	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->AirControl = 90000.f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	PlayerCamera = CreateDefaultSubobject<UMyCharacterCamera>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->bUsePawnControlRotation = false;
}


 void AmagicisfrondCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	 //Super::SetupPlayerInputComponent(PlayerInputComponent);
	 ((AMyCharacterController*)Controller)->SetupPlayerInputComponent(PlayerInputComponent);
}
void AmagicisfrondCharacter::BeginPlay() {
	Super::BeginPlay();
	MyController = (AMyCharacterController*)Controller;
	MyController->camera = PlayerCamera;
	PlayerCamera->SetRelativeLocation(FVector(-400, 100, 100));

	//Add Input Mapping Context

	EnemySelectorInstance = CreateWidget<UUserWidget>(GetWorld(), W_EnemyTargeted);
	EnemySelectorInstance->AddToViewport();
	EnemySelectorInstance->SetVisibility(ESlateVisibility::Hidden);

	CrosshairInstance = CreateWidget<UUserWidget>(GetWorld(), W_Crosshair);
	CrosshairInstance->AddToViewport();
	UGameViewportClient* Viewport = GetWorld()->GetGameViewport();
	FIntPoint ViewSize = Viewport->Viewport->GetSizeXY();
	//PlayerCamera->SetRelativeLocation(FVector(-400, DistanceSide * 2, Height));

	CrosshairInstance->SetRenderScale(FVector2D(.033, .05));
	GetMovementComponent()->SetPlaneConstraintNormal(FVector(0, 0, 1));
}

//////////////////////////////////////////////////////////////////////////
// Input


void AmagicisfrondCharacter::Landed(const FHitResult& Hit) {
	MyController->bCanAirJump = true;
}



void AmagicisfrondCharacter::NotifyHit(
	class UPrimitiveComponent* MyComp,
	AActor* Other,
	class UPrimitiveComponent* OtherComp,
	bool bSelfMoved,
	FVector HitLocation,
	FVector HitNormal,
	FVector NormalImpulse,
	const FHitResult& Hit
) {
	 {
		if (GetActorRightVector().Dot(HitNormal) < 0.0f)
			MyController->StartWallrun(ESide::RIGHT, HitNormal);
		else if (GetActorRightVector().Dot(HitNormal) > 0.0f)
			MyController->StartWallrun(ESide::LEFT, HitNormal);
	}
}

void AmagicisfrondCharacter::updateDynamicUi() {
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

void AmagicisfrondCharacter::Tick(float DeltaSeconds) {
	updateDynamicUi();
}