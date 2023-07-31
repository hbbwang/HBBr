#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <thread>

//每次顶点Buffer最大增加的大小 :
//(UINT32_MAX/64)等于64M
#define BufferSizeRange (UINT32_MAX/1024)

struct BufferWaitToRelease
{
	int				safeDestoryOldBuffer = -1;
	VkBuffer		old_buffer = VK_NULL_HANDLE;
	VkDeviceMemory	old_bufferMemory = VK_NULL_HANDLE;
};

class Buffer
{
public:
	Buffer(VkBufferUsageFlags usageFlags, uint64_t bufferSize = BufferSizeRange);
	~Buffer();
	void Resize(uint64_t newSize);

	void AddSize(uint64_t newSize);

	void CreateBuffer(VkBufferUsageFlags usageFlags, uint64_t bufferSize = BufferSizeRange);

	void MapMemory();

	void UnMapMemory();

	inline void BufferMapping(void* mappingData, uint64_t bufferSize)
	{
		void* bufBegin = (uint8_t*)(_bufferMapping);
		memcpy(bufBegin, mappingData, bufferSize);
	}

	inline void BufferMapping(void* mappingData, uint64_t offset, uint64_t bufferSize)
	{
		void* bufBegin = (uint8_t*)(_bufferMapping) + offset;
		memcpy(bufBegin, mappingData, bufferSize);
	}

	template<class T>
	inline void BufferMapping(T mappingStruct)
	{
		void* bufBegin = (uint8_t*)(_bufferMapping);
		memcpy(bufBegin, &mappingStruct, sizeof(T));
	}

	template<class T>
	inline void BufferMapping(T* mappingData, uint64_t offset, uint64_t bufferSize)
	{
		void* bufBegin = (uint8_t*)(_bufferMapping) + offset;
		memcpy(bufBegin, mappingData, bufferSize);
	}

	template<class T>
	inline void BufferMapping(T* mappingData, uint64_t bufferSize)
	{
		void* bufBegin = (uint8_t*)(_bufferMapping);
		memcpy(bufBegin, mappingData, bufferSize);
	}

	__forceinline const bool IsMapping()const { return _bufferIsMapping; }

	__forceinline void* GetBufferMemory()const{return _bufferMapping;}

	VkBuffer GetBuffer();

private:

	VkBuffer						_buffer = VK_NULL_HANDLE;

	VkDeviceMemory					_bufferMemory = VK_NULL_HANDLE;

	std::vector<BufferWaitToRelease>_oldBuffer;

	uint64_t						_bufferCapacity;

	VkBufferUsageFlags				_bufferUsage;

	/* The pointer point to buffer */
	void* _bufferMapping = NULL;

	bool _bufferIsMapping = false;
};
