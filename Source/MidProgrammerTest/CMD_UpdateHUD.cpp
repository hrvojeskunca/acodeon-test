#include "CMD_UpdateHUD.h"

// Sets default values
ACMD_UpdateHUD::ACMD_UpdateHUD()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MaxHP = 120.0f;
	CurrentHP = 120.0f;
    OwningActor = nullptr;

}

// Called when the game starts or when spawned
void ACMD_UpdateHUD::BeginPlay()
{
	Super::BeginPlay();
	
}

float ACMD_UpdateHUD::GetHP(EHPType HPType) const
{
    switch (HPType)
    {
    case EHPType::Current:
        return CurrentHP;
    case EHPType::Max:
        return MaxHP;
    default:
        return 0.0f;
    }
}

void ACMD_UpdateHUD::SetHP(EHPType HPType, float NewHP)
{
    switch (HPType)
    {
    case EHPType::Current:
        CurrentHP = NewHP;
        break;
    case EHPType::Max:
        MaxHP = NewHP;
        break;
    default:
        break;
    }
}

// Called every frame
void ACMD_UpdateHUD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

