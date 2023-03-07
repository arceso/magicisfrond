// Copyright Epic Games, Inc. All Rights Reserved.

#include "Moon_AbyssGameMode.h"
#include "Moon_AbyssCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMoon_AbyssGameMode::AMoon_AbyssGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
