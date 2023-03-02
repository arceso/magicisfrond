// Copyright Epic Games, Inc. All Rights Reserved.

#include "magicisfrondCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "MyCharacterCamera.h" //"Source/Private/MyCharacterCamera.h"	


//////////////////////////////////////////////////////////////////////////
// AmagicisfrondCharacter

AmagicisfrondCharacter::AmagicisfrondCharacter() :
	CanAirJump(true),
	CameraSide(RIGHT)
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(-1, -1, -1);

	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->AirControl = 90000.f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	PlayerCamera = CreateDefaultSubobject<UMyCharacterCamera>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->bUsePawnControlRotation = false;
}

void AmagicisfrondCharacter::BeginPlay() {
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller)) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
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

void AmagicisfrondCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AmagicisfrondCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AmagicisfrondCharacter::Look);
		EnhancedInputComponent->BindAction(dbjump, ETriggerEvent::Triggered, this, &AmagicisfrondCharacter::DBJump);
		EnhancedInputComponent->BindAction(WR_MoveAction, ETriggerEvent::Triggered, this, &AmagicisfrondCharacter::WR_Move);
		EnhancedInputComponent->BindAction(WR_LookAction, ETriggerEvent::Triggered, this, &AmagicisfrondCharacter::WR_Look);
		EnhancedInputComponent->BindAction(WR_dbjumpAction, ETriggerEvent::Triggered, this, &AmagicisfrondCharacter::WR_dbjump);
	}
}

void AmagicisfrondCharacter::DBJump(const FInputActionValue& Value) {
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (GetCharacterMovement()->IsFalling() && CanAirJump) {
		CanAirJump = false;
		FVector VInMDir = FVector(MovementVector.GetSafeNormal(), 1) * (GetMovementComponent()->Velocity.Length() * .5);
		VInMDir.Z += 1000;
		FVector finalV = Controller->GetControlRotation().RotateVector(VInMDir);
		LaunchCharacter(finalV, true, true);
	} else ACharacter::Jump();
}

void AmagicisfrondCharacter::Move(const FInputActionValue& Value) {
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		const FVector lCameraPos = FVector(0, DistanceSide * -1, Height);
		const FVector rCameraPos = FVector(0, DistanceSide, Height);

		double distance = lCameraPos.Length();

		FRotator lRot = lCameraPos.Rotation();
		FRotator rRot = rCameraPos.Rotation();

		FVector a = lRot.RotateVector(GetActorForwardVector() * distance);
		FVector b = rRot.RotateVector(GetActorForwardVector() * distance);

		if (MovementVector.Y < 0) {
			//CameraBoom->SetRelativeLocation(a);
		}

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AmagicisfrondCharacter::Look(const FInputActionValue& Value) {
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr) {

		PlayerCamera->HandleInput(LookAxisVector);
		
		//RootComponent->SetRelativeRotation(
		//	(RootComponent->GetRelativeRotation() + FRotator(LookAxisVector.Y * -1.f, LookAxisVector.X, 0.f)).Quaternion()
		//);

		AddControllerYawInput(LookAxisVector.X);
		//AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AmagicisfrondCharacter::Landed(const FHitResult& Hit) {
	//Super::Landed(Hit);
	CanAirJump = true;
}

void AmagicisfrondCharacter::EndWallrun() {
	WRData.WallRuning = false;
	//WRData.WR_SIDE = NONE;
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller)) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
			Subsystem->RemoveMappingContext(WallrunMappingContext);
		}
	}
	GetCharacterMovement()->bConstrainToPlane = false;
	CanAirJump = true;
	bUseControllerRotationYaw = true;
}

void AmagicisfrondCharacter::WR_Movement(E_WR_Side side, float movement, FHitResult fhr) {
	//  The snappy behavior on curved surfaces should be fixed on tick with a turnrate or sm
	GetCharacterMovement()->StopMovementKeepPathing();
	FRotator RotationOf90Degrees(0, -90*side, 0);
	FRotator LeftOrRightDirection = RotationOf90Degrees.RotateVector(fhr.Normal).Rotation();
	FRotator newRotation(0, LeftOrRightDirection.Yaw, 0);
	SetActorRotation(newRotation, ETeleportType::None);
	FVector NewLoc = FRotationMatrix(fhr.Normal.Rotation()).GetScaledAxis(EAxis::Y) * movement *  20;
	if (side == LEFT) NewLoc = -NewLoc + GetActorLocation();
	else NewLoc = NewLoc + GetActorLocation();
	SetActorLocation(NewLoc); // , true, NULL, ETeleportType::TeleportPhysics);
}

void AmagicisfrondCharacter::WR_Move(const FInputActionValue& Value) {
	FVector2D MovementVector = Value.Get<FVector2D>();
	const int length = 100;
	struct FHitResult OutHit;
	const FVector Start = GetActorLocation() + GetActorForwardVector();
	const FVector End = Start + WRData.NormalHit * -1 * length;
	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), true, NULL);
	TraceParams.AddIgnoredActor(this);
	bool isWallFound = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, TraceParams);

	if (isWallFound) {
		WRData.NormalHit = OutHit.ImpactNormal;
		if (Controller != nullptr) {
			if (MovementVector.Y >= .5f) {
				if (WRData.WR_SIDE == RIGHT && MovementVector.X > .0f) WR_Movement(RIGHT, MovementVector.Y, OutHit);
				else if (WRData.WR_SIDE == LEFT && MovementVector.X < .0f) WR_Movement(LEFT, MovementVector.Y, OutHit);
				else EndWallrun();
			} else EndWallrun();
		}
	} else EndWallrun();
}

void AmagicisfrondCharacter::WR_Look(const FInputActionValue& Value) {
}

void AmagicisfrondCharacter::WR_dbjump(const FInputActionValue& Value) {
	EndWallrun();
	LaunchCharacter((GetActorForwardVector() + (WRData.WR_SIDE == LEFT ? 1 : -1) * GetActorRightVector()) * 1000, true, true);
}

void AmagicisfrondCharacter::StartWallrun(E_WR_Side Side, FVector normal) {
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller)) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
			Subsystem->AddMappingContext(WallrunMappingContext, 1);
		}
	}
	
	GetCharacterMovement()->bConstrainToPlane = true;
	bUseControllerRotationYaw = false;
	WRData.WallRuning = true;
	WRData.WR_SIDE = Side;
	WRData.NormalHit = normal;
	PlayerCamera->SetCameraSide(Side == LEFT ? ECameraSide::RIGHT : ECameraSide::LEFT);
	accTimeCamera = 0.f;
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
	if (!WRData.WallRuning && GetCharacterMovement()->IsFalling()) {
		if (GetActorRightVector().Dot(HitNormal) < 0.0f) StartWallrun(RIGHT, HitNormal);
		else if (GetActorRightVector().Dot(HitNormal) > 0.0f) StartWallrun(LEFT, HitNormal);
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