#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * AuraGameplayTags
 * 
 * Singleton containing native Gameplay Tags 
 */

struct FAuraGameplayTags
{
public:
	static FAuraGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	/*
	 * Primary Attributes
	 */
	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Intelligence;
	FGameplayTag Attributes_Primary_Resistance;
	FGameplayTag Attributes_Primary_Vigor;

	/*
	 * Secondary Attributes
	 */
	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	FGameplayTag Attributes_Secondary_BlockChance;
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	FGameplayTag Attributes_Secondary_CriticalHitResistance;
	FGameplayTag Attributes_Secondary_HealthRegenRate;
	FGameplayTag Attributes_Secondary_ManaRegenRate;
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxMana;

	/**
	 * 伤害抵抗
	 */
	FGameplayTag Attributes_Resistance_Fire;
	FGameplayTag Attributes_Resistance_Lightning;
	FGameplayTag Attributes_Resistance_Arcane;
	FGameplayTag Attributes_Resistance_Physical;


	/*
	 * Input Tags
	 */
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;

	/*
	 * 伤害相关 
	 */
	FGameplayTag Damage;
	FGameplayTag Damage_Fire; // 火
	FGameplayTag Damage_Lightning; // 电
	FGameplayTag Damage_Arcane; // 奥术
	FGameplayTag Damage_Physical; // 物理

	/**
	 * 可以攻击能力
	 */
	FGameplayTag Abilities_Attack;
	FGameplayTag Abilities_Summon;

	/**
	 * 标记攻击的类型
	 */
	FGameplayTag CombatSocket_Weapon;
	FGameplayTag CombatSocket_RightHand;
	FGameplayTag CombatSocket_LeftHand;
	FGameplayTag CombatSocket_Tail;

	/**
	 * 标记攻击的蒙太奇的类型
	 */
	FGameplayTag Montage_Attack_1;
	FGameplayTag Montage_Attack_2;
	FGameplayTag Montage_Attack_3;
	FGameplayTag Montage_Attack_4;
	
	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistance;

	FGameplayTag Effect_HitReact;

private:
	static FAuraGameplayTags GameplayTags;
};
