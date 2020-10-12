// Fill out your copyright notice in the Description page of Project Settings.


#include "AIEnemyAnimInstance.h"
#include "AIEnemy.h"

void UAIEnemyAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			AIEnemy = Cast<AAIEnemy>(Pawn);
		}
	}
}

void UAIEnemyAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
	}
	if (Pawn)
	{
		FVector Speed = Pawn->GetVelocity();
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.0f);
		MovementSpeed = LateralSpeed.Size();

		if (AIEnemy == nullptr)
		{
			AIEnemy = Cast<AAIEnemy>(Pawn);
		}
	}
}
