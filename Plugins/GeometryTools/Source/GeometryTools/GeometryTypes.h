// Bartender Ender
// Naughty Panda @ 2022

#pragma once

#include "CoreMinimal.h"
#include "GeometryTypes.generated.h"

/**
 * Geometry tools types.
 */

UENUM(BlueprintType)
enum class ETools_MirrorAxis : uint8
{
	X,
	Y,
	Z
};

UENUM(BlueprintType)
enum class ETools_WindowType : uint8
{
	None,
	Rectangle,
	Circle
};

UENUM(BlueprintType)
enum class ETools_WallBorderType : uint8
{
	None,
	Upper,
	Lower,
	Both
};

UENUM(BlueprintType)
enum class ETools_StairsStepsType : uint8
{
	AdaptiveSteps,
	FixedSteps
};
