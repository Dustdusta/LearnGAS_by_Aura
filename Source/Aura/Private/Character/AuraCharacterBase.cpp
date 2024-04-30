// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacterBase.h"

AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// ʹ��CreateDefaultSubobject��������һ��Ĭ�ϵ�USkeletalMeshComponent���������������ʵ��������������Ϊ"Weapon"
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	// ���´����������������Ϊ���ӵ���ǰActor�������ǽ�ɫ������������ϣ����ӵ�Ϊ"WeaponHandSocket"���
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	// ���������������ײ״̬Ϊ���ã���������������ײ���
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	
}


