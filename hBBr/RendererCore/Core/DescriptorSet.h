#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <thread>

//每次顶点Buffer最大增加的大小 :
//(UINT32_MAX/64)等于64M
#define BufferSizeRange (UINT32_MAX/1024)

class DescriptorSet
{
public:
	DescriptorSet(VkDescriptorType type, uint32_t bindingCount, VkShaderStageFlags shaderStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	~DescriptorSet();

	void Resize(uint32_t newDescriptorSetCount);

	void CreateBuffer(VkBufferUsageFlags usageFlags, uint64_t bufferSize = BufferSizeRange);

	void MapMemory();

	void UnMapMemory();

	template<class T>
	inline void BufferMapping(T mappingStruct, uint32_t bufferIndex)
	{
		if (_bufferBeginPos.size() > bufferIndex)
		{
			void* bufBegin = (uint8_t*)(_bufferMapping)+_bufferBeginPos[bufferIndex];
			memcpy(bufBegin, &mappingStruct, sizeof(T));
		}
	}

	template<class T>
	inline void BufferMapping(T* mappingData, uint64_t offset, uint64_t bufferSize, uint32_t bufferIndex)
	{
		if (_bufferBeginPos.size() > bufferIndex)
		{
			void* bufBegin = (uint8_t*)(_bufferMapping)+_bufferBeginPos[bufferIndex] + offset;
			memcpy(bufBegin, mappingData, bufferSize);
		}
	}

	template<class T>
	inline void BufferMapping(T* mappingData, uint64_t bufferSize, uint32_t bufferIndex)
	{
		if (_bufferBeginPos.size() > bufferIndex)
		{
			void* bufBegin = (uint8_t*)(_bufferMapping)+_bufferBeginPos[bufferIndex];
			memcpy(bufBegin, mappingData, bufferSize);
		}
	}

	__forceinline const bool IsMapping()const { return _bufferIsMapping; }

	__forceinline void* GetBufferMemory()const{return _bufferMapping;}

	__forceinline VkDescriptorSetLayout GetDescriptorSetLayout()const { return _descriptorSetLayout; }

	__forceinline std::vector<std::vector<VkDescriptorSet>> GetDescriptorSets()const { return _descriptorSets; }
private:
	VkDescriptorType				_descriptorTypes;

	std::vector<std::vector<VkDescriptorSet>>_descriptorSets;

	VkDescriptorSetLayout			_descriptorSetLayout;

	VkShaderStageFlags				_shaderStageFlags;

	uint32_t						_descriptorSetCount ;

	VkBuffer						_buffer;

	VkDeviceMemory					_bufferMemory;

	uint64_t						_bufferCapacity;

	std::vector<uint64_t>			_bufferBeginPos;

	/* The pointer point to buffer */
	void* _bufferMapping = NULL;

	bool _bufferIsMapping = false;
};
