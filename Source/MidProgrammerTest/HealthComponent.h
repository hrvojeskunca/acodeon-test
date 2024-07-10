#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HPInterface.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, MaxHealth, float, CurrentHealth);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MIDPROGRAMMERTEST_API UHealthComponent : public UActorComponent, public IHPInterface
{
	GENERATED_BODY()

public:	
	
	UHealthComponent();

protected:
	
	virtual void BeginPlay() override;


public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Health")
	float MaxHealth;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Health")
	float CurrentHealth;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;

	virtual void CharacterDead() override;

	void UpdateHUD_HP(float MaxHP, float CurrentHP);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUpdateHealth(float NewHealth);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
