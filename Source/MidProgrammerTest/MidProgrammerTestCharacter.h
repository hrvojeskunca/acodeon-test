#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Blueprint/UserWidget.h"
#include "HealthComponent.h"
#include "HPInterface.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "MidProgrammerTestCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UUserWidget;
class UHealthComponent;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Health", meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	UPROPERTY(BlueprintReadWrite, Category = "Character State")
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

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Explosion")
	UParticleSystem* ExplosionEffect;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastExplosion(const FVector& Location);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTriggerExplosion(const FVector& Location);

	UFUNCTION()
	void HandleExplosionDamage(const FVector& Location);

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

	UFUNCTION()
	void PrintMessage(FString Message);

	UFUNCTION(Category = "Utility")
	void DisableMovement();

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

	virtual void CharacterDead() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateHUD_HP(float MaxHP, float CurrentHP);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastUpdateHUD_HP(float MaxHP, float CurrentHP);

	UFUNCTION(Client, Reliable)
	void ClientUpdateHUD_HP(float MaxHP, float CurrentHP);

#pragma endregion

#pragma region Utility
public:

	UFUNCTION(BlueprintCallable, Category = "Utility")
	FVector GetWorldPositionFromScreenPosition(APlayerController* PlayerController, FVector2D ScreenPosition);

#pragma endregion

#pragma region Multiplayer

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma endregion
};

