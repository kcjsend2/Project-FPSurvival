// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpawner.h"

#include "ZombieCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AZombieSpawner::AZombieSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RifleAmmoObjectPool = CreateDefaultSubobject<UActorObjectPoolComponent>(TEXT("RifleObjectPool"));
	PistolAmmoObjectPool = CreateDefaultSubobject<UActorObjectPoolComponent>(TEXT("PistolObjectPool"));
	HealthPickupObjectPool = CreateDefaultSubobject<UActorObjectPoolComponent>(TEXT("HealthObjectPool"));
	ZombieObjectPool = CreateDefaultSubobject<UCharacterObjectPoolComponent>(TEXT("ZombieObjectPool"));
	
	RandomStream.Initialize(FDateTime::Now().GetSecond());
}

// Called when the game starts or when spawned
void AZombieSpawner::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle SpawnTimerHandle;
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this,
		&AZombieSpawner::SpawnZombieAtCurrentLocation, 3, true);

	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("SpawnPoint"), SpawnPoint);
	
}

// Called every frame
void AZombieSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AZombieSpawner::SpawnZombieAtCurrentLocation()
{
	SpawnZombie(GetActorLocation(), GetActorRotation());
}

void AZombieSpawner::SpawnZombie(FVector Location, FRotator Rotation)
{
	AZombieCharacter* Zombie = Cast<AZombieCharacter>(ZombieObjectPool->SpawnPoolableCharacter());
	if(Zombie == nullptr)
		return;
	
	Zombie->SetDefault();
	Zombie->SetActorLocation(Location);
	Zombie->SetActorRotation(Rotation);

	float CurrentDropRange = 0;
	const int RandomValue = RandomStream.RandRange(1, 100);

	APoolableActor* DropItem = nullptr;
	
	//구간 별 확률, 어떻게 개선해야할지 감이 잘 안온다...
	if(RandomValue <= RifleAmmoDropChance && RandomValue > CurrentDropRange && DropItem == nullptr)
		DropItem = RifleAmmoObjectPool->SpawnPooledActor();
	else
		CurrentDropRange += RifleAmmoDropChance;
	
	if(RandomValue <= CurrentDropRange + PistolAmmoDropChance && RandomValue > CurrentDropRange && DropItem == nullptr)
		DropItem = PistolAmmoObjectPool->SpawnPooledActor();
	else
		CurrentDropRange += PistolAmmoDropChance;

	if(RandomValue <= CurrentDropRange + HealthPickupDropChance && RandomValue > CurrentDropRange && DropItem == nullptr)
		DropItem = HealthPickupObjectPool->SpawnPooledActor();

	if(DropItem != nullptr)
	{
		DropItem->SetDefault();
		DropItem->SetActive(true);
		Zombie->SetDropItem(Cast<AItemPickup>(DropItem));
	}

	Zombie->SetActive(true);
	Zombie->OnPoolableActorDespawn.AddDynamic(this, &AZombieSpawner::OnZombieDespawn);
	SpawnedZombieCounter++;
}

void AZombieSpawner::OnZombieDespawn(APoolableCharacter* PoolableCharacter)
{
	SpawnedZombieCounter--;
}

void AZombieSpawner::SpawnZombieAtSpawnPoint(int ZombieNum, int Interval)
{
	if(ZombieNum == 0)
		return;

	if(SpawnPoint.Num() == 0)
		return;
	
	const int RandomValue = RandomStream.RandRange(0, SpawnPoint.Num() - 1);
	
	SpawnZombie(SpawnPoint[RandomValue]->GetActorLocation(), SpawnPoint[RandomValue]->GetActorRotation());
	
	FTimerDelegate TimerDelegate;
	FTimerHandle SpawnTimerHandle;
	TimerDelegate.BindUFunction(this, FName("SpawnZombieAtSpawnPoint"), ZombieNum - 1, Interval);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, TimerDelegate, Interval, false);
}

