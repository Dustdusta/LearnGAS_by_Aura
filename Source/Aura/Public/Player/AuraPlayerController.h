// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "AuraPlayerController.generated.h"

class UAuraInputConfig;
// 声明一个输入映射上下文类（UInputMappingContext）的引用
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnemyInterface;
class UAuraAbilitySystemComponent;
class USplineComponent;
/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAuraPlayerController();
	virtual void PlayerTick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	// 具有EditAnywhere编辑器属性且分类为"Input"的属性，类型为UInputMappingContext指针，此属性允许在Unreal Editor中可视化编辑并存储一个指向UInputMappingContext对象的引用
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	void Move(const FInputActionValue& InputActionValue);

	void CursorTrace();

	TScriptInterface<IEnemyInterface> LastActor;
	TScriptInterface<IEnemyInterface> ThisActor;

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);


	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UAuraInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	UAuraAbilitySystemComponent* GetASC();


	// 存储目标位置
	FVector CachedDestination = FVector::ZeroVector;
	// 存储按键按下的时长
	float FollowTime = 0.f;
	// 存储判断短按的时长
	float ShortPressThreshold = 0.5f;
	// 存储是否自动移动
	bool bAutoRunning = false;
	// 存储鼠标是否正在瞄准敌人
	bool bTargeting = false;

	// 存储自动运行接受半径
	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;

	// 存储构造的样条线
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;
};
