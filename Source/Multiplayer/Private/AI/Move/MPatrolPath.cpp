// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Move/MPatrolPath.h"

// Sets default values
AMPatrolPath::AMPatrolPath()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Spline Component"));
	SplineComponent->SetupAttachment(GetRootComponent());

	SetReplicates(true);
}

void AMPatrolPath::FindNextPointAndIndex(FVector& NextPoint, int& Index)
{
	if (SplineComponent->IsClosedLoop())
	{
		Index = Index + 1 > SplineComponent->GetNumberOfSplinePoints() ? 0 : Index + 1;
	}
	else
	{
		if (bReverseMove)
		{
			if (Index == 0)
			{
				Index = 0;
				bReverseMove = false;
			}
			else
			{
				Index = Index - 1;
			}
		}
		else
		{
			if (Index == SplineComponent->GetNumberOfSplinePoints())
			{
				Index = Index - 1;
				bReverseMove = true;
			}
			else
			{
				Index = Index + 1;
			}
		}

	}

	NextPoint = SplineComponent->GetLocationAtSplinePoint(Index, ESplineCoordinateSpace::World);
}

bool AMPatrolPath::IsSetOwner()
{
	return (OwnerPawn != nullptr);
}

FVector AMPatrolPath::GetStartPoint()
{
	return SplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
}

FRotator AMPatrolPath::GetRotatorForStartPoint()
{
	return UKismetMathLibrary::FindLookAtRotation(SplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World), SplineComponent->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World));
}

void AMPatrolPath::SetOwnerPawn(APawn* NewOwnerPawn)
{
	OwnerPawn = NewOwnerPawn;
}

