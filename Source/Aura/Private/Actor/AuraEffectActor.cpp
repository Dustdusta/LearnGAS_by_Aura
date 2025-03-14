#include "Actor/AuraEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}

void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	// 判断碰撞的Actor是否为Enemy
	if (TargetActor->ActorHasTag("Enemy") && !bApplyEffectsToEnemies)
	{
		return;
	}
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr) return;

	check(GameplayEffectClass);
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	// 检查GE的时间策略是否为无限
	const bool bIsInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	if (bIsInfinite && InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		// ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
		ActiveEffectHandles.Add(TargetASC, ActiveEffectHandle);
	}

	// 判断是否为瞬时的GE
	if (bDestroyOnEffectApplication && !bIsInfinite)
	{
		Destroy();
	}
}

// 各种情况下的开始覆盖函数
void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	// 判断碰撞的Actor是否为Enemy
	if (TargetActor->ActorHasTag("Enemy") && !bApplyEffectsToEnemies)
	{
		return;
	}
	
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}
	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
}

// 各种情况下的结束覆盖函数
void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	// 判断碰撞的Actor是否为Enemy
	if (TargetActor->ActorHasTag("Enemy") && !bApplyEffectsToEnemies)
	{
		return;
	}
	
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}
	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
	if (InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!IsValid(TargetASC)) return;

		// TArray<FActiveGameplayEffectHandle> HandlesToRemove;
		// for (TTuple<FActiveGameplayEffectHandle, UAbilitySystemComponent*> HandlePair : ActiveEffectHandles)
		// {
		// 	if (TargetASC == HandlePair.Value)
		// 	{
		// 		TargetASC->RemoveActiveGameplayEffect(HandlePair.Key, 1);
		// 		HandlesToRemove.Add(HandlePair.Key);
		// 	}
		// }
		// for (auto& Handle : HandlesToRemove)
		// {
		// 	ActiveEffectHandles.FindAndRemoveChecked(Handle);
		// }
		const FActiveGameplayEffectHandle* ActiveGameplayEffectHandle = ActiveEffectHandles.Find(TargetASC);
		if(ActiveGameplayEffectHandle == nullptr)
			return;
		TargetASC->RemoveActiveGameplayEffect(*ActiveGameplayEffectHandle,1);
		ActiveEffectHandles.FindAndRemoveChecked(TargetASC);
	}
}
