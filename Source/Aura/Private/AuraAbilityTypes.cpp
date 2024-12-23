#include "AuraAbilityTypes.h"


bool FAuraGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	// 定义一个8位无符号整数来存储哪些成员需要被复制的信息。
	uint32 RepBits = 0;
	// 如果正在保存（即从服务器向客户端发送数据）
	if (Ar.IsSaving())
	{
		// 检查各个成员变量是否有效，并设置RepBits相应位，表示这些成员需要被复制。
		/*------------父类原先有的数据----------------*/
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}
		if (bReplicateEffectCauser && EffectCauser.IsValid() )
		{
			RepBits |= 1 << 1;
		}
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		/*-------------------------------------------*/
		if (bIsBlockedHit)
		{
			RepBits |= 1 << 7;
		}
		if (bIsCriticalHit)
		{
			RepBits |= 1 << 8;
		}
	}
	// 序列化RepBits，使用7个bit来表示上述7个条件的状态。
	Ar.SerializeBits(&RepBits,9);

	// 根据RepBits的值，决定哪些成员变量需要被序列化。
	/*------------父类原先有的数据----------------*/
	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	if (RepBits & (1 << 5))
	{
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}
	/*-------------------------------------------*/
	if (RepBits & (1 << 7))
	{
		Ar << bIsBlockedHit;
	}
	if (RepBits & (1 << 8))
	{
		Ar << bIsCriticalHit;
	}

	// 如果正在加载，则添加发起者以初始化相关的组件。
	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get()); // Just to initialize InstigatorAbilitySystemComponent
	}
	// 设置输出参数为成功，并返回true。
	bOutSuccess = true;

	
	return true;
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