// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"
#include "AbilitySystemComponent.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	// 因为此函数在客户端本地触发时，也会在服务器上触发。所以在客户端本地触发时需发送数据到服务器
	// 判断是否为本地控制
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	if(bIsLocallyControlled)
	{
		SendMouseCursorData();
	}
	else
	{
		// AURATODO: We are on the server, so listen for target data. 
	}
	
	
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	// 创建范围预测窗口，并生成一个新的预测键和对应的密钥
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());

	
	
	// 通过GT自带的Ability参数，寻找当前玩家控制器，并将点击结果存储到变量CursorHit中
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Visibility,false,CursorHit);

	// 创建Data句柄
	FGameplayAbilityTargetDataHandle DataHandle;
	// 使用FGameplayAbilityTargetData_SingleTargetHit，因为Target data with a single hit result, data is packed into the hit result
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = CursorHit;
	DataHandle.Add(Data);

	// 将点击数据发送到服务器
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),// 当能力启动时，产生的预测键
		DataHandle,
		FGameplayTag(),// 传递一个空的GameplayTag
		AbilitySystemComponent->ScopedPredictionKey
		);

	// 检查ShouldBroadcastAbilityTaskDelegates()，如果该GT尚未激活，这将阻止我们在不应该进行广播的情况下进行广播
	if(ShouldBroadcastAbilityTaskDelegates())
	{
		// 在本地广播点击数据句柄
		ValidData.Broadcast(DataHandle);
	}
}
