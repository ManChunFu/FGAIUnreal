// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_RandomLocation.h"
#include "AIEnemy.h"
#include "BehaviorTree/BlackBoardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"


UBTTask_RandomLocation::UBTTask_RandomLocation()
{
	NodeName = TEXT("Random Location");
}

EBTNodeResult::Type UBTTask_RandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AAIEnemy* AIEnemy = Cast<AAIEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	if (AIEnemy == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	FVector SelfLocation = AIEnemy->GetActorLocation();

	return EBTNodeResult::Type();
}
