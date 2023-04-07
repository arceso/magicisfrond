// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Grappable_Actor.generated.h"

enum class EGrappable_Type {
	Static,
	Enemy,
	Object
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MOON_ABYSS_API UGrappable_Actor : public UActorComponent {
	GENERATED_BODY()

public:	
	UGrappable_Actor();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere)
	UShapeComponent* GrappableTargetArea;

protected:
	virtual void BeginPlay() override;

private:
	EGrappable_Type Type;
	double Mass;
	bool isMovable;

public:	
	void SetType(EGrappable_Type Type);
	void SetMass(const double& Mass);
	void SetisMovable(bool isMovable);

	EGrappable_Type GetType();
	const double& GetMass();
	bool GetisMovable();

		
};
