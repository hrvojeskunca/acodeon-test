#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HPInterface.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MIDPROGRAMMERTEST_API UHealthComponent : public UActorComponent, public IHPInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	//HPInterface
	virtual void ApplyDamage(float DamageAmount) override;
	
	virtual void CharacterDead() override;

	virtual void UpdateUI_HP(float MaxHP, float CurrentHP) override;

	// Health properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealth;

	UFUNCTION()
	void CheckCurrentHealth(float Health);
};
