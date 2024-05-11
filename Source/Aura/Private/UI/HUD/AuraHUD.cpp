// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/AuraHUD.h"

#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"

UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);

		return OverlayWidgetController;
	}
	return OverlayWidgetController;
}

void AAuraHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	// 检查OverlayWidgetClass和OverlayWidgetControllerClass是否已设置，如果没有则输出错误信息到日志。
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialized, please fill out BP_AuraHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class uninitialized, please fill out BP_AuraHUD"));

	// 使用指定的OverlayWidget类在当前世界的上下文中创建一个用户界面Widget。
	UUserWidget* Widget = CreateWidget(GetWorld(), OverlayWidgetClass);
	// 将新创建的Widget安全地转换为我们的自定义UAuraUserWidget类型以便使用其特定功能。
	OverlayWidget = Cast<UAuraUserWidget>(Widget);

	// 准备用于初始化WidgetControllerParams。
	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	// 设置WidgetController，以便Widget可以通过控制器访问游戏逻辑和数据。
	OverlayWidget->SetWidgetController(WidgetController);
	// 通知WidgetController广播初始值，这通常用于设置UI上显示的起始数值或状态。
	WidgetController->BroadcastInitialValues();
	OverlayWidget->AddToViewport();
}
