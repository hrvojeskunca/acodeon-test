#include "HealthComponent.h"
#include "GameFramework/Actor.h"
#include "HPInterface.h"
#include "Engine/Engine.h"
#include "MidProgrammerTestCharacter.h"

UHealthComponent::UHealthComponent()
{
	MaxHealth = 120.0f;
	CurrentHealth = 120.0f;
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UHealthComponent::ApplyDamage(float DamageAmount)
{
	CurrentHealth -= DamageAmount;
	CheckCurrentHealth(CurrentHealth);
}

void UHealthComponent::CheckCurrentHealth(float Health)
{
	if (Health <= 0.0f)
	{
		CurrentHealth = 0.0f;
		CharacterDead();
		UpdateUI_HP(MaxHealth, CurrentHealth);
	}
	else
	{
		UpdateUI_HP(MaxHealth, CurrentHealth);
	}
}

void UHealthComponent::CharacterDead()
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		IHPInterface* Interface = Cast<IHPInterface>(Owner);
		if (Interface)
		{
			Interface->CharacterDead();
		}
	}
}

void UHealthComponent::UpdateUI_HP(float MaxHP, float CurrentHP)
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		IHPInterface* Interface = Cast<IHPInterface>(Owner);
		if (Interface)
		{
			Interface->UpdateUI_HP(MaxHP, CurrentHP);
		}
	}
}
