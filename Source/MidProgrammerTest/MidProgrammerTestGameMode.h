#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MidProgrammerTestGameMode.generated.h"

UCLASS(minimalapi)
class AMidProgrammerTestGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMidProgrammerTestGameMode();

protected:
	virtual void BeginPlay() override;
};



