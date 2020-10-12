// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_IncrementPatrolPointIndex.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EDirectionType :uint8
{
	EDT_Forward	UMETA(DisplayName = "Forward"),
	EDT_Reverse	UMETA(DisplayName = "Reverse"),

	EDT_MAX		UMETA(DisplayName = "Default_MAX")
};
UCLASS()
class FIRSTPROJECTUDEMY_API UBTTask_IncrementPatrolPointIndex : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_IncrementPatrolPointIndex();

	EDirectionType Direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	bool BiDirection;
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:

};
