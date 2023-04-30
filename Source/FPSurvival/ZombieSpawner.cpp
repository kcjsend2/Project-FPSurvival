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

	// FTimerHandle SpawnTimerHandle;
	// GetWorldTimerManager().SetTimer(SpawnTimerHandle, this,
	// 	&AZombieSpawner::SpawnZombieAtCurrentLocation, 3, true);

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
	
	if(RandomValue <= RifleAmmoDropChance && RandomValue > CurrentDropRange && DropItem == nullptr)
		DropItem = RifleAmmoObjectPool->SpawnPoolableActor();
	else
		CurrentDropRange += RifleAmmoDropChance;
	
	if(RandomValue <= CurrentDropRange + PistolAmmoDropChance && RandomValue > CurrentDropRange && DropItem == nullptr)
		DropItem = PistolAmmoObjectPool->SpawnPoolableActor();
	else
		CurrentDropRange += PistolAmmoDropChance;

	if(RandomValue <= CurrentDropRange + HealthPickupDropChance && RandomValue > CurrentDropRange && DropItem == nullptr)
		DropItem = HealthPickupObjectPool->SpawnPoolableActor();

	if(DropItem != nullptr)
	{
		DropItem->SetDefault();
		DropItem->SetActive(true);
		Zombie->SetDropItem(Cast<AItemPickup>(DropItem));
	}

	Zombie->SetActive(true);
	Zombie->OnZombieDead.BindUFunction(this, TEXT("OnZombieDead"));
	OnDeactivateAll.AddUFunction(Zombie, "OnDespawnCall");
	AliveZombieCounter++;
}

void AZombieSpawner::OnZombieDead()
{
	AliveZombieCounter--;
}

void AZombieSpawner::DeactivateAllZombies()
{
	AliveZombieCounter = 0;
	bIsSpawning = false;
	if(SpawnTimerHandle.IsValid())
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	OnDeactivateAll.Broadcast();
}

void AZombieSpawner::SpawnZombieAtSpawnPoint(int ZombieNum, float Interval)
{
	if(ZombieNum == 0)
	{
		bIsSpawning = false;
		return;
	}
	if(SpawnPoint.Num() == 0)
		return;

	bIsSpawning = true;
	
	const int RandomValue = RandomStream.RandRange(0, SpawnPoint.Num() - 1);
	
	SpawnZombie(SpawnPoint[RandomValue]->GetActorLocation(), SpawnPoint[RandomValue]->GetActorRotation());
	
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("SpawnZombieAtSpawnPoint"), ZombieNum - 1, Interval);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, TimerDelegate, Interval, false);
}

