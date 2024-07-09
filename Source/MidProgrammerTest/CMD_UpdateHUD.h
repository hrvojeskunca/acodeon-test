#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CMD_UpdateHUD.generated.h"

UENUM(BlueprintType)
enum class EHPType : uint8
{
	Current,
	Max
};

UCLASS()
class MIDPROGRAMMERTEST_API ACMD_UpdateHUD : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACMD_UpdateHUD();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// HP update
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float CurrentHP;

	UPROPERTY(BlueprintReadWrite, Category = "Utility")
	AActor* OwningActor;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHP(EHPType HPType) const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHP(EHPType HPType, float NewHP);
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
