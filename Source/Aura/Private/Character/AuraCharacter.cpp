// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerState.h"

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
}

void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Init ability actor info for the Client
	InitAbilityActorInfo();
}


void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();
}
