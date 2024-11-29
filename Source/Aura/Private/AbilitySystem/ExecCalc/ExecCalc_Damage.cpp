// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

struct FAuraDamageStatic
{
	// 我们创建并定义了一个名为 armor 的 capture def 属性，它存储在 damage statics 并且我们可以通过 DAMAGE statics 函数来访问它，该 static 函数将返回该静态变量

	// 使用 DECLARE_ATTRIBUTE_CAPTUREDEF 宏声明一个属性捕获定义（CaptureDef），这里针对 Armor 属性。
	// 这个宏用于在编译时生成一些必要的元数据，以便在运行时能够识别和操作 Armor 属性。
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);

	FAuraDamageStatic()
	{
		// 使用 DEFINE_ATTRIBUTE_CAPTUREDEF 宏来具体定义 Armor 属性的捕获定义。
		// 第一个参数 UAuraAttributeSet 指定了属性集的类型。
		// 第二个参数 Armor 是要捕获的属性名。
		// 第三个参数 Target 表示这是目标属性（相对于源属性）。
		// 第四个参数 false 表示这个捕获定义不是可选的。
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
	}
};

/**
 * 静态函数，返回一个 FAuraDamageStatic 的实例
 * @return 返回静态变量的引用
 */
static const FAuraDamageStatic& DamageStatic()
{
	// 定义一个静态变量 DamageStatic，并初始化为 FAuraDamageStatic 类型的对象。
	// 由于它是静态的，因此它只会在第一次调用此函数时被创建，并且在程序的整个生命周期内保持不变。
	static FAuraDamageStatic DamageStatic;
	// 返回静态变量的引用。
	return DamageStatic;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatic().ArmorDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// 这里我们可以决定如何更改各种属性
	// 从ExecutionParams中获取源和目标的能力系统组件（Ability System Component, ASC）。
	// 这些组件通常用于管理和应用角色的各种属性和能力。
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	// 获取源和目标的AvatarActor（通常是玩家控制的角色）。
	// 如果ASC为空，则AvatarActor也会是nullptr。
	const AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	const AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	// 获取当前游戏效果规格（Effect Spec），它包含了关于该效果的所有信息。
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// 获取捕获的源和目标标签。这些标签可以用来影响效果的计算方式。
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	// 初始化一个评估参数结构体，用于传递给属性计算方法。
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// 定义一个变量来存储护甲值。
	float Armor = 0.f;
	// 尝试计算捕获的Armor属性的数值。使用DamageStatic().ArmorDef作为属性定义。如果计算成功，结果会存储在Armor变量中。
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatic().ArmorDef, EvaluationParameters, Armor);
	// 确保护甲值至少为0，防止负数。
	Armor = FMath::Max<float>(0.f, Armor);
	// 增加护甲值1。这可能是一个简单的示例，实际游戏中可能需要更复杂的逻辑。
	++Armor;

	// 创建一个新的FGameplayModifierEvaluatedData实例，表示一个属性修改器。这里指定了要修改的属性（DamageStatic().ArmorProperty）、操作类型（Additive）以及修改的值（Armor）。
	const FGameplayModifierEvaluatedData EvaluatedData(DamageStatic().ArmorProperty, EGameplayModOp::Additive, Armor);
	// 将计算出的属性修改器添加到输出中。这个修改器将会被应用到目标角色的相应属性上。
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
