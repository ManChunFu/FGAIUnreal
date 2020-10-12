// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrollPoint.generated.h"

UCLASS()
class FIRSTPROJECTUDEMY_API APatrollPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APatrollPoint();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (MakeEditWidget = "true"))
	TArray<FVector> PatrolPoints;
public:
	FORCEINLINE FVector GetPatrolPoint(int index) { return PatrolPoints[index]; }
	FORCEINLINE int32 GetPatrolPointsLength() { return PatrolPoints.Num(); }

};
