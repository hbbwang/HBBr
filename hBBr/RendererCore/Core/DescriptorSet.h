#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <thread>
#include "Buffer.h"

class DescriptorSet
{
public:
	DescriptorSet(class VulkanRenderer* renderer , VkDescriptorType type, uint32_t bindingCount,VkDeviceSize bufferSizeInit = BufferSizeRange, VkShaderStageFlags shaderStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	~DescriptorSet();

	HBBR_INLINE VkDescriptorSetLayout GetDescriptorSetLayout()const { return _descriptorSetLayout; }

	//初始化的时候会根据bindingCount创建相同数量的VkBuffer(数组),bufferIndex是为了识别该数组,如果是1，则保持0即可
	void BufferMapping(void* mappingData, uint64_t offset, uint64_t bufferSize, int bufferIndex = 0);

	bool ResizeDescriptorBuffer(VkDeviceSize newSize , int bufferIndex = 0);

	void UpdateDescriptorSet(std::vector<uint32_t> bufferRanges, std::vector<uint32_t> offsets);

	void UpdateDescriptorSet(uint32_t bufferSize , uint32_t offset = 0);

	void UpdateDescriptorSet(uint32_t sameBufferSize, std::vector<uint32_t> offsets);

	void UpdateDescriptorSetAll(uint32_t sameBufferSize);

	HBBR_INLINE Buffer* GetBuffer(int bufferIndex = 0)const { return _buffers[bufferIndex].get(); }

	HBBR_INLINE std::vector<VkDescriptorType> GetTypes()const { return _descriptorTypes; }

	HBBR_INLINE void NeedUpdate() {
		memset(_needUpdates.data(), 1, sizeof(uint8_t) * _needUpdates.size());
	}

	const VkDescriptorSet& GetDescriptorSet();

	const VkDescriptorSet& GetDescriptorSet(int index);

private:
	std::vector<VkDescriptorType>	_descriptorTypes;

	std::vector<VkDescriptorSet>	_descriptorSets;

	std::vector<uint8_t>	_needUpdates;

	VkDescriptorSetLayout			_descriptorSetLayout = VK_NULL_HANDLE;

	VkShaderStageFlags				_shaderStageFlags;

	std::vector<std::unique_ptr<Buffer>> _buffers;

	class VulkanRenderer* _renderer;
};
