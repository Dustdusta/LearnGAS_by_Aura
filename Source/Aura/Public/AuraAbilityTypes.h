#pragma once

#include "GameplayEffectTypes.h"

#include "AuraAbilityTypes.generated.h"

/**
 * 继承FGameplayEffectContext以新增我们需要的一些参数，方便数据的传输
 */
USTRUCT(BlueprintType)
struct FAuraGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	// 新增参数的获取
	bool IsCriticalHit() const { return bIsCriticalHit; }
	bool IsBlockedHit() const { return bIsBlockedHit; }

	void SetIsCriticalHit(bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; }
	void SetIsBlockedHit(bool bInIsBlockedHit) { bIsBlockedHit = bInIsBlockedHit; }

	/** Returns the actual struct used for serialization, subclasses must override this! */
	virtual UScriptStruct* GetScriptStruct() const
	{
		// UE5.3.2特需修改：此处返回无需限定
		// return FGameplayEffectContext::StaticStruct();
		return StaticStruct();

	}

	/** Creates a copy of this context, used to duplicate for later modifications */
	// UE5.3.2特需修改：Duplicate()需返回自定义的类，此处为FAuraGameplayEffectContext
	virtual FAuraGameplayEffectContext* Duplicate() const
	{
		FAuraGameplayEffectContext* NewContext = new FAuraGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	// 复写NetSerialize()以实现添加自定义的数据到传输列表
	/** Custom serialization, subclasses must override this */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

protected:
	
	// 新增的参数
	UPROPERTY()
	bool bIsBlockedHit = false;

	UPROPERTY()
	bool bIsCriticalHit = false;
};

// 创建FAuraGameplayEffectContext的TStructOpsTypeTraits模板
template<>
struct TStructOpsTypeTraits<FAuraGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FAuraGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true,
	};
};