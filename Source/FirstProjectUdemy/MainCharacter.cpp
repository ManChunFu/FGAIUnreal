// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon.h"
#include "Components/InputComponent.h"
#include "Animation/AnimInstance.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "FirstProjectUdemySaveGame.h"
#include "ItemStorage.h"
#include "FGAIAssignment/AIEnemy.h"


// Sets default values
AMainCharacter::AMainCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create Camera Boom (pulls towards the player if there's a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.0f; // Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller

	// Set size for collision capsule
	GetCapsuleComponent()->SetCapsuleSize(34.0f, 100.0f);

	// Create FollowCamera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and left the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false;

	// Set our turn rates for input
	BaseTurnRate = 65.0f;
	BaseLookUpRate = 65.0f;

	// Don't rotate when the controller rotates.
	// Let that just affect the camera.
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Configure character movement 
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 840.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 650.0f;
	GetCharacterMovement()->AirControl = 0.2f;

	//Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	RunningSpeed = 650.0f;
	SprintingSpeed = 950.0f;
	bShiftKeyDown = false;
	bMovingForward = false;
	bMovingRight = false;
	bESCDown = false;

	MaxHealth = 100.0f;
	Health = 65.0f;
	MaxStamina = 150.0f;
	Stamina = 120.0f;
	Coins = 0;

	StaminaDrainRate = 25.0f;
	MinSprintStamina = 50.0f;

	InterpSpeed = 15.0f;
	InterpSpeed = false;

	bIsArmed = false;
	bAttacking = false;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());

	FString Map = GetWorld()->GetMapName();
	Map.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	if (Map != "SunTemple")
	{
		LoadGameNoSwitch();
		if (MainPlayerController)
		{
			MainPlayerController->GameModeOnly();
		}
	}
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementStatus == EMovementStatus::EMS_Dead)
	{
		return;
	}

	float DeltaStamina = StaminaDrainRate * DeltaTime;

	if (bShiftKeyDown)
	{
		if (bMovingForward || bMovingRight)
		{
			switch (StaminaStatus)
			{
			case EStaminaStatus::ESS_Normal:
				SetMovementStatus(EMovementStatus::EMS_Sprinting); // Affect animations
				if ((Stamina -= DeltaStamina) <= MinSprintStamina) // Affect stamina bar
				{
					SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				}
				break;
			case EStaminaStatus::ESS_BelowMinimum:
				if ((Stamina -= DeltaStamina) <= 0.0f)
				{
					Stamina = 0.0f;
					SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
				else
				{
					SetMovementStatus(EMovementStatus::EMS_Sprinting);
				}
				break;
			case EStaminaStatus::ESS_Exhausted:
				Stamina = 0;
				SetMovementStatus(EMovementStatus::EMS_Normal);
				break;
			case EStaminaStatus::ESS_ExhaustedRecovering:
				SetMovementStatus(EMovementStatus::EMS_Normal);
				break;
			default:
				;
			}
		}
	}
	else
	{
		switch (StaminaStatus)
		{
		case EStaminaStatus::ESS_Normal:
			SetMovementStatus(EMovementStatus::EMS_Normal);
			if ((Stamina += DeltaStamina) >= MaxStamina)
			{
				Stamina = MaxStamina;
			}
			break;
		case EStaminaStatus::ESS_BelowMinimum:
			SetMovementStatus(EMovementStatus::EMS_Normal);
			if ((Stamina += DeltaStamina) >= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
			}
			break;
		case EStaminaStatus::ESS_Exhausted:
			Stamina += DeltaStamina;
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
			SetMovementStatus(EMovementStatus::EMS_Normal);
			break;
		case EStaminaStatus::ESS_ExhaustedRecovering:
			SetMovementStatus(EMovementStatus::EMS_Normal);
			if ((Stamina += DeltaStamina) >= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
			}
			break;
		default:
			;
		}
	}

	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);
		SetActorRotation(InterpRotation);
	}

	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent); // check if it's valid

	// called character's own functions
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMainCharacter::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMainCharacter::ShiftKeyUp);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMainCharacter::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMainCharacter::ESCUp);


	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMainCharacter::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMainCharacter::LMBUp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);

	// Called controller that is inherited from Pawn class
	PlayerInputComponent->BindAxis("Turn", this, &AMainCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMainCharacter::LookUp);

	PlayerInputComponent->BindAxis("TurnRate", this, &AMainCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMainCharacter::LookUpAtRate);
}

void AMainCharacter::LookUp(float Value)
{
	if (CanMove(Value))
	{
		AddControllerPitchInput(Value);
	}
}

void AMainCharacter::Turn(float Value)
{
	if (CanMove(Value))
	{
		AddControllerYawInput(Value);
	}
}

bool AMainCharacter::CanMove(float Value)
{
	if (MovementStatus == EMovementStatus::EMS_Dead)
	{
		return false;
	}
	else if (MainPlayerController)
	{
		return (Value != 0.0f) && !bAttacking && !MainPlayerController->bPauseMenuVisible;
	}
	return true;
}


void AMainCharacter::MoveForward(float Value)
{
	bMovingForward = false;

	if (CanMove(Value))
	{
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
		bMovingForward = true;
	}
}

void AMainCharacter::MoveRight(float Value)
{
	bMovingRight = false;

	if (CanMove(Value))
	{
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
		bMovingRight = true;
	}
}

void AMainCharacter::TurnAtRate(float Rate)
{
	if (MovementStatus == EMovementStatus::EMS_Dead)
	{
		return;
	}

	// Turn 65 degrees per second
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::LookUpAtRate(float Rate)
{
	if (MovementStatus == EMovementStatus::EMS_Dead)
	{
		return;
	}

	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}


void AMainCharacter::DecrementHealth(float Amout)
{
	if (Health > 0.0f)
	{
		if ((Health -= Amout) <= 0.0f)
		{
			Health = 0;
			Die();
		}
	}
}

float AMainCharacter::TakeDamage(float DamageAmout, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	DecrementHealth(DamageAmout);

	return DamageAmout;
}

void AMainCharacter::Jump()
{
	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		if (MainPlayerController)
		{
			if (MainPlayerController->bPauseMenuVisible) return;
		}
		Super::Jump();
	}
}

void AMainCharacter::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		int32 Selection = FMath::RandRange(0, 2);
		switch (Selection)
		{
		case 0:
			AnimInstance->Montage_Play(DeathMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Death_1"), DeathMontage);
			break;
		case 1:
			AnimInstance->Montage_Play(DeathMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Death_2"), DeathMontage);
			break;
		case 2:
			AnimInstance->Montage_Play(DeathMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Death_3"), DeathMontage);
			break;
		default:
			;
		}
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);
	GetMovementComponent()->StopMovementImmediately();

	//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMainCharacter::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

void AMainCharacter::UpdateCombatTarget()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, EnemyFilter);

	if (OverlappingActors.Num() <= 0)
	{
		if (MainPlayerController)
		{
			MainPlayerController->RemoveEnemyHealthBar();
		}
		return;
	}

	float DistanceToActor = 0;
	float MinDistance = 99999;
	ACharacter* ClosestEnemy = nullptr;

	for (auto Actor : OverlappingActors)
	{
		FVector SelfLocation = GetActorLocation();
		if (Actor->Tags.Contains("AIEnemy"))
		{
			AAIEnemy* AIEnemy = Cast<AAIEnemy>(Actor);
			DistanceToActor = (AIEnemy->GetActorLocation() - SelfLocation).Size();
			if (DistanceToActor < MinDistance)
			{
				MinDistance = DistanceToActor;
				ClosestEnemy = AIEnemy;
			}
		}
		else
		{
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			DistanceToActor = (Enemy->GetActorLocation() - SelfLocation).Size();
			if (DistanceToActor < MinDistance)
			{
				MinDistance = DistanceToActor;
				ClosestEnemy = Enemy;
			}
		}
	}

	SetCombatTarget(ClosestEnemy);
	if (MainPlayerController)
	{
		MainPlayerController->DisplayEnemyHealthBar();
	}

}

void AMainCharacter::SaveGame()
{
	UFirstProjectUdemySaveGame* SaveGameInstance = Cast<UFirstProjectUdemySaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstProjectUdemySaveGame::StaticClass()));

	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Coins = Coins;

	if (EquippedWeapon)
	{
		SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->Name;
	}

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	SaveGameInstance->CharacterStats.LevelName = MapName;

	SaveGameInstance->CharacterStats.Locaion = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
}

void AMainCharacter::LoadGame(bool SetPosition)
{
	UFirstProjectUdemySaveGame* LoadGameInstance = Cast<UFirstProjectUdemySaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstProjectUdemySaveGame::StaticClass()));

	LoadGameInstance = Cast<UFirstProjectUdemySaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (LoadGameInstance->CharacterStats.LevelName != "" && LoadGameInstance->CharacterStats.LevelName != GetWorld()->GetMapName())
	{
		UGameplayStatics::OpenLevel(GetWorld(), FName(*LoadGameInstance->CharacterStats.LevelName));
	}

	if (LoadGameInstance->CharacterStats.WeaponName != "")
	{
		if (WeaponStorage)
		{
			AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
			if (Weapons)
			{
				if (Weapons->WeaponMap.Contains(LoadGameInstance->CharacterStats.WeaponName))
				{
					AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[LoadGameInstance->CharacterStats.WeaponName]);
					if (WeaponToEquip)
					{
						WeaponToEquip->Equip(this);
					}
				}
			}
		}
	}

	if (SetPosition)
	{
		SetActorLocation(LoadGameInstance->CharacterStats.Locaion);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}

void AMainCharacter::LoadGameNoSwitch()
{
	UFirstProjectUdemySaveGame* LoadGameInstance = Cast<UFirstProjectUdemySaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstProjectUdemySaveGame::StaticClass()));

	LoadGameInstance = Cast<UFirstProjectUdemySaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (LoadGameInstance->CharacterStats.WeaponName != "")
	{
		if (WeaponStorage)
		{
			AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
			if (Weapons)
			{
				if (Weapons->WeaponMap.Contains(LoadGameInstance->CharacterStats.WeaponName))
				{
					AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[LoadGameInstance->CharacterStats.WeaponName]);
					if (WeaponToEquip)
					{
						WeaponToEquip->Equip(this);
					}
				}
			}
		}
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}

void AMainCharacter::IncrementCoins(int32 Amout)
{
	Coins += Amout;
}

void AMainCharacter::IncrementHealth(float Amout)
{
	if ((Health += Amout) >= MaxHealth)
	{
		Health = MaxHealth;
	}
}

void AMainCharacter::ShowPickupLocation()
{
	/*for (int32 index = 0; index < PickupLocations.Num(); index++)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, PickupLocations[index], 25.0f, 8, FLinearColor::Blue, 10.0f, 0.5f);
	}*/
	if (PickupLocations.Num() > 0)
	{
		for (FVector Location : PickupLocations)
		{
			UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.0f, 8, FLinearColor::Blue, 10.0f, 0.5f);
		}
	}
}

void AMainCharacter::PlaySwingSound()
{
	if (EquippedWeapon->SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}

void AMainCharacter::ShiftKeyDown()
{
	bShiftKeyDown = true;
}

void AMainCharacter::ShiftKeyUp()
{
	bShiftKeyDown = false;
}

void AMainCharacter::Attack()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead)
	{
		bAttacking = true;
		SetInterpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage)
		{
			AnimInstance->Montage_Play(CombatMontage, 2.0f);
			AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
			/*int32 Selection = FMath::RandRange(0, 1);
			switch(Selection)
			{
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 2.0f);
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
				break;
			case 1:
				AnimInstance->Montage_Play(CombatMontage, 1.35f);
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
				break;
			default:
				;
			}*/
		}
	}
}

void AMainCharacter::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);
	if (bLMBDown)
	{
		Attack();
	}
}

void AMainCharacter::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

FRotator AMainCharacter::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.0f, LookAtRotation.Yaw, 0.0f);
	return LookAtRotationYaw;
}

void AMainCharacter::LMBDown()
{
	bLMBDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead)
	{
		return;
	}

	if (MainPlayerController)
	{
		if (MainPlayerController->bPauseMenuVisible) return;

		if (ActiveOverlappingItem)
		{
			AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
			if (Weapon)
			{
				Weapon->Equip(this);
				SetActiveOverlappingItem(nullptr);
			}
		}
		else
		{
			if ((bIsArmed))
				Attack();
		}
	}
}

void AMainCharacter::LMBUp()
{
	bLMBDown = false;
}

void AMainCharacter::ESCDown()
{
	bESCDown = true;

	if (MainPlayerController)
	{
		MainPlayerController->TogglePauseMenu();
	}
}

void AMainCharacter::ESCUp()
{
	bESCDown = false;

}


