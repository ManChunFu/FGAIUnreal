// Fill out your copyright notice in the Description page of Project Settings.


#include "Floater.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AFloater::AFloater()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));

	InitialLocation = FVector(0.0f);
	PlacedLocation = FVector(0.0f);
	WorldOrigin = FVector(0.0f, 0.0f, 0.0f);
	InitialDirection = FVector(0.0f, 0.0f, 0.0f);
	InitialForce = FVector(2000000.0f, 0.0f, 0.0f);
	InitialTorque = FVector(2000000.0f, 0.0f, 0.0f);

	bInitializeFloaterLocations = false;
	bShouldFloat = false;

	RunningTime = 0.0f;

	A = 0.0f;
	B = 0.0f;
	C = 0.0f;
	D = 0.0f;
}

// Called when the game starts or when spawned
void AFloater::BeginPlay()
{
	Super::BeginPlay();

	//float InitialX = FMath::FRand(); // random numbers between 0 or 1
	//float InitialY = FMath::FRand();
	//float InitialZ = FMath::FRand();

	float InitialX = FMath::FRandRange(-500.0f, 500.0f); // random numbers between 0 or 1
	float InitialY = FMath::FRandRange(-500.0f, 500.0f);
	float InitialZ = FMath::FRandRange(0.0f, 500.f);

	InitialLocation.X = InitialX;
	InitialLocation.Y = InitialY;
	InitialLocation.Z = InitialZ;

	//InitialLocation *= 500.0f; // so the random numbers will be 0 or 500

	PlacedLocation = GetActorLocation();

	if (bInitializeFloaterLocations)
	{
		SetActorLocation(InitialLocation);
	}

	BaseZLocation = PlacedLocation.Z;

	// Torque is to let the object spin
	/*StaticMesh->AddForce(InitialForce);
	StaticMesh->AddTorque(InitialTorque);*/
	
}

// Called every frame
void AFloater::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldFloat)
	{
		FVector NewLocation = GetActorLocation();

		// sin is giving the value between -1 to 1
		NewLocation.Z = BaseZLocation + A * FMath::Sin(B * RunningTime - C) + D; // Period = 2 * PI / ABS(B)

		SetActorLocation(NewLocation);
		RunningTime += DeltaTime;

		/*FHitResult HitResult;
		AddActorLocalOffset(InitialDirection, true, &HitResult);
		FVector HitLocation = HitResult.Location;*/
	}

	//FRotator Rotation = FRotator(0.0f, 1.0f, 0.0f);
	//AddActorWorldRotation(Rotation);

}

