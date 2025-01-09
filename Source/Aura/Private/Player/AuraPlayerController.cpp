// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "MovieSceneTracksComponentTypes.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Character.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "UI/Widget/DamageTextComponent.h"


AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
	AutoRun();
}

void AAuraPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	// 检查TargetCharacter是否有效（即非空且未被销毁），以及DamageTextComponentClass是否已设置，以及不在服务器上
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		// 创建一个新的UDamageTextComponent实例，使用之前指定的DamageTextComponentClass作为模板。
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		// 注册新创建的组件，使其成为游戏世界的一部分。
		DamageText->RegisterComponent();
		// 将新创建的伤害文本组件附加到目标角色的根组件上，并保持相对变换不变。这意味着伤害数字将跟随目标角色移动，但相对于其位置保持固定。
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		// 从组件上分离，但保持其在世界中的变换不变。这一步确保了即使目标角色移动，伤害数字也会停留在原位。
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		// 设置伤害文本的内容为传入的DamageAmount值。这个方法可能负责更新组件内部UI元素来显示具体的伤害数值。
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControllerPawn = GetPawn())
	{
		// 获取一个在Spline上最接近ControllerPawn的位置，以及该位置在Spline上的方向
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(
			ControllerPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(
			LocationOnSpline, ESplineCoordinateSpace::World);
		ControllerPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}

void AAuraPlayerController::CursorTrace()
{
	// 获取鼠标光标下的命中结果，ECC_Visibility表示我们关心的碰撞频道（在这种情况下可能是可见几何体）
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	// 检查命中测试是否成功并返回一个阻塞命中（即是否有物体被鼠标光标下方的射线击中）
	if (!CursorHit.bBlockingHit)
		return;
	// 将上一次选中的Actor保存到LastActor变量中
	LastActor = ThisActor;
	// 对当前鼠标光标下的命中结果所对应的Actor进行类型转换，尝试将其转换为IEnemyInterface接口类型
	ThisActor = CursorHit.GetActor();

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
	if (LastActor != ThisActor)
	{
		if (LastActor)
		{
			LastActor->UnHighlightActor();
		}
		if (ThisActor)
		{
			ThisActor->HighlightActor();
		}
	}
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		// 通过ThisActor是否为空判断是否正在瞄准敌人
		bTargeting = ThisActor ? true : false;
		bAutoRunning = false;
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	// 检查输入标签是否与左鼠标按钮（LMB）的输入标签精确匹配。
	// 如果不是左鼠标按钮的输入标签，则调用能力系统组件（ASC）的 AbilityInputTagReleased 方法，并提前返回。
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		// 获取玩家控制器的能力系统组件
		if (GetASC())
		{
			// 通知ASC关于输入标签的释放
			GetASC()->AbilityInputTagReleased(InputTag);
		}
		return;
	}

	// 如果是左鼠标按钮的输入标签，同样调用能力系统组件的 AbilityInputTagReleased 方法。
	if (GetASC())
	{
		GetASC()->AbilityInputTagReleased(InputTag);
	}

	// 检查当前是否不在瞄准状态且Shift键没有被按下。
	if (!bTargeting && !bShiftKeyDown)
	{
		// 获取当前控制的Pawn（通常为角色）
		const APawn* ControlledPawn = GetPawn();
		// 检查跟随时间是否小于短按阈值，并且控制的角色存在
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			// 同步计算从角色当前位置到缓存目的地的导航路径
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(
				this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				// 清除现有的样条点
				Spline->ClearSplinePoints();
				// 遍历导航路径上的所有点，并将它们添加到样条上
				if (!NavPath->PathPoints.IsEmpty())
				{
					for (const FVector& PointLoc : NavPath->PathPoints)
					{
						Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
						// 在世界中绘制一个调试球体以可视化路径点
						DrawDebugSphere(GetWorld(), PointLoc, 8.f, 8, FColor::Green, false, 5.f);
					}
					if (NavPath->PathPoints.Num() > 0)
					{
						// 更新缓存的目的地为路径的最后一段
						CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
						// 设置自动运行标志为真
						bAutoRunning = true;
					}
				}
			}
		}
		// 重置跟随时间和瞄准标志
		FollowTime = 0.f;
		bTargeting = false;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	// 判断左键是否按下
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		return;
	}

	// 判断是否正在瞄准敌人
	if (bTargeting || bShiftKeyDown)
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();

		if (CursorHit.bBlockingHit)
		{
			// 获取鼠标光标下的命中位置
			CachedDestination = CursorHit.ImpactPoint;
		}
		if (APawn* ControllerPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControllerPawn->GetActorLocation()).GetSafeNormal();
			ControllerPawn->AddMovementInput(WorldDirection);
		}
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}


void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 检查成员变量AuraContext是否存在，如果为空则抛出断言错误
	check(AuraContext);

	// 获取当前本地玩家关联的UEnhancedInputLocalPlayerSubsystem子系统实例
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer());
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
	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);

	// 绑定移动动作到Move方法
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPress);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);
	AuraInputComponent->BindAbilityAction(InputConfig, this, &ThisClass::AbilityInputTagPressed,
	                                      &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
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
