// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindPatrolPoint.h"
#include "AIEnemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PatrollPoint.h"

UBTTask_FindPatrolPoint::UBTTask_FindPatrolPoint()
{
	NodeName = TEXT("Find Patrol Point Location");
}

EBTNodeResult::Type UBTTask_FindPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	int32 index = OwnerComp.GetBlackboardComponent()->GetValueAsInt("PatrolPointIndex");
	FVector PointLocation = AIEnemy->GetPatrolPoints()->GetPatrolPoint(index);

	// transform this point to a world position using its parent
	FVector PointWorldLocation = AIEnemy->GetPatrolPoints()->GetActorTransform().TransformPosition(PointLocation);

	OwnerComp.GetBlackboardComponent()->SetValueAsVector("PatrolPointLocation", PointWorldLocation);

	return EBTNodeResult::Succeeded;
}
