// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_ChangeSpeed.h"
#include "AIEnemy.h"
#include "FGAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTService_ChangeSpeed::UBTService_ChangeSpeed()
{
	NodeName = TEXT("Change Speed");
	bNotifyBecomeRelevant = true;
	Speed = 600.0f;
}

void UBTService_ChangeSpeed::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	if (!OwnerComp.GetAIOwner())
	{
		return;
	}

	AAIEnemy* AIEnemy = Cast<AAIEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	if (!AIEnemy)
	{
		return;
	}

	AIEnemy->GetCharacterMovement()->MaxWalkSpeed = Speed;
}

FString UBTService_ChangeSpeed::GetStaticServiceDescription() const
{
	return FString("Change the AIEnemy speed");
}
