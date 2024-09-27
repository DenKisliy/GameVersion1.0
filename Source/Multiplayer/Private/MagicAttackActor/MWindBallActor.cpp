// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicAttackActor/MWindBallActor.h"

// Sets default values
AMWindBallActor::AMWindBallActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

	CollisionComponent->CanCharacterStepUpOn = ECB_No;
	CollisionComponent->SetShouldUpdatePhysicsVolume(true);
	CollisionComponent->SetCanEverAffectNavigation(false);
	CollisionComponent->bDynamicObstacle = true;
	RootComponent = CollisionComponent;

	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
	MovementComponent->SetUpdatedComponent(CollisionComponent);
	MovementComponent->InitialSpeed = 500.0f;
	MovementComponent->MaxSpeed = 600.0f;
	MovementComponent->bRotationFollowsVelocity = true;
	MovementComponent->bShouldBounce = true;
	MovementComponent->Bounciness = 0.3f;
	MovementComponent->ProjectileGravityScale = 0.0f;
	MovementComponent->bAutoActivate = false;

	WindBallParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("WindBallParticle"));
	WindBallParticleComponent->AttachToComponent(CollisionComponent, FAttachmentTransformRules::KeepRelativeTransform);

	CollisionComponent->OnComponentHit.AddDynamic(this, &AMWindBallActor::OnHit);

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AMWindBallActor::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent->Activate();

	GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [this]()
		{
			GetWorld()->GetTimerManager().ClearTimer(const_cast<FTimerHandle&>(DestroyTimer));
			Destroy();
		}, TimeToDestroy, false);
}

void AMWindBallActor::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	GetWorld()->GetTimerManager().ClearTimer(DestroyTimer);

	if (AMBaseCharacter* hitActor = Cast<AMBaseCharacter>(OtherActor))
	{
		hitActor->GetAbilitySystemComponent()->ApplyGameplayEffectToSelf(DamageGameplayEffect, 1.0f, hitActor->GetAbilitySystemComponent()->MakeEffectContext());
	}

	Destroy();
}

void AMWindBallActor::SetDamageEffect(UGameplayEffect* GameplayEffect)
{
	DamageGameplayEffect = GameplayEffect;
}

