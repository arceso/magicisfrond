#pragma once

#include "UObject/ObjectMacros.h"

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_NONE				UMETA(Hidden),
	CMOVE_Wallruning		UMETA(DisplayName = "Wallruning"),
	CMOVE_Grappling			UMETA(DisplayName = "Grappling"),
	CMOVE_Crouching			UMETA(DisplayName = "Crouching"),
	CMOVE_Sliding			UMETA(DisplayName = "Sliding"),
	CMOVE_Sprinting			UMETA(DisplayName = "Sprinting"),
	CMOVE_MAX				UMETA(Hidden),
};