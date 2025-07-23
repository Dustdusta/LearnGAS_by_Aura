// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AsyncTasks/WaitCooldownChange.h"

#include "AbilitySystemComponent.h"

UWaitCooldownChange* UWaitCooldownChange::WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& InCooldownTag)
{
	// 创建任务实例
	UWaitCooldownChange* WaitCooldownChange = NewObject<UWaitCooldownChange>();
	WaitCooldownChange->ASC = AbilitySystemComponent;
	WaitCooldownChange->CooldownTag = InCooldownTag;

	// 验证参数有效性
	if (!IsValid(AbilitySystemComponent) || !InCooldownTag.IsValid())
	{
		WaitCooldownChange->EndTask();
		return nullptr;
	}

	// 注册冷却标签事件（标签添加/移除）
	// To know when a cooldown has ended (Cooldown Tag has been removed)
	AbilitySystemComponent->RegisterGameplayTagEvent(InCooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(
		WaitCooldownChange,
		&UWaitCooldownChange::CooldownTagChanged);

	// 注册GE添加事件（检测冷却开始）
	// To know when a cooldown effect has been applied
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(WaitCooldownChange, &UWaitCooldownChange::OnActiveEffectAdded);

	return WaitCooldownChange;
}

void UWaitCooldownChange::EndTask()
{
	// 取消标签事件监听
	if (IsValid(ASC))
		ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);

	// 安全销毁标记对象
	SetReadyToDestroy();
	MarkAsGarbage();
}

void UWaitCooldownChange::CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount)
{
	// 当标签计数归零时（冷却结束）
	if (NewCount == 0)
	{
		CooldownEnd.Broadcast(0.f);
	}
}

void UWaitCooldownChange::OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	// 获取效果关联的标签
	FGameplayTagContainer AssetTag;
	SpecApplied.GetAllAssetTags(AssetTag);

	FGameplayTagContainer GrantedTag;
	SpecApplied.GetAllGrantedTags(GrantedTag);

	// 检查是否包含目标冷却标签
	if (AssetTag.HasTagExact(CooldownTag) || GrantedTag.HasTagExact(CooldownTag))
	{
		// 创建标签查询
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());
		// 获取所有匹配效果的剩余时间
		TArray<float> TimesRemaining = ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		if (TimesRemaining.Num() > 0)
		{
			// 取最长剩余时间（应对叠加情况）
			float TimeRemaining = TimesRemaining[0];
			for (int32 i = 1; i < TimesRemaining.Num(); i++)
			{
				if (TimesRemaining[i] > TimeRemaining)
				{
					TimeRemaining  = TimesRemaining[i];
				}
			}
			// 广播冷却开始事件
			CooldownStart.Broadcast(TimeRemaining);
		}
	}
}

/*--------------------
██       ██
█ ██       ██
█   ██       ██
█     ██       ██
█       ██       ██
█         ██████████
█       ████      ██
█     ██  ██      ██
█   ██    ██      ██
█ ██      ██      ██
██        ██      ██
  ██      ██      ██
	██    ██      ██
	  ██  ██      ██
		████      ██
--------------------*/
