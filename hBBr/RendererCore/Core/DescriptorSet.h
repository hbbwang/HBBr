#pragma once
#include "VulkanManager.h"
#include <vector>
#include <thread>
#include "VMABuffer.h"

class DescriptorSet
{
public:
	DescriptorSet(class VulkanRenderer* renderer, VkDescriptorType type,VkDescriptorSetLayout setLayout,VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU,  VkDeviceSize bufferSizeInit = VMABufferSizeRange, VkShaderStageFlags shaderStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	~DescriptorSet();

	//初始化的时候会根据bindingCount创建相同数量的VkBuffer(数组)
	void BufferMapping(void* mappingData, uint64_t offset, uint64_t bufferSize);

	//可缩放大，也可缩放小
	bool ResizeDescriptorBuffer(VkDeviceSize newSize);

	//只有比之前申请的内存更大，才会执行
	bool ResizeBigDescriptorBuffer(VkDeviceSize newSize);

	void UpdateDescriptorSet(std::vector<uint32_t> bufferRanges, std::vector<uint32_t> offsets, uint32_t dstBinding = 0);

	void UpdateDescriptorSet(uint32_t bufferSize , uint32_t offset = 0, uint32_t dstBinding = 0);

	void UpdateDescriptorSet(uint32_t sameBufferSize, std::vector<uint32_t> offsets, uint32_t dstBinding = 0);

	void UpdateDescriptorSetAll(uint32_t sameBufferSize);

	void UpdateTextureDescriptorSet(std::vector<class Texture2D*> textures, std::vector<VkSampler> samplers, int beginBindingIndex = 0);

	void UpdateStoreTextureDescriptorSet(std::vector<class Texture2D*> textures, int beginBindingIndex = 0);

	void UpdateTextureViewDescriptorSet(std::vector<VkImageView> images, std::vector<VkSampler> samplers);

	HBBR_INLINE VMABuffer* GetBuffer()const { return _buffer.get(); }

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

	std::vector<VkShaderStageFlags>	_shaderStageFlags;

	std::unique_ptr<VMABuffer> _buffer;

	class VulkanRenderer* _renderer;
};
