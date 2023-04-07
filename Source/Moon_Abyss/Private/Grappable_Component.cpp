// Fill out your copyright notice in the Description page of Project Settings.


#include "Grappable_Component.h"

// Sets default values for this component's properties
UGrappable_Component::UGrappable_Component()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	GrappableTargetArea = CreateDefaultSubobject<UShapeComponent>(TEXT("Area Box"));
	if (GrappableTargetArea) {
		GrappableTargetArea->InitializeComponent();
		GrappableTargetArea->SetCollisionProfileName(TEXT("BlockAll"));
		GrappableTargetArea->SetupAttachment((this));
	}
	// ...
}


// Called when the game starts
void UGrappable_Component::BeginPlay()
{
	Super::BeginPlay();
	if (GrappableTargetArea) {
		GrappableTargetArea->InitializeComponent();
		GrappableTargetArea->SetCollisionProfileName(TEXT("BlockAll"));
		GrappableTargetArea->SetupAttachment((this));
	}

	// ...
	
}


// Called every frame
void UGrappable_Component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

