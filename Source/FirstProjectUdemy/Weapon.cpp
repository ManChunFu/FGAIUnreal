// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "MainCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Enemy.h"
#include "GameFramework/DamageType.h"
#include "FGAIAssignment/AIEnemy.h"

AWeapon::AWeapon()
{
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetRootComponent());
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);


	bWeaponParticles = false;

	WeaponState = EWeaponState::EWS_Pickup;

	Damage = 25.0f;

}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapEnd);
}

void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (WeaponState == EWeaponState::EWS_Pickup && OtherActor)
	{
		AMainCharacter* MainCharacter = Cast<AMainCharacter>(OtherActor);
		if (MainCharacter)
		{
			MainCharacter->SetActiveOverlappingItem(this);
		}
	}
}

void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	if (OtherActor)
	{
		AMainCharacter* MainCharacter = Cast<AMainCharacter>(OtherActor);
		if (MainCharacter)
		{
			MainCharacter->SetActiveOverlappingItem(nullptr);
		}
	}
}

void AWeapon::Equip(AMainCharacter* Character)
{
	if (Character)
	{
		SetInstigator(Character->GetController());
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

		SkeletalMesh->SetSimulatePhysics(false);

		const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName("RightHandSocket");
		if (RightHandSocket)
		{
			RightHandSocket->AttachActor(this, Character->GetMesh());
			bRotate = false;
			// Check if player already has a weapon in hand
			if (Character->GetEquippedWeapon())
			{
				Character->EquippedWeapon->Destroy();
			}
			Character->SetEquippedWeapon(this);
			Character->SetActiveOverlappingItem(nullptr);
			Character->bIsArmed = true;
		}
		if (OnEquipSound)
		{
			UGameplayStatics::PlaySound2D(this, OnEquipSound);
		}
		if (!bWeaponParticles)
		{
			IdleParticlesComponent->Deactivate();
		}
		WeaponState = EWeaponState::EWS_Equipped;
	}
}

void AWeapon::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (OtherActor->Tags.Contains("AIEnemy"))
		{
			AAIEnemy* AIEnemy = Cast<AAIEnemy>(OtherActor);
			if (AIEnemy)
			{
				if (AIEnemy->HitParticle)
				{
					const USkeletalMeshSocket* WeaponSocket = SkeletalMesh->GetSocketByName("WeaponSocket");
					if (WeaponSocket)
					{
						FVector SocketLocation = WeaponSocket->GetSocketLocation(SkeletalMesh);
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), AIEnemy->HitParticle, SocketLocation, FRotator(0.0f), false);
					}
					if (AIEnemy->HitSound)
					{
						UGameplayStatics::PlaySound2D(this, AIEnemy->HitSound);
					}
					if (DamageTypeClass)
					{
						UGameplayStatics::ApplyDamage(AIEnemy, Damage, WeaponInstigator, this, DamageTypeClass);
					}
				}
			}
		}
		else
		{
			AEnemy* Enemy = Cast<AEnemy>(OtherActor);
			if (Enemy)
			{
				if (Enemy->HitParticle)
				{
					const USkeletalMeshSocket* WeaponSocket = SkeletalMesh->GetSocketByName("WeaponSocket");
					if (WeaponSocket)
					{
						FVector SocketLocation = WeaponSocket->GetSocketLocation(SkeletalMesh);
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticle, SocketLocation, FRotator(0.0f), false);
					}
					if (Enemy->HitSound)
					{
						UGameplayStatics::PlaySound2D(this, Enemy->HitSound);
					}
					if (DamageTypeClass)
					{
						UGameplayStatics::ApplyDamage(Enemy, Damage, WeaponInstigator, this, DamageTypeClass);
					}
				}
			}
		}
	}
}

void AWeapon::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AWeapon::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AWeapon::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
