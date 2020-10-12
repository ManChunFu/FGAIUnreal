// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "AIController.h"
#include "BehaviorTree/BlackBoardComponent.h"
#include "AIEnemy.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	APawn* Target = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("Target"));
	if (Target)
	{
		AIEnemy->AIAttack(Target);
	}

	return EBTNodeResult::Succeeded;
}
