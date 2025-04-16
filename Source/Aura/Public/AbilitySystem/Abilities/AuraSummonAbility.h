// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraSummonAbility.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraSummonAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	// 获取召唤位置
	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetSpawnLocations();

	// 随机获取一个召唤的类
	UFUNCTION(BlueprintPure, Category="Summoning")
	TSubclassOf<APawn> GetRandomMinionClass();
	
	// 召唤的数量
	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	int32 NumMinions = 5;

	// 召唤的类
	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	TArray<TSubclassOf<APawn>> MinionClasses;

	// 召唤的最小的位置
	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	float MinSpawnDistance = 50.0f;
	// 召唤的最大的位置
	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	float MaxSpawnDistance = 250.0f;

	// 召唤的扇面角度
	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	float SpawnSpread = 90.0f;

};
