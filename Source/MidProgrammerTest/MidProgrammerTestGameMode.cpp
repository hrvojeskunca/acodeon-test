// Copyright Epic Games, Inc. All Rights Reserved.

#include "MidProgrammerTestGameMode.h"
#include "MidProgrammerTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMidProgrammerTestGameMode::AMidProgrammerTestGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TestAssets/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
