// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"

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
	// 如果不在服务器端，则不执行下面的代码
	if (!bIsServer) return;

	// 尝试将当前GA的AvatarActor转换为ICombatInterface接口
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		// 从ICombatInterface接口获取战斗相关的SocketLocation，这通常是武器或者角色身体上某个特定点的位置
		const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();
		// 使用目标位置和SocketLocation计算出方向
		FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
		//设置方向的Pitch为0，以实现方向和地面平行
		Rotation.Pitch = 0.0f;

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

		const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), SourceASC->MakeEffectContext());
		Projectile->DamageEffectSpecHandle = SpecHandle;

		// 完成投射物的生成过程，此时会应用所有已设定的属性，并将投射物加入到世界中
		Projectile->FinishSpawning(SpawnTransform);
	}
}
