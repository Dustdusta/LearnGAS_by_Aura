// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"


AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
}

void AAuraPlayerController::CursorTrace()
{
	// 获取鼠标光标下的命中结果，ECC_Visibility表示我们关心的碰撞频道（在这种情况下可能是可见几何体）
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	// 检查命中测试是否成功并返回一个阻塞命中（即是否有物体被鼠标光标下方的射线击中）
	if (!CursorHit.bBlockingHit)
		return;
	// 将上一次选中的Actor保存到LastActor变量中
	LastActor = ThisActor;
	// 对当前鼠标光标下的命中结果所对应的Actor进行类型转换，尝试将其转换为IEnemyInterface接口类型
	 ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());
	// ThisActor.SetInterface(Cast<IEnemyInterface>(CursorHit.GetActor()));
	/**
	 * Line trace from cursor. There are several scenarios;
	 * A. LastActor is null && ThisActor is null
	 *		- Do nothing
	 * B. LastActor is null && ThisActor is valid
	 *		- Highlight ThisActor
	 * C. LastActor is valid && ThisActor is null
	 * 		- UnHighlight LastActor
	 * D. Both actors are valid, but LastActor != ThisActor
	 * 		- UnHighlight LastActor, Highlight ThisActor
	 * E. Both actors are valid, and LastActor == ThisActor
	 * 		- Do nothing
	 */
	if (LastActor == nullptr)
	{
		if (ThisActor != nullptr)
		{
			// Case B
			ThisActor->HighlightActor();
		}
	}
	else // LastActor is valid
	{
		if (ThisActor == nullptr)
		{
			// Case C
			LastActor->UnHighlightActor();
		}
		else // Both actors are valid
		{
			if (LastActor != ThisActor)
			{
				// Case D
				LastActor->UnHighlightActor();
				ThisActor->HighlightActor();
			}
		}
	}
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 检查成员变量AuraContext是否存在，如果为空则抛出断言错误
	check(AuraContext);

	// 获取当前本地玩家关联的UEnhancedInputLocalPlayerSubsystem子系统实例
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	// 检查获取的子系统实例是否存在，如果为空则抛出断言错误
	if (Subsystem)
	{
		// 将成员变量AuraContext添加到增强输入子系统中
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	// 设置显示鼠标光标并定义默认鼠标光标形状
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	// 设置输入模式，允许游戏和UI交互，不锁定鼠标，捕获时也不隐藏鼠标
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

/**
 * 设置输入组件，绑定移动动作。
 */
void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 将输入组件转换为增强型输入组件
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	// 绑定移动动作到Move方法
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

/**
 * 处理移动动作。
 * 
 * @param InputActionValue 输入动作的值，包含动作的强度和方向。
 */
void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	// 获取输入轴向量
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	// 获取当前的控制旋转
	const FRotator Rotation = GetControlRotation();
	// 仅提取yaw成分，构建一个新的旋转
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	// 根据yaw旋转计算前向和侧向方向
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// 如果当前玩家控制器成功获取到了所控制的pawn（游戏角色）对象，并将其转换为APawn类型的指针赋值给ControlledPawn
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		// 向Pawn添加移动输入，根据输入轴向量控制移动方向和速度
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}
