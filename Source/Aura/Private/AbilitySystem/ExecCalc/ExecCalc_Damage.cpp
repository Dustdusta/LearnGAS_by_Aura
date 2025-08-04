// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interaction/CombatInterface.h"

struct FAuraDamageStatic
{
	// 我们创建并定义了一个名为 armor 的 capture def 属性，它存储在 damage statics 并且我们可以通过 DAMAGE statics 函数来访问它，该 static 函数将返回该静态变量

	// 使用 DECLARE_ATTRIBUTE_CAPTUREDEF 宏声明一个属性捕获定义（CaptureDef），这里针对 Armor 属性。
	// 这个宏用于在编译时生成一些必要的元数据，以便在运行时能够识别和操作 Armor 属性。
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor); // 捕获护甲
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration); // 捕获护甲穿透
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance); // 捕获格挡几率
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance); // 捕获暴击抗性
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance); // 捕获暴击几率
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage); // 捕获暴击伤害

	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance); // 捕获火焰抗性
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance); // 捕获闪电抗性
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance); // 捕获奥术抗性
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance); // 捕获物理抗性

	// 建立一个map以匹配Tag和对应的捕获
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;

	FAuraDamageStatic()
	{
		// 使用 DEFINE_ATTRIBUTE_CAPTUREDEF 宏来具体定义 Armor 属性的捕获定义。
		// 第一个参数 UAuraAttributeSet 指定了属性集的类型。
		// 第二个参数 Armor 是要捕获的属性名。
		// 第三个参数 Target 表示这是目标属性（相对于源属性）。
		// 第四个参数 false 表示这个捕获定义不是可选的。
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, LightningResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArcaneResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, PhysicalResistance, Target, false);

		const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, ArmorDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance, BlockChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitResistance, CriticalHitResistanceDef);

		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration, ArmorPenetrationDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitChance, CriticalHitChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitDamage, CriticalHitDamageDef);

		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, FireResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning, LightningResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Arcane, ArcaneResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical, PhysicalResistanceDef);
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
	// 将 Armor 属性的定义添加到 RelevantAttributesToCapture 中。这意味着在执行自定义游戏效果时，系统会尝试捕获并计算该属性的值。
	RelevantAttributesToCapture.Add(DamageStatic().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatic().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatic().CriticalHitResistanceDef);

	RelevantAttributesToCapture.Add(DamageStatic().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatic().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatic().CriticalHitDamageDef);

	RelevantAttributesToCapture.Add(DamageStatic().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatic().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatic().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatic().PhysicalResistanceDef);
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
	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	int32 SourcePlayerLevel = 1;
	if (SourceAvatar->Implements<UCombatInterface>())
	{
		SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}
	int32 TargetPlayerLevel = 1;
	if (TargetAvatar->Implements<UCombatInterface>())
	{
		TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}
	

	// 获取当前游戏效果规格（Effect Spec），它包含了关于该效果的所有信息。
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// 获取捕获的源和目标标签。这些标签可以用来影响效果的计算方式。
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	// 初始化一个评估参数结构体，用于传递给属性计算方法。
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// Get Damage Set by Caller Magnitude
	float Damage = 0.f;
	for (const TTuple<FGameplayTag, FGameplayTag>& Pair : FAuraGameplayTags::Get().DamageTypesToResistance)
	{
		const FGameplayTag DamageTypeTag = Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;

		// 根据抗性类型获取捕获定义
		// 现在已经碰到全局初始化之间依赖性的关系。本类依赖FAuraGameplayTags初始化，
		// 但FAuraGameplayTags是在模块AssetManager启动时才进行初始化的，因此存在致命Bug!!!
		checkf(FAuraDamageStatic().TagsToCaptureDefs.Contains(ResistanceTag), TEXT("TagsToCaptureDefs doesn't contain Tag: [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
		const FGameplayEffectAttributeCaptureDefinition CaptureDef = FAuraDamageStatic().TagsToCaptureDefs[ResistanceTag];

		// 根据伤害类型查询伤害值
		float DamageTypeValue = Spec.GetSetByCallerMagnitude(DamageTypeTag, false);

		// 获取抗性的数值
		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParameters, Resistance);
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);

		DamageTypeValue *= (100.f - Resistance) / 100.f;
		Damage += DamageTypeValue;
	}

	// Capture BlockChance on Target, and determine if there was a successful Back
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatic().BlockChanceDef, EvaluationParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);

	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;

	// 获取bBlocked并设置到EffectContext中
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
	UAuraAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);

	// If Block, halve the damage.
	Damage = bBlocked ? Damage / 2.f : Damage;

	// 计算护甲和护甲穿透
	// 获取 Armor 的值并存储到变量 TargetArmor
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatic().ArmorDef, EvaluationParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(TargetArmor, 0.f);

	// 获取 ArmorPenetration 的值并存储到变量 SourceArmorPenetration
	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatic().ArmorPenetrationDef, EvaluationParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0.f);

	const UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);

	// 通过 CharacterClassInfo 上的 DamageCalculationCoefficients，获取护甲穿透的计算系数
	const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("ArmorPenetration"), FString());
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourcePlayerLevel);

	// ArmorPenetration ignores a percentage of the Target's Armor
	const float EffectiveArmor = TargetArmor *= (100 - SourceArmorPenetration * ArmorPenetrationCoefficient) / 100.f;
	// 通过 CharacterClassInfo 上的 DamageCalculationCoefficients，获取护甲对伤害的计算系数
	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("EffectiveArmor"), FString());
	const float EffectiveArmorCurveCoefficient = EffectiveArmorCurve->Eval(TargetPlayerLevel);
	// Armor ignores a percentage of incoming Damage 
	Damage *= (100 - EffectiveArmor * EffectiveArmorCurveCoefficient) / 100.f;


	// 计算暴击相关
	// 获取 CriticalHitChance 的值并存储到变量 SourceCriticalHitChance
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatic().CriticalHitChanceDef, EvaluationParameters, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(SourceCriticalHitChance, 0.f);

	// 获取 CriticalHitResistance 的值并存储到变量 TargetCriticalHitResistance
	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatic().CriticalHitResistanceDef, EvaluationParameters, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(TargetCriticalHitResistance, 0.f);

	// 获取 CriticalHitDamage 的值并存储到变量 SourceCriticalHitDamage
	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatic().CriticalHitDamageDef, EvaluationParameters, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(SourceCriticalHitDamage, 0.f);

	// 通过 CharacterClassInfo 上的 DamageCalculationCoefficients，获取暴击抗性的计算系数
	const FRealCurve* CriticalHitResistanceCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("CriticalHitResistance"), FString());
	const float CriticalHitResistanceCoefficient = CriticalHitResistanceCurve->Eval(TargetPlayerLevel);

	// Critical Hit Resistance reduces Critical Hit Chance by a certain percentage
	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalHitResistanceCoefficient;
	const bool bCriticalHit = FMath::RandRange(1, 100) < EffectiveCriticalHitChance;


	// 获取bCriticalHit并设置到EffectContext中
	UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);

	// Double damage plus a bonus if critical hit
	Damage = bCriticalHit ? Damage * 2.f + SourceCriticalHitDamage : Damage;


	// 创建一个新的 FGameplayModifierEvaluatedData 实例，表示一个属性修改器。这里指定了要修改的属性（UAuraAttributeSet::GetIncomingDamageAttribute()）、操作类型（Additive）以及修改的值（Damage）。
	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	// 将计算出的属性修改器添加到输出中。这个修改器将会被应用到目标角色的相应属性上。
	OutExecutionOutput.AddOutputModifier(EvaluatedData);


	/*
	// 定义一个变量来存储护甲值。
	float Armor = 0.f;
	// 尝试计算捕获的Armor属性的数值。使用DamageStatic().ArmorDef作为属性定义。如果计算成功，结果会存储在Armor变量中。
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatic().ArmorDef, EvaluationParameters, Armor);
	// 确保护甲值至少为0，防止负数。
	Armor = FMath::Max<float>(Armor, 0.f);
	// 增加护甲值1。这可能是一个简单的示例，实际游戏中可能需要更复杂的逻辑。
	++Armor;

	// 创建一个新的FGameplayModifierEvaluatedData实例，表示一个属性修改器。这里指定了要修改的属性（DamageStatic().ArmorProperty）、操作类型（Additive）以及修改的值（Armor）。
	const FGameplayModifierEvaluatedData EvaluatedData(DamageStatic().ArmorProperty, EGameplayModOp::Additive, Armor);
	// 将计算出的属性修改器添加到输出中。这个修改器将会被应用到目标角色的相应属性上。
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
	*/
}


/*--------------------
██       ██
█ ██       ██
█   ██       ██
█     ██       ██
█       ██       ██
█         ██████████
█       ████      ██
█     ██  ██      ██
█   ██    ██      ██
█ ██      ██      ██
██        ██      ██
  ██      ██      ██
	██    ██      ██
	  ██  ██      ██
		████      ██
--------------------*/
