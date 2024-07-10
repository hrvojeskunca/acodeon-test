#include "MidProgrammerTestCharacter.h"

#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"

#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/InputSettings.h"

#include "Camera/CameraComponent.h"

#include "Components/CapsuleComponent.h"
#include "HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "InputMappingContext.h"

#include "Blueprint/UserWidget.h"

#include "Kismet/GameplayStatics.h"

#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AMidProgrammerTestCharacter::AMidProgrammerTestCharacter()
{
	HUDWidget = nullptr;
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	CharacterState = ECharacterState::Alive;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; 
	CameraBoom->bUsePawnControlRotation = true; 

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; 
	
	bReplicates = true;
}	

void AMidProgrammerTestCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetController<APlayerController>();
	if (PC)
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("PC is set!"));
	}

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (HUDWidget)
		{
			float DefaultMaxHealth = 120.0f;
			float DefaultCurrentHealth = DefaultMaxHealth;

			UpdateHUD_HP(DefaultMaxHealth, DefaultCurrentHealth);
			HUDWidget->AddToViewport();
		}
	}

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &AMidProgrammerTestCharacter::UpdateHUD_HP);
	}
}

void AMidProgrammerTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMidProgrammerTestCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMidProgrammerTestCharacter::Look);

		// FireAction
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AMidProgrammerTestCharacter::Fire);

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

#pragma region Move&Look

void AMidProgrammerTestCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMidProgrammerTestCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

#pragma endregion


#pragma region Health

void AMidProgrammerTestCharacter::CharacterDead()
{
	CharacterState = ECharacterState::Dead;
	DisableMovement();

	UE_LOG(LogTemp, Log, TEXT("Character Dieded!"));
	FString Message = "Character Dieded!";
	PrintMessage(Message);
}

#pragma endregion


#pragma region HUD

void AMidProgrammerTestCharacter::UpdateHUD_HP_Implementation(float MaxHP, float CurrentHP)
{
	if (HasAuthority())
	{
		ClientUpdateHUD_HP(MaxHP, CurrentHP);
	}
	else
	{
		MulticastUpdateHUD_HP(MaxHP, CurrentHP);
	}
}

void AMidProgrammerTestCharacter::MulticastUpdateHUD_HP_Implementation(float MaxHP, float CurrentHP)
{
	if (HUDWidget)
	{
		FName FunctionName = "WBP_UpdateHealth";
		UFunction* Function = HUDWidget->FindFunction(FunctionName);
		if (Function)
		{
			struct FCustomThunk
			{
				FVector2D HPVector;
			};

			FCustomThunk Parms;
			Parms.HPVector = FVector2D(MaxHP, CurrentHP);

			HUDWidget->ProcessEvent(Function, &Parms);
		}
	}
	else
	{
		FString ErrorMessage = TEXT("HUDWidget is null. Cannot update HUD.");
		PrintMessage(ErrorMessage);
	}
}

void AMidProgrammerTestCharacter::ClientUpdateHUD_HP_Implementation(float MaxHP, float CurrentHP)
{
	MulticastUpdateHUD_HP(MaxHP, CurrentHP);
}


#pragma endregion


#pragma region Attack

void AMidProgrammerTestCharacter::Fire()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController) return;

	FVector2D CrosshairScreenPosition = GetCrosshairScreenPosition(PlayerController);
	FVector ExplosionWorldPosition = GetWorldPositionFromScreenPosition(PlayerController, CrosshairScreenPosition);

	if (HasAuthority())
	{
		MulticastExplosion(ExplosionWorldPosition);
	}
	else
	{
		ServerTriggerExplosion(ExplosionWorldPosition);
	}
}

void AMidProgrammerTestCharacter::MulticastExplosion_Implementation(const FVector& Location)
{
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, Location);

		HandleExplosionDamage(Location);
	}
}

void AMidProgrammerTestCharacter::HandleExplosionDamage(const FVector& Location)
{
	float DamageAmount = 30.0f;
	float DamageRadius = 200.f;

	UGameplayStatics::ApplyRadialDamage(
		this,                   // World context
		DamageAmount,           // BaseDamage
		Location,               // Origin
		DamageRadius,           // Damage Radius
		nullptr,                // DamageTypeClass
		TArray<AActor*>(),      // Actors to ignore
		this,                   // Damage Causer
		GetInstigatorController(), // Instigator Controller
		true,                   // Do full damage
		ECC_Visibility          // Damage prevention channel
	);
}

bool AMidProgrammerTestCharacter::ServerTriggerExplosion_Validate(const FVector& Location)
{
	return true;
}

void AMidProgrammerTestCharacter::ServerTriggerExplosion_Implementation(const FVector& Location)
{
	MulticastExplosion(Location);
}

FVector2D AMidProgrammerTestCharacter::GetCrosshairScreenPosition(APlayerController* PlayerController)
{
	if (HUDWidget)
	{
		FName FunctionName = "WBP_Fetch_CrosshairPosition";
		UFunction* Function = HUDWidget->FindFunction(FunctionName);

		if (Function)
		{
			FVector2D ReturnValue;

			HUDWidget->ProcessEvent(Function, &ReturnValue);

			return ReturnValue;
		}
	}

	return FVector2D::ZeroVector;
}

FVector AMidProgrammerTestCharacter::GetWorldPositionFromScreenPosition(APlayerController* PlayerController, FVector2D ScreenPosition)
{
	if (!PlayerController) return FVector::ZeroVector;

	FVector WorldLocation, WorldDirection;
	PlayerController->DeprojectScreenPositionToWorld(ScreenPosition.X, ScreenPosition.Y, WorldLocation, WorldDirection);

	FVector EndLocation = WorldLocation + (WorldDirection * 3000.0f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PlayerController->GetPawn());

	if (PlayerController->GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, EndLocation, ECC_Visibility, Params))
	{
		return HitResult.Location;
	}

	return EndLocation;
}

#pragma endregion


#pragma region Utility

void AMidProgrammerTestCharacter::DisableMovement()
{
	APlayerController* PlayerController = GetController<APlayerController>();

	if (PlayerController)
	{
		if (PlayerController->GetPawn() == this)
		{
			DisableInput(PlayerController);
			UE_LOG(LogTemp, Log, TEXT("Input Disabled"));
			PrintMessage("Input Disabled");
		}
	}
}

void AMidProgrammerTestCharacter::PrintMessage(FString Message)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message);
	}
}

#pragma endregion

#pragma region Multiplayer

void AMidProgrammerTestCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMidProgrammerTestCharacter, ExplosionEffect);
	DOREPLIFETIME(AMidProgrammerTestCharacter, HealthComponent);
}

#pragma endregion
