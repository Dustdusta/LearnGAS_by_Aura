// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"

AAuraCharacter::AAuraCharacter()
{
	// 设置角色移动时的朝向，使其自动根据移动方向调整朝向
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// 设置角色的旋转速率，以便快速响应移动方向的变化
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	// 启用对角色移动的平面约束
	GetCharacterMovement()->bConstrainToPlane = true;
	// 在开始时将角色位置约束到平面上
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// 禁用控制器的俯仰、偏航和翻滚控制，以自定义角色的旋转行为
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Init ability actor info for the Server
	InitAbilityActorInfo();
	AddCharacterAbilities();
}

void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Init ability actor info for the Client
	InitAbilityActorInfo();
}

int32 AAuraCharacter::GetPlayerLevel()
{
	const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetPlayerLevel();
}


void AAuraCharacter::InitAbilityActorInfo()
{
	// 获取玩家状态对象，并将其转换为AAuraPlayerState类型
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	// 检查AuraPlayerState是否成功获取，如果失败则会触发断言
	check(AuraPlayerState);
	// 调用该函数来设置OwnerActor和AvatarActor
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
	// 将AuraPlayerState的能力系统组件转换为UAuraAbilitySystemComponent类型，并调用其AbilityActorInfoSet方法
	Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	// 将AuraPlayerState的能力系统组件赋值给本地变量AbilitySystemComponent
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	// 将AuraPlayerState的属性集赋值给本地变量AttributeSet
	AttributeSet = AuraPlayerState->GetAttributeSet();

	// Init Overlay in HUD
	// 获取Controller并转类型为AAuraPlayerController，并判断是否为空
	// 联机时，在客户端，自己的Controller不为空，其余玩家的Controller为空
	if(AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		// HUD仅对本地控制器的玩家有效
		if(AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}

	// 在设置ASC后，调用初始化GE
	InitializeDefaultAttributes();
}
