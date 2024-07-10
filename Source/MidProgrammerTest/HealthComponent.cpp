#include "HealthComponent.h"
#include "GameFramework/Actor.h"
#include "HPInterface.h"
#include "Engine/Engine.h"
#include "MidProgrammerTestCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

UHealthComponent::UHealthComponent()
{
	MaxHealth = 120.0f;
	CurrentHealth = MaxHealth;

	SetIsReplicatedByDefault(true);
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
	}
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || CurrentHealth <= 0.0f)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
	
	if (GetOwner()->HasAuthority())
	{
		ServerUpdateHealth(CurrentHealth);
	}

	OnHealthChanged.Broadcast(MaxHealth, CurrentHealth);

	if (IHPInterface* HPInterface = Cast<IHPInterface>(GetOwner()))
	{
		IHPInterface::Execute_UpdateHUD_HP(GetOwner(), MaxHealth, CurrentHealth);
	}

	if (CurrentHealth <= 0.0f)
	{
		CharacterDead();
	}
}

void UHealthComponent::ServerUpdateHealth_Implementation(float NewHealth)
{
	CurrentHealth = NewHealth;
	OnHealthChanged.Broadcast(MaxHealth, CurrentHealth);
	if (IHPInterface* HPInterface = Cast<IHPInterface>(GetOwner()))
	{
		IHPInterface::Execute_UpdateHUD_HP(GetOwner(), MaxHealth, CurrentHealth);
	}
}

bool UHealthComponent::ServerUpdateHealth_Validate(float NewHealth)
{
	return true;
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, CurrentHealth);
	DOREPLIFETIME(UHealthComponent, MaxHealth);
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

void UHealthComponent::UpdateHUD_HP(float MaxHP, float CurrentHP)
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		IHPInterface* Interface = Cast<IHPInterface>(Owner);
		if (Interface)
		{
			IHPInterface::Execute_UpdateHUD_HP(Owner, MaxHP, CurrentHP);
		}
	}
}
