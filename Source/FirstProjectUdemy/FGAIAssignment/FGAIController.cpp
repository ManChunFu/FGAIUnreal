// Fill out your copyright notice in the Description page of Project Settings.


#include "FGAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "AIEnemy.h"
#include "../MainCharacter.h"

AFGAIController::AFGAIController()
{
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component")));
	EyeSightRadius = 500.0f;
	LoseEyeSightRadius = 50.0f;
	VisonAngleDegree = 90.0f;
	MemoryTime = 5.0f;
	SuccessRangeFromLastSeenLocation = 900.0f;

	SightConfig->SightRadius = EyeSightRadius;
	SightConfig->LoseSightRadius = SightConfig->SightRadius + LoseEyeSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = VisonAngleDegree;
	SightConfig->SetMaxAge(MemoryTime);  
	SightConfig->AutoSuccessRangeFromLastSeenLocation = SuccessRangeFromLastSeenLocation;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AFGAIController::SetTargetCaught);
	GetPerceptionComponent()->ConfigureSense(*SightConfig);

}

void AFGAIController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	// Get reference to the character itself
	AAIEnemy* AIEnemy = Cast<AAIEnemy>(InPawn);
	{
		if (AIEnemy && AIBehavior)
		{
			RunBehaviorTree(AIBehavior);
			GetBlackboardComponent()->SetValueAsVector(TEXT("StartLocation"), AIEnemy->GetActorLocation());
		}
	}
}

void AFGAIController::SetTargetCaught(AActor* Actor, FAIStimulus Stimulus)
{
	AMainCharacter* Player = Cast<AMainCharacter>(Actor);
	if (Player)
	{
		GetBlackboardComponent()->SetValueAsBool("CanSeePlayer", Stimulus.WasSuccessfullySensed());
	}
	if (Stimulus.WasSuccessfullySensed())
	{
		GetBlackboardComponent()->SetValueAsObject("Target", Player);
	}
	else
	{
		GetPerceptionComponent()->ForgetActor(Player);
		GetBlackboardComponent()->SetValueAsBool("CanSeePlayer", false);
		GetBlackboardComponent()->ClearValue("Target");
	}
}

void AFGAIController::GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const
{
	DeterminateAISightPerceptionViewPoint(Location, Rotation);
}




