// Bartender Ender
// Naughty Panda @ 2022

#include "BakedStaticMeshActor.h"

ABakedStaticMeshActor::ABakedStaticMeshActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}
