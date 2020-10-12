// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "FGAIController.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPROJECTUDEMY_API AFGAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AFGAIController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBehaviorTree* AIBehavior;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI | Perception")
	class UAISenseConfig_Sight* SightConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI | Perception")
	float EyeSightRadius;

	/** EyeSightRaidus + given value*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI | Perception")
	float LoseEyeSightRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI | Perception")
	float VisonAngleDegree;

	/** The time that sets to Perception stimuli to forget after seeing*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI | Perception")
	float MemoryTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Perception")
	float SuccessRangeFromLastSeenLocation;

public:
	virtual void SetPawn(APawn* InPawn) override;
	
	UFUNCTION()
	void SetTargetCaught(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION(BlueprintImplementableEvent, Category = "AI | Perception")
	void DeterminateAISightPerceptionViewPoint(FVector& Location, FRotator& Rotation) const;

	void GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const override;

};
