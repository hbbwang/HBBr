#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <thread>
#include "Buffer.h"
//每次顶点Buffer最大增加的大小 :
//(UINT32_MAX/64)等于64M
#define BufferSizeRange (UINT32_MAX/1024)

class DescriptorSet
{
public:
	DescriptorSet(class VulkanRenderer* renderer , VkDescriptorType type, uint32_t bindingCount, VkShaderStageFlags shaderStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	~DescriptorSet();

	__forceinline VkDescriptorSetLayout GetDescriptorSetLayout()const { return _descriptorSetLayout; }

	//初始化的时候会根据bindingCount创建相同数量的VkBuffer(数组),bufferIndex是为了识别该数组,如果是1，则保持默认0即可
	void BufferMapping(void* mappingData, uint64_t bufferSize, int bufferIndex = 0);

	//初始化的时候会根据bindingCount创建相同数量的VkBuffer(数组),bufferIndex是为了识别该数组,如果是1，则保持0即可
	void BufferMapping(void* mappingData, uint64_t offset, uint64_t bufferSize, int bufferIndex);

	//初始化的时候会根据bindingCount创建相同数量的VkBuffer(数组),bufferIndex是为了识别该数组,如果是1，则保持0即可
	//该函数会影响CurrentOffset,类似文件流一样，会根据bufferSize更新下一个Buffer的位置
	void BufferMappingOffset(void* mappingData, uint64_t bufferSize, int bufferIndex);

	inline void ResetBufferMappingOffset()
	{
		_currentOffset = 0;
	}

	inline void SetBufferMappingOffset(VkDeviceSize newOffset)
	{
		_currentOffset = newOffset;
	}

	inline uint64_t GetCurrentOffset() const
	{
		return _currentOffset;
	}

	__forceinline Buffer* GetBuffer(int bufferIndex = 0)const { return _buffers[bufferIndex].get(); }

	__forceinline std::vector<VkDescriptorType> GetTypes()const { return _descriptorTypes; }

	const VkDescriptorSet& GetDescriptorSet();
private:
	std::vector<VkDescriptorType>	_descriptorTypes;

	std::vector<VkDescriptorSet>	_descriptorSets;

	VkDescriptorSetLayout			_descriptorSetLayout = VK_NULL_HANDLE;

	VkShaderStageFlags				_shaderStageFlags;

	std::vector<std::unique_ptr<Buffer>> _buffers;

	class VulkanRenderer* _renderer;

	VkDeviceSize _currentOffset = 0;
};
