// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_IncrementPatrolPointIndex.h"
#include "AIController.h"
#include "AIEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PatrollPoint.h"

UBTTask_IncrementPatrolPointIndex::UBTTask_IncrementPatrolPointIndex()
{
	NodeName = TEXT("Increment Patrol Point Index");
	Direction = EDirectionType::EDT_Forward;
	BiDirection = true;
}

EBTNodeResult::Type UBTTask_IncrementPatrolPointIndex::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	int32 TotalPoints = AIEnemy->GetPatrolPoints()->GetPatrolPointsLength();

	int32 CurrentIndex = OwnerComp.GetBlackboardComponent()->GetValueAsInt("PatrolPointIndex");

	if (BiDirection)
	{
		if (CurrentIndex >= TotalPoints - 1 && Direction == EDirectionType::EDT_Forward)
		{
			Direction = EDirectionType::EDT_Reverse;
		}
		else if (CurrentIndex <= 0 && Direction == EDirectionType::EDT_Reverse)
		{
			Direction = EDirectionType::EDT_Forward;
		}
	}
	else
	{
		if (CurrentIndex >= TotalPoints - 1)
		{
			CurrentIndex = -1;
		}
	}
	OwnerComp.GetBlackboardComponent()->SetValueAsInt("PatrolPointIndex", (Direction == EDirectionType::EDT_Forward)? FMath::Abs(++CurrentIndex) : FMath::Abs(--CurrentIndex));
	return EBTNodeResult::Succeeded;
}
