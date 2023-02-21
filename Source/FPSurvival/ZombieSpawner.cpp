// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpawner.h"

#include "ZombieCharacter.h"

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

	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this,
		&AZombieSpawner::SpawnZombieAtCurrentLocation, 10, true);
}

// Called every frame
void AZombieSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AZombieSpawner::SpawnZombieAtCurrentLocation()
{
	AZombieCharacter* Zombie = Cast<AZombieCharacter>(ZombieObjectPool->SpawnPoolableCharacter());
	Zombie->SetDefault();
	Zombie->SetActorLocation(GetActorLocation());
	Zombie->SetActorRotation(GetActorRotation());

	float CurrentDropRange = 0;
	const int RandomValue = RandomStream.RandRange(1, 100);

	APoolableActor* DropItem = nullptr;
	
	//구간 별 확률, 어떻게 개선해야할지 감이 잘 안온다...
	if(RandomValue <= RifleAmmoDropChance)
		DropItem = RifleAmmoObjectPool->SpawnPooledActor();
	else
		CurrentDropRange += RifleAmmoDropChance;
	
	if(RandomValue <= CurrentDropRange + PistolAmmoDropChance)
		DropItem = PistolAmmoObjectPool->SpawnPooledActor();
	else
		CurrentDropRange += PistolAmmoDropChance;

	if(RandomValue <= CurrentDropRange + HealthPickupDropChance)
		DropItem = HealthPickupObjectPool->SpawnPooledActor();

	if(DropItem != nullptr)
	{
		DropItem->SetDefault();
		DropItem->SetActive(true);
		Zombie->SetDropItem(Cast<AItemPickup>(DropItem));
	}

	Zombie->SetActive(true);
}



void AZombieSpawner::SpawnZombie(FVector Location, FRotator Rotation)
{
	AZombieCharacter* Zombie = Cast<AZombieCharacter>(ZombieObjectPool->SpawnPoolableCharacter());
	Zombie->SetActorLocation(Location);
	Zombie->SetActorRotation(Rotation);

	float CurrentDropRange = 0;
	const int RandomValue = RandomStream.RandRange(1, 100);

	APoolableActor* DropItem = nullptr;
	
	//구간 별 확률, 어떻게 개선해야할지 감이 잘 안온다...
	if(RandomValue <= RifleAmmoDropChance)
		DropItem = RifleAmmoObjectPool->SpawnPooledActor();
	else
		CurrentDropRange += RifleAmmoDropChance;
	
	if(RandomValue <= CurrentDropRange + PistolAmmoDropChance)
		DropItem = PistolAmmoObjectPool->SpawnPooledActor();
	else
		CurrentDropRange += PistolAmmoDropChance;

	if(RandomValue <= CurrentDropRange + HealthPickupDropChance)
		DropItem = HealthPickupObjectPool->SpawnPooledActor();

	if(DropItem != nullptr)
		Zombie->SetDropItem(Cast<AItemPickup>(DropItem));
}

