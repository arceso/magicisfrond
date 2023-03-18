#include "Moon_AbyssCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCamera.h"
#include "MainCharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PawnMovementComponent.h"
#include "CustomMovementFlags.h"

//////////////////////////////////////////////////////////////////////////
// AMoon_AbyssCharacter

AMoon_AbyssCharacter::AMoon_AbyssCharacter(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UMainCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Magic Numbers
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 0.f);

	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->AirControl = 1.f;
	GetCharacterMovement()->MaxWalkSpeed = 1000.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	PlayerCamera = CreateDefaultSubobject<UPlayerCamera>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->bUsePawnControlRotation = false;


	//PlayerMovement = CreateDefaultSubobject<UMainCharacterMovementComponent>(TEXT("PlayerMovement"));
	//PlayerMovement->UpdatedComponent = RootComponent;
}


void AMoon_AbyssCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	((APlayerMainCharacterController*)Controller)->SetupPlayerInputComponent(PlayerInputComponent);
}
void AMoon_AbyssCharacter::BeginPlay() {
	Super::BeginPlay();
	MyController = (APlayerMainCharacterController*)Controller;
	MyController->camera = PlayerCamera;

	// Magic Numbers
	//PlayerCamera->SetRelativeLocation(FVector(-400, 100, 100));



	EnemySelectorInstance = CreateWidget<UUserWidget>(GetWorld(), W_EnemyTargeted);
	EnemySelectorInstance->AddToViewport();
	EnemySelectorInstance->SetVisibility(ESlateVisibility::Hidden);

	CrosshairInstance = CreateWidget<UUserWidget>(GetWorld(), W_Crosshair);
	CrosshairInstance->AddToViewport();
	UGameViewportClient* Viewport = GetWorld()->GetGameViewport();
	FIntPoint ViewSize = Viewport->Viewport->GetSizeXY();


	//PlayerCamera->SetRelativeLocation(FVector(-400, DistanceSide * 2, Height));

	// Magic Numbers y dumb things
	CrosshairInstance->SetRenderScale(FVector2D(.033, .05));
}

void AMoon_AbyssCharacter::StartWallrun(ESide side, FVector& hit) {
	PlayerMovement->StartWallrun(side, hit);
	PlayerCamera->SetCameraSide(side == ESide::LEFT ? ESide::RIGHT : ESide::RIGHT);
}

//////////////////////////////////////////////////////////////////////////
// Input


void AMoon_AbyssCharacter::Landed(const FHitResult& Hit) {
	PlayerMovement->bCanAirJump = true;
}

void AMoon_AbyssCharacter::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) {
	float VectorSimilarity = GetActorRightVector().Dot(HitNormal);
	if (PlayerMovement->MovementMode != CMOVE_Wallruning) {
		if (VectorSimilarity < -0.5f) StartWallrun(ESide::RIGHT, HitNormal);
		else if (VectorSimilarity > 0.5f) StartWallrun(ESide::LEFT, HitNormal);
	}
	else if (VectorSimilarity > -0.50f && VectorSimilarity < 0.5f) PlayerMovement->EndWallrun(EEndReason::Hit);
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


void AMoon_AbyssCharacter::PostInitializeComponents() {
	Super::PostInitializeComponents();
	UPawnMovementComponent* pepegarcialopez = nullptr;
	pepegarcialopez = Super::GetMovementComponent();
	PlayerMovement = reinterpret_cast<UMainCharacterMovementComponent*>(Super::GetMovementComponent());
}

UMainCharacterMovementComponent* AMoon_AbyssCharacter::GetMovementComponent() const {
	return PlayerMovement;
}

void AMoon_AbyssCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	updateDynamicUi();
}