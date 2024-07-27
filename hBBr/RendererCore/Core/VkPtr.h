#pragma once
#include <vector>
#include <memory>
#include <atomic>
#include <VulkanObjectManager.h>

class RefCounter
{
public:
	inline RefCounter()
	{
		_refCount = 1;
	}
	inline void Add()
	{
		_refCount++;
	}
	inline void Sub()
	{
		_refCount--;
	}
	inline const int Get() const
	{
		return _refCount;
	}
private:
	std::atomic<int> _refCount;
};

class VkPtrBase
{
	friend class VulkanObjectManager;
public:
	inline int GetRefCount()
	{
		return _refCounter->Get();
	}
	inline void IsImmediateRelease(bool bImmediateRelease)
	{
		_bImmediateRelease = bImmediateRelease ? 1 : 0;
	}
protected:
	void* _ptr;
	std::size_t _typeHash;
	RefCounter* _refCounter;
	uint8_t _bImmediateRelease;
};
 
//这不是普通的智能指针，这是专门给VulkanObject GC使用的
//销毁逻辑主要集中在VulkanObjectManager里，这里只是作为引用计数作用
//当引用计数 等于0 的时候，会把指针发送到VulkanObjectManager等待GC,
template <class T>
class VkPtr :public VkPtrBase
{
	friend class VulkanObjectManager;
public:
	//装载
	inline VkPtr(T* ptr)
	{
		_typeHash = typeid(T).hash_code();
		_ptr = ptr;
		_refCounter = new RefCounter();
	}
	inline VkPtr(const VkPtr& obj)
	{
		_ptr = obj._ptr;
		_typeHash = obj._typeHash;		
		_bImmediateRelease = obj._bImmediateRelease;
		_refCounter = obj._refCounter;
		_refCounter->Add();
	}
	inline void Clear()
	{
		RefSub();
		_ptr = nullptr;
		_typeHash = 0;
		_refCounter = nullptr;
	}
	inline ~VkPtr()
	{
		RefSub();
	}
	inline VkPtr& operator=(const VkPtr& obj)
	{
		RefSub();
		_ptr = obj._ptr;
		_typeHash = obj._typeHash;
		_bImmediateRelease = obj._bImmediateRelease;
		_refCounter = obj._refCounter;
		_refCounter->Add();
		return *this;
	}
	inline T* Get()
	{
		return (T*)_ptr;
	}
private:
	inline void RefSub()
	{
		_refCounter->Sub();
		if (_refCounter->Get() <= 0)
		{
			//不自动释放，而是交给GC，安全释放(至少等待3帧之后再销毁)
			VulkanObjectManager::Get()->VulkanPtrGC(this);
		}
	}
};
