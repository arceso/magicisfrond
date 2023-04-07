// Fill out your copyright notice in the Description page of Project Settings.


#include "Grappable_Actor.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "CollisionShape.h"

UGrappable_Actor::UGrappable_Actor():Super(){
	PrimaryComponentTick.bCanEverTick = true;
	//GetOwner()->Tags.Add("Character.General.Grappable");
	/*FVector ABStart, ABEnd;

	GetOwner()->GetActorBounds(true, ABStart, ABEnd);*/

	GrappableTargetArea = CreateDefaultSubobject<UShapeComponent>(TEXT("Area Box"));
}

// Called when the game starts
void UGrappable_Actor::BeginPlay() {
	Super::BeginPlay();
	if (GrappableTargetArea) {
		GrappableTargetArea->Activate(true);
		GrappableTargetArea->InitializeComponent();
		GrappableTargetArea->RegisterComponent();
		GrappableTargetArea->SetupAttachment(GetOwner()->GetRootComponent());
		GrappableTargetArea->SetRelativeLocation(GetOwner()->GetActorLocation());
		GrappableTargetArea->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
		GetOwner()->Tags.Add(TEXT("Grappable"));
	}	
}

// Called every frame
void UGrappable_Actor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGrappable_Actor::SetType(EGrappable_Type NewType) {
	Type = NewType;
}

void UGrappable_Actor::SetMass(const double& NewMass) {
	Mass = NewMass;

}

void UGrappable_Actor::SetisMovable(bool NewIsMovable) {
	isMovable = NewIsMovable;
}

EGrappable_Type UGrappable_Actor::GetType() {
	return Type;
}

const double& UGrappable_Actor::GetMass() {
	return Mass;
}

bool UGrappable_Actor::GetisMovable() {
	return isMovable;
}