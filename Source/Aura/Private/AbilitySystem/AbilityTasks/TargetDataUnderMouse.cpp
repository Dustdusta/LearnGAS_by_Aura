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
	if (bIsLocallyControlled)
	{
		//  若是本地控制的，则表面在客户端，需发送数据至服务器
		SendMouseCursorData();
	}
	else
	{
		// 若不是本地控制的，则表明在服务器，需监听数据

		// 创建变量并存储规格句柄和激活预测键，以保证前后一致
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		// 给对应的广播绑定函数，开始监听数据
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);
		// 若是目标数据已经发送且触发了委托，则bCalledDelegate将为true。若是目标已经发送但委托未被触发，则bCalledDelegate将为false
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);
		if(!bCalledDelegate)
		{
			// 若bCalledDelegate将为false，则继续等待客户端的数据
			SetWaitingOnRemotePlayerData();
		}
	}
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	// 创建范围预测窗口，并生成一个新的预测键和对应的密钥
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());


	// 通过GT自带的Ability参数，寻找当前玩家控制器，并将点击结果存储到变量CursorHit中
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	// 创建Data句柄
	FGameplayAbilityTargetDataHandle DataHandle;
	// 使用FGameplayAbilityTargetData_SingleTargetHit，因为Target data with a single hit result, data is packed into the hit result
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = CursorHit;
	DataHandle.Add(Data);

	// 将点击数据发送到服务器
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(), // 当能力启动时，产生的预测键
		DataHandle,
		FGameplayTag(), // 传递一个空的GameplayTag
		AbilitySystemComponent->ScopedPredictionKey
	);

	// 检查ShouldBroadcastAbilityTaskDelegates()，如果该GT尚未激活，这将阻止我们在不应该进行广播的情况下进行广播
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		// 在本地广播点击数据句柄
		ValidData.Broadcast(DataHandle);
	}
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
	// ConsumeClientReplicatedTargetData会根据句柄和预测键检查服务器接收的数据Map，以确保数据被接收了
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(),GetActivationPredictionKey());
	// 检查ShouldBroadcastAbilityTaskDelegates()，如果该GT尚未激活，这将阻止我们在不应该进行广播的情况下进行广播
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		// 在本地广播点击数据句柄
		ValidData.Broadcast(DataHandle);
	}
}
