#pragma once
#include "VulkanManager.h"
#include <vector>
#include <thread>
#include "VMABuffer.h"

class DescriptorSet
{
	friend class VulkanObjectManager;

public:

	DescriptorSet(class VulkanRenderer* renderer);

	~DescriptorSet();

	//创建DescriptorSet绑定
	//如果是UniformBuffer或者StorageBuffer这种类型，会帮忙创建VMABuffer,
	void CreateBinding(
		VkDescriptorType type,
		VkShaderStageFlags shaderStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
	);

	//给当前VkDescriptorType创建Buffer（仅限可用Buffer，像Image类型的就不会创建）
	VMABuffer* CreateBuffer(
		uint32_t bindingIndex,
		VkDeviceSize initSize,
		VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU,
		bool bAlwayMapping = true,
		bool bFocusCreateDedicatedMemory = false,
		HString debugName = "VMABuffer");

	//创建VkDescriptorSetLayer,并分配VkDescriptorSet
	void BuildDescriptorSet();

	VMABuffer* GetBuffer(uint32_t bindingIndex)const {
		auto it = _buffers.find(bindingIndex);
		if (it != _buffers.end())
			return it->second.get();
		return nullptr;
	}

	HBBR_INLINE VkDescriptorSetLayout GetLayout()const {
		return _layout;
	}

	//更新DescriptorSet
	void UpdateDescriptorSet(uint32_t bindingIndex, VkDeviceSize offset, VkDeviceSize range);

	void UpdateDescriptorSetWholeBuffer(uint32_t bindingIndex);

	void UpdateTextureDescriptorSet(std::vector<TextureUpdateInfo> texs, int beginBindingIndex = 0 );

	void UpdateTextureDescriptorSet(std::vector<std::shared_ptr<class Texture2D>> texs, std::vector<VkSampler>samplers, int beginBindingIndex = 0);

	void UpdateStoreTextureDescriptorSet(std::vector<class Texture2D*> textures, int beginBindingIndex = 0);

	HBBR_INLINE std::vector<VkDescriptorType>GetTypes()const {
		return _descriptorTypes;
	}

	//需要更新
	void RefreshDescriptorSet();

	const VkDescriptorSet& GetDescriptorSet();

private:

	//<type , buffer> 可以跟某个类型创建Buffer
	std::map<uint32_t, std::shared_ptr<VMABuffer>>_buffers;

	std::vector<VkDescriptorType>	_descriptorTypes;

	std::vector<VkShaderStageFlags>	_shaderStageFlags;

	//一般不会出现超过4缓冲的,直接固定给4个
	std::vector<VkDescriptorSet>	_descriptorSets;

	class VulkanRenderer* _renderer;

	VkDescriptorSetLayout _layout;

	bool _bUpdateStatus;
};
