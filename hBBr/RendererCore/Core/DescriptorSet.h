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

	void Resize(uint32_t newDescriptorSetCount);

	__forceinline VkDescriptorSetLayout GetDescriptorSetLayout()const { return _descriptorSetLayout; }

	__forceinline Buffer* GetBuffer()const { return _buffer.get(); }

	std::vector<VkDescriptorSet> GetDescriptorSets()const;
private:
	VkDescriptorType				_descriptorTypes;

	std::vector<std::vector<VkDescriptorSet>>_descriptorSets;

	VkDescriptorSetLayout			_descriptorSetLayout = VK_NULL_HANDLE;

	VkShaderStageFlags				_shaderStageFlags;

	uint32_t						_descriptorSetCount;

	std::unique_ptr<Buffer>			_buffer;

	class VulkanRenderer* _renderer;
};
