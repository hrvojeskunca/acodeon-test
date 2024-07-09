#include "MidProgrammerTestCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "HealthComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "GameFramework/InputSettings.h"
#include "CMD_UpdateHUD.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AMidProgrammerTestCharacter::AMidProgrammerTestCharacter()
{
	// HUD
	HUDWidget = nullptr;
	
	// Health Component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	LocalMaxHealth = 120.0f;
	LocalCurrentHealth = 120.0f;

	// Health State
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

	// Create HUD & AddToViewport
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (HUDWidget)
		{
			UpdateUI_HP(LocalMaxHealth, LocalCurrentHealth);
			HUDWidget->AddToViewport();
		}
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

void AMidProgrammerTestCharacter::ApplyDamageAction(float DamageAmount)
{
	ApplyDamage(DamageAmount);
}

void AMidProgrammerTestCharacter::ApplyDamage(float DamageAmount)
{
	if (HealthComponent && CharacterState == ECharacterState::Alive)
	{
		HealthComponent->ApplyDamage(DamageAmount);
	}
}

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

void AMidProgrammerTestCharacter::UpdateUI_HP(float MaxHP, float CurrentHP)
{
	AMidProgrammerTestCharacter* ThisCharacter = this;

	ACMD_UpdateHUD* NewCMD = CreateCommand(MaxHP, CurrentHP, ThisCharacter);
	if (NewCMD && HUDWidget)
	{
		FName FunctionName = TEXT("WBP_UpdateHUD_HP");
		if (UFunction* Function = HUDWidget->FindFunction(FunctionName))
		{
			struct FCustomThunk
			{
				ACMD_UpdateHUD* CMD;
			};

			FCustomThunk Parms;
			Parms.CMD = NewCMD;

			HUDWidget->ProcessEvent(Function, &Parms);
		}
	}
}

ACMD_UpdateHUD* AMidProgrammerTestCharacter::CreateCommand(float MaxHP, float CurrentHP, AActor* OwningActor)
{
	if (CMD_UpdateHUDClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwningActor;

		FVector SpawnLocation = GetActorLocation();
		FRotator SpawnRotation = GetActorRotation();

		ACMD_UpdateHUD* NewCMD = GetWorld()->SpawnActor<ACMD_UpdateHUD>(CMD_UpdateHUDClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (NewCMD)
		{
			NewCMD->OwningActor = OwningActor;
			NewCMD->SetHP(EHPType::Max, MaxHP);
			NewCMD->SetHP(EHPType::Current, CurrentHP);

			SpawnedCMDs.Add(NewCMD);
		}

		return NewCMD;
	}
	
	return nullptr;
}

void AMidProgrammerTestCharacter::DeleteCMDs()
{
	for (ACMD_UpdateHUD* CMD : SpawnedCMDs)
	{
		if (CMD)
		{
			CMD->Destroy();
		}
	}
	SpawnedCMDs.Empty();
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
	}
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

	FVector EndLocation = WorldLocation + (WorldDirection * 3000.0f); // Adjust the range as necessary

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
}

#pragma endregion
