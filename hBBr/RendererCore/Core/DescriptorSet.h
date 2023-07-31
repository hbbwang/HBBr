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

	void BufferMapping(void* mappingData, uint64_t bufferSize, int bufferIndex = 0);

	inline void BufferMapping(void* mappingData, uint64_t offset, uint64_t bufferSize , int bufferIndex = 0)
	{
		_buffers[bufferIndex]->BufferMapping(mappingData, offset, bufferSize);
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
};
