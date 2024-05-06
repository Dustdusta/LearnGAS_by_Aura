// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacterBase.h"

AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// 使用CreateDefaultSubobject函数创建一个默认的USkeletalMeshComponent（骨骼网格组件）实例，并将其命名为"Weapon"
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	// 将新创建的武器组件设置为附加到当前Actor（假设是角色）的网格组件上，连接点为"WeaponHandSocket"插槽
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	// 设置"Weapon"组件的碰撞检测状态为禁用（NoCollision），这意味着它将不会参与任何物理碰撞计算
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	
}


