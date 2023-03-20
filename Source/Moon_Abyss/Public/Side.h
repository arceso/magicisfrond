#pragma once

#include "UObject/ObjectMacros.h"

//UENUM(BlueprintType)
enum class ESide: int {
	None = 0,//	UMETA(DisplayName = "None"),
	Left = 1,//	UMETA(DisplayName = "Left"),
	Right = 2,//	UMETA(DisplayName = "Right"),
	Center = 3,
	MAX = 4
};