#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Blueprint/UserWidget.h"
#include "HealthComponent.h"
#include "HPInterface.h"
#include "CMD_UpdateHUD.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "MidProgrammerTestCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UUserWidget;
struct FInputActionValue;

// ------------------------------------------------ Enums -------------------------------------------------

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Alive UMETA(DisplayName = "Alive"),
	Dead UMETA(DisplayName = "Dead")
};

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AMidProgrammerTestCharacter : public ACharacter, public IHPInterface
{
	GENERATED_BODY()

// ---------------------------------------------- Variables ----------------------------------------------

#pragma region Components

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float LocalMaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float LocalCurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character State")
	ECharacterState CharacterState;

#pragma endregion

#pragma region Input
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

#pragma endregion

#pragma region Explosion

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	UParticleSystem* ExplosionEffect;

#pragma endregion


#pragma region HUD

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	TSubclassOf<UUserWidget> HUDWidgetClass;

private:
	UUserWidget* HUDWidget;

#pragma endregion

// ---------------------------------------------- Functions ----------------------------------------------

#pragma region General

public:

	AMidProgrammerTestCharacter();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamageAction(float DamageAmount);

	UFUNCTION()
	void PrintMessage(FString Message);

	UFUNCTION(Category = "Utility")
	void DisableMovement();

	UFUNCTION(Category = "HUD")
	ACMD_UpdateHUD* CreateCommand(float MaxHP, float CurrentHP, AActor* OwningActor);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void DeleteCMDs();

protected:
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void BeginPlay() override;

#pragma endregion

#pragma region HUD

public:
	FVector2D GetCrosshairScreenPosition(APlayerController* PlayerController);

#pragma endregion

#pragma region Move&Look

protected:

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Fire();

#pragma endregion

#pragma region Getters

public:

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

#pragma endregion

#pragma region Interfaces
public:

	virtual void ApplyDamage(float DamageAmount) override;

	virtual void CharacterDead() override;

	virtual void UpdateUI_HP(float MaxHP, float CurrentHP) override;

#pragma endregion

#pragma region Commands
private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ACMD_UpdateHUD> CMD_UpdateHUDClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD", meta = (AllowPrivateAccess = "true"))
	TArray<ACMD_UpdateHUD*> SpawnedCMDs;

#pragma endregion

#pragma region Utility
public:

	UFUNCTION(BlueprintCallable, Category = "Utility")
	FVector GetWorldPositionFromScreenPosition(APlayerController* PlayerController, FVector2D ScreenPosition);

#pragma endregion
};

