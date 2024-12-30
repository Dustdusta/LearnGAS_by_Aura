// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Aura/Aura.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	// 设置是否允许客户端复制
	bReplicates = true;

	// 生成并设置球体碰撞体组件的相关信息
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	// 设置碰撞启用模式为QueryOnly，这意味着只用于物理查询（如视线测试），而不是物理模拟
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 生成并设置抛射物组件的相关信息
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 550.f;
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();
	// 设置投射物的生命期限，在此期限后投射物将自动销毁
	SetLifeSpan(LifeSpan);
	// 给投射物的碰撞组件（Sphere）注册一个开始重叠的事件回调，当与其他物体发生重叠时会触发
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereOverlap);
	// 创建并绑定一个循环播放的声音组件到投射物的根组件上
	LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
}

void AAuraProjectile::Destroyed()
{
	// 如果投射物没有命中目标，并且不是服务器端（即没有权威）
	if (!bHit && !HasAuthority())
	{
		// 在投射物的位置播放撞击音效
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
		// 在投射物的位置生成粒子效果
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
		// 停止循环音效的播放
		if (LoopingSoundComponent)
			LoopingSoundComponent->Stop();
	}
	// 调用父类的Destroyed方法，完成销毁过程
	Super::Destroyed();
}

void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (DamageEffectSpecHandle.Data.IsValid() && DamageEffectSpecHandle.Data.Get()->GetContext().GetEffectCauser() == OtherActor)
		return;

	if (!bHit)
	{
		// 播放撞击音效
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
		// 生成粒子效果
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
		// 停止循环音效的播放
		if (LoopingSoundComponent)
			LoopingSoundComponent->Stop();
	}


	// 如果是服务器端（有权威）
	if (HasAuthority())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
		}

		// 销毁投射物
		Destroy();
	}
	else
	{
		// 标记投射物为已命中
		bHit = true;
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
