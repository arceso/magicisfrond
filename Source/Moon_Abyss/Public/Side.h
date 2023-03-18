#pragma once

#include "UObject/ObjectMacros.h"

//UENUM(BlueprintType)
enum class ESide {
	NONE = 0,//	UMETA(DisplayName = "None"),
	LEFT = -1,//	UMETA(DisplayName = "Left"),
	RIGHT = 1,//	UMETA(DisplayName = "Right"),
};