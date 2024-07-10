// All Rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HPInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHPInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MIDPROGRAMMERTEST_API IHPInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void CharacterDead() = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateHUD_HP(float MaxHealth, float CurrentHealth);
};
