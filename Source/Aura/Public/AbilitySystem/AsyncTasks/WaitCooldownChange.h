// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "WaitCooldownChange.generated.h"


struct FGameplayEffectSpec;
class UAbilitySystemComponent;

// 声明冷却时间变化的动态多播委托，参数为剩余时间
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCooldownChangeSignature, float, TimeRemaining);

/**
 * 异步节点：等待特定冷却标签（CooldownTag）的状态变化
 * 用于在技能冷却开始/结束时触发蓝图事件
 */
UCLASS(BlueprintType, meta = (ExposedAsyncProxy="AsyncTask"))
class AURA_API UWaitCooldownChange : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// 当冷却开始时广播的委托（剩余时间为冷却总时长）
	UPROPERTY(BlueprintAssignable)
	FCooldownChangeSignature CooldownStart;

	// 当冷却结束时广播的委托（剩余时间为0）
	UPROPERTY(BlueprintAssignable)
	FCooldownChangeSignature CooldownEnd;

	// 创建异步任务实例的静态方法
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UWaitCooldownChange* WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& InCooldownTag);

	// 手动终止异步任务
	UFUNCTION(BlueprintCallable)
	void EndTask();

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC; // 缓存的技能系统组件

	FGameplayTag CooldownTag; // 需要监听的冷却标签

	// 冷却标签数量变化时的回调（NewCount=0表示冷却结束）
	void CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount);
	// 当新的GameplayEffect被应用时的回调（检测冷却开始）
	void OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied,  FActiveGameplayEffectHandle ActiveEffectHandle);
};
