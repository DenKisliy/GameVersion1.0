// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Move/MSplineActor.h"

// Sets default values
AMSplineActor::AMSplineActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Spline Component"));
	SplineComponent->SetupAttachment(GetRootComponent());

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	MeshComponent->SetupAttachment(SplineComponent);
}

// Called when the game starts or when spawned
void AMSplineActor::BeginPlay()
{
	Super::BeginPlay();
}

void AMSplineActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bCanMove)
	{
		SetNewPosition(DeltaTime);
	}
}

void AMSplineActor::OnExplode()
{
	bCanMove = true;
	GetWorld()->GetTimerManager().ClearTimer(FuzeTimerHandle);
}

void AMSplineActor::SetNewPosition(float DeltaTime)
{
	DistanceAlongSpline = DistanceAlongSpline + DeltaTime * Speed;
	FVector currentSplineLocation = SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::Local);

	if (CheckPoints != -2)
	{
		CheckPoint(currentSplineLocation); 
	}
	else
	{
		CheckPoints = 0;
	}

	MeshComponent->SetRelativeLocation(currentSplineLocation);
	
	if (SplineComponent->IsClosedLoop())
	{
		if (DistanceAlongSpline > SplineComponent->GetSplineLength())
		{
			DistanceAlongSpline = 0;
		}
	}
}

void AMSplineActor::CheckPoint(FVector CurentPosition)
{
	FVector NextPoint = SplineComponent->GetLocationAtSplinePoint(CheckPoints + 1, ESplineCoordinateSpace::Local);
	if (FVector::Dist(CurentPosition, NextPoint) < 1)
	{
		bCanMove = false;
		CheckPoints = CheckPoints + 1 == SplineComponent->GetNumberOfSplinePoints() ? 0 : CheckPoints + 1;

		GetWorld()->GetTimerManager().SetTimer(FuzeTimerHandle, this, &AMSplineActor::OnExplode, 2.0f, false);
	}
}


