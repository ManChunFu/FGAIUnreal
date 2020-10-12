// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Floater.generated.h"

class UStaticMeshComponent;
UCLASS()
class FIRSTPROJECTUDEMY_API AFloater : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFloater();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ActorMeshComponents")
	UStaticMeshComponent* StaticMesh;

	// Location used by SetActorLocation() when BeginPlay() is called
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FloaterVariables")
	FVector InitialLocation;
	// Location of the Actor when dragged in from the editor
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "FloaterVariables")
	FVector PlacedLocation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "FloaterVariables")
	FVector WorldOrigin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloaterVariables")
	FVector InitialDirection;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FloaterVariables")
	FVector InitialForce;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FloaterVariables")
	FVector InitialTorque;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloaterVariables")
	bool bShouldFloat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FloaterVariables")
	bool bInitializeFloaterLocations;

	// Amplitude is how much we oscillate up and down
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloaterVariables")
	float A;

	// Period is 2 * PI/ (absolute value of B)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloaterVariables")
	float B;

	// Phase Shift is ( C / B ) // starts with different curve
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloaterVariables")
	float C;

	// Vertical Shift is D
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloaterVariables")
	float D;
private:
	float RunningTime;
	float BaseZLocation;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
