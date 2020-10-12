// Fill out your copyright notice in the Description page of Project Settings.


#include "AIEnemy.h"
#include "Components/SphereComponent.h"
#include "../MainCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"
#include "../MainCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "GameFramework/DamageType.h"
#include "Components/CapsuleComponent.h"
#include "../MainPlayerController.h"
#include "Perception/AISense_Sight.h"
#include "FGAIController.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"


// Sets default values
AAIEnemy::AAIEnemy()
{
	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(75.0f);

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetMesh(), FName("EnemySocket"));

	Stimulus = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("Stimulus"));
	Stimulus->RegisterForSense(TSubclassOf<UAISense_Sight>());
	Stimulus->RegisterWithPerceptionSystem();

	Health = 75.0f;
	MaxHealth = 100.0f;
	Damage = 10.0f;

	AttackMinTime = 0.5f;
	AttackMaxTime = 3.5f;

	DeathDelay = 3.0f;

	TagName = TEXT("AIEnemy");

}

// Called when the game starts or when spawned
void AAIEnemy::BeginPlay()
{
	Super::BeginPlay();

	const USkeletalMeshSocket* EyeSocket = GetMesh()->GetSocketByName("EyeSocket");

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AAIEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AAIEnemy::CombatSphereOnOverlapEnd);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AAIEnemy::ClawOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AAIEnemy::ClawOnOverlapEnd);


	Tags.Add(TagName);
}

float AAIEnemy::TakeDamage(float DamageAmout, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if ((Health -= DamageAmout) <= 0.0f)
	{
		Health = 0.0f;
		Die();

	}
	return DamageAmout;
}

void AAIEnemy::Die()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}

	DetachFromControllerPendingDestroy();
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CombatTarget->UpdateCombatTarget();
}

void AAIEnemy::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AAIEnemy::Disappear, DeathDelay);
}

void AAIEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive())
	{
		AMainCharacter* MainCharacter = Cast<AMainCharacter>(OtherActor);
		if (MainCharacter)
		{
			MainCharacter->SetCombatTarget(this);
			MainCharacter->UpdateCombatTarget();
			CombatTarget = MainCharacter;
		}
	}
}

void AAIEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AMainCharacter* MainCharacter = Cast<AMainCharacter>(OtherActor);
		if (MainCharacter)
		{
			if (MainCharacter->CombatTarget == this)
			{
				MainCharacter->SetCombatTarget(nullptr);
				MainCharacter->UpdateCombatTarget();
			}
			CombatTarget = nullptr;
		}
	}
}


void AAIEnemy::ClawOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMainCharacter* MainCharacter = Cast<AMainCharacter>(OtherActor);
		if (MainCharacter)
		{
			if (MainCharacter->HitParticle)
			{
				const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("TipSocket");
				if (TipSocket)
				{
					FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MainCharacter->HitParticle, SocketLocation, FRotator(0.0f), false);
				}
				if (MainCharacter->HitSound)
				{
					UGameplayStatics::PlaySound2D(this, MainCharacter->HitSound);
				}
				if (DamageTypeClass)
				{
					UGameplayStatics::ApplyDamage(MainCharacter, Damage, GetController(), this, DamageTypeClass);
				}
			}
		}
	}
}

void AAIEnemy::ClawOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
}

void AAIEnemy::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AAIEnemy::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AAIEnemy::AttackEnd()
{
	bAttacking = false;
}

void AAIEnemy::PlaySwingSound()
{
	if (SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

bool AAIEnemy::Alive()
{
	return (Health > 0);
}

void AAIEnemy::Disappear()
{
	Destroy();
}

void AAIEnemy::AIAttack(APawn* Target)
{
	CombatTarget = Cast<AMainCharacter>(Target);
	if (!CombatTarget)
	{
		return;
	}

	if (Alive() && CombatTarget->MovementStatus != EMovementStatus::EMS_Dead)
	{
		if (!bAttacking)
		{
			bAttacking = true;
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				if (CombatMontage)
				{
					AnimInstance->Montage_Play(CombatMontage, 1.35f);
					AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
				}
			}
		}
	}
}

