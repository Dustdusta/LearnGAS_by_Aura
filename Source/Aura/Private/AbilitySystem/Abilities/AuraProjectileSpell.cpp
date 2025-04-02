// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Aura/Public/AuraGameplayTags.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UKismetSystemLibrary::PrintString(this, FString("ActivateAbility (C++)"), true, true, FLinearColor::Yellow, 3);
}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	UE_LOG(LogTemp, Display, TEXT("GetOwningActorFromActorInfo(): %s"), *(GetOwningActorFromActorInfo()->GetName()));
	UE_LOG(LogTemp, Display, TEXT("GetAvatarActorFromActorInfo(): %s"), *(GetAvatarActorFromActorInfo()->GetName()));
	UE_LOG(LogTemp, Display, TEXT("bIsServer: %s"), bIsServer ? TEXT("true") : TEXT("false"));
	// 如果在服务器端，则执行之后的代码
	if (!bIsServer) return;


	// 从ICombatInterface接口获取战斗相关的SocketLocation，这通常是武器或者角色身体上某个特定点的位置
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), FAuraGameplayTags::Get().CombatSocket_Weapon);
	// 使用目标位置和SocketLocation计算出方向
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();


	// 创建一个FTransform对象，用来存储投射物生成时的位置和旋转信息
	FTransform SpawnTransform;
	// 设置投射物的生成位置为SocketLocation
	SpawnTransform.SetLocation(SocketLocation);
	// 设置投射物的生成方向为Rotation
	SpawnTransform.SetRotation(Rotation.Quaternion());

	// 使用SpawnActorDeferred推迟生成以达到设置初始化值的目的,这样可以在正式生成之前调整投射物的属性。
	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		// 指定要生成的投射物类
		ProjectileClass,
		// 传递给投射物的初始变换信息
		SpawnTransform,
		// 获取拥有者Actor，通常是指发动技能的角色
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn); // 指定碰撞处理方式，这里选择总是生成投射物，即使它与场景中的其他物体发生碰撞

	// 获取源角色的能力系统组件
	const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());

	// 创建一个EffectContextHandle，并将其所有参数填满
	FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
	EffectContextHandle.SetAbility(this);
	EffectContextHandle.AddSourceObject(Projectile);
	TArray<TWeakObjectPtr<AActor>> Actors;
	Actors.Add(Projectile);
	EffectContextHandle.AddActors(Actors);
	FHitResult HitResult;
	HitResult.Location = ProjectileTargetLocation;
	EffectContextHandle.AddHitResult(HitResult);


	// 创建一个游戏效果规格句柄，用于应用伤害效果
	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);
	// 获取游戏标签实例
	FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	// 根据当前技能等级计算缩放后的伤害值
	for (auto& Pair : DamageTypes)
	{
		const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
		// 在屏幕上显示一条调试消息
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("FireBolt Damage: %f"), ScaledDamage));

		// 为游戏效果规格句柄分配一个由调用者设置的Damage标签，并设置伤害值为样条线Damage的对应等级的值
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
	}
	// 将创建的游戏效果规格句柄赋值给投射物对象的DamageEffectSpecHandle属性
	Projectile->DamageEffectSpecHandle = SpecHandle;

	// 完成投射物的生成过程，此时会应用所有已设定的属性，并将投射物加入到世界中
	Projectile->FinishSpawning(SpawnTransform);
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
