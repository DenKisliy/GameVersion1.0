// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Move/MSplineCharacter.h"

// Sets default values
AMSplineCharacter::AMSplineCharacter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Spline Component"));
	SplineComponent->SetupAttachment(GetRootComponent());

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh Component"));
	MeshComponent->SetupAttachment(SplineComponent);

}

USplineComponent* AMSplineCharacter::GetSplineComponent()
{
	return SplineComponent;
}

// Called when the game starts or when spawned
void AMSplineCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMSplineCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	/*if (bCanMove)
	{
		SetNewPosition(DeltaTime);
	}*/
}

void AMSplineCharacter::OnExplode()
{
	bCanMove = true;
	GetWorld()->GetTimerManager().ClearTimer(FuzeTimerHandle);
}

void AMSplineCharacter::SetNewPosition(float DeltaTime)
{
	DistanceAlongSpline = DistanceAlongSpline + DeltaTime * Speed;
	FVector currentSplineLocation = SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::Local);
	FRotator currentSplineRotator = SplineComponent->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::Local);
	if (CheckPoints != -2)
	{
		CheckPoint(currentSplineLocation);
	}
	else
	{
		CheckPoints = 0;
	}
	currentSplineRotator.Yaw = currentSplineRotator.Yaw - 90;
	MeshComponent->SetRelativeRotation(currentSplineRotator);
	MeshComponent->SetRelativeLocation(currentSplineLocation);

	if (SplineComponent->IsClosedLoop())
	{
		if (DistanceAlongSpline > SplineComponent->GetSplineLength())
		{
			DistanceAlongSpline = 0;
		}
	}
}

void AMSplineCharacter::CheckPoint(FVector CurentPosition)
{
	FVector NextPoint = SplineComponent->GetLocationAtSplinePoint(CheckPoints + 1, ESplineCoordinateSpace::Local);
	if (FVector::Dist(CurentPosition, NextPoint) < 1)
	{
		bCanMove = false;
		CheckPoints = CheckPoints + 1 == SplineComponent->GetNumberOfSplinePoints() ? 0 : CheckPoints + 1;

		GetWorld()->GetTimerManager().SetTimer(FuzeTimerHandle, this, &AMSplineCharacter::OnExplode, 2.0f, false);
	}
}

