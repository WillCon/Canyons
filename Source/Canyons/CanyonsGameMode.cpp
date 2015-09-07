// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Canyons.h"
#include "CanyonsGameMode.h"
#include "CanyonsCharacter.h"

ACanyonsGameMode::ACanyonsGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
