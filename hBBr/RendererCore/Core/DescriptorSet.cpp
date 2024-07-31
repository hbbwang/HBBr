#include "DescriptorSet.h"
#include "VulkanManager.h"
#include "VulkanRenderer.h"
#include "Texture2D.h"
#include <vector>
#include "VulkanObjectManager.h"

DescriptorSet::DescriptorSet(class VulkanRenderer* renderer)
{
	_renderer = renderer;
	_bUpdateStatus = false;
}

DescriptorSet::~DescriptorSet()
{
}

void DescriptorSet::CreateBinding(VkDescriptorType type, VkShaderStageFlags shaderStageFlags)
{
	_descriptorTypes.push_back(type);
	_shaderStageFlags.push_back(shaderStageFlags);
}

VMABuffer* DescriptorSet::CreateBuffer(uint32_t bindingIndex, VkDeviceSize initSize, VmaMemoryUsage memoryUsage, bool bAlwayMapping, bool bFocusCreateDedicatedMemory, HString debugName)
{
	const auto& manager = VulkanManager::GetManager();
	auto it = _buffers.find(bindingIndex);
	if (it != _buffers.end())
	{
		//此Binding已经创建buffer了！
		return it->second.get();
	}
	//只有Buffer类型才能创建
	std::shared_ptr<VMABuffer> newBuffer;
	std::pair<std::map<uint32_t, std::shared_ptr<VMABuffer>>::iterator, bool> insert_result;
	//UniformBuffer
	if (_descriptorTypes[bindingIndex] & VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || _descriptorTypes[bindingIndex] & VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
	{
		initSize = manager->GetMinUboAlignmentSize(initSize);
		newBuffer.reset(new VMABuffer(initSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryUsage, bAlwayMapping, bFocusCreateDedicatedMemory, debugName));
		insert_result = _buffers.insert(std::pair<uint32_t, std::shared_ptr<VMABuffer>>(bindingIndex, newBuffer));
	}
	//Storage Buffer
	else if (_descriptorTypes[bindingIndex] & VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || _descriptorTypes[bindingIndex] & VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
	{
		initSize = manager->GetMinSboAlignmentSize(initSize);
		newBuffer.reset(new VMABuffer(initSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, memoryUsage, bAlwayMapping, bFocusCreateDedicatedMemory, debugName));
		insert_result = _buffers.insert(std::pair<uint32_t, std::shared_ptr<VMABuffer>>(bindingIndex, newBuffer));
	}
	else
	{
		return nullptr;
	}
	return insert_result.first->second.get();
}

void DescriptorSet::BuildDescriptorSet()
{
	const auto& manager = VulkanManager::GetManager();
	manager->CreateDescripotrSetLayout(_descriptorTypes, _shaderStageFlags, _layout);
	_descriptorSets.resize(manager->GetSwapchainBufferCount());
	for (int i = 0; i < (int)manager->GetSwapchainBufferCount(); i++)
	{
		manager->AllocateDescriptorSet(manager->GetDescriptorPool(), _layout, _descriptorSets[i]);
	}
	RefreshDescriptorSet();
}

void DescriptorSet::UpdateDescriptorSet(uint32_t bindingIndex, VkDeviceSize offset, VkDeviceSize range)
{
	if (_bUpdateStatus)
	{
		const auto& manager = VulkanManager::GetManager();
		manager->UpdateBufferDescriptorSet(
			_buffers[bindingIndex]->GetBuffer(),
			GetDescriptorSet(),
			_descriptorTypes[bindingIndex],
			bindingIndex, offset, range);
	}
}

void DescriptorSet::UpdateDescriptorSetWholeBuffer(uint32_t bindingIndex)
{
	if (_bUpdateStatus)
	{
		const auto& manager = VulkanManager::GetManager();
		manager->UpdateBufferDescriptorSet(
			_buffers[bindingIndex]->GetBuffer(),
			GetDescriptorSet(),
			_descriptorTypes[bindingIndex],
			bindingIndex, 0, VK_WHOLE_SIZE);
	}
}

void DescriptorSet::UpdateTextureDescriptorSet(std::vector<TextureUpdateInfo> texs, int beginBindingIndex)
{
	if (_bUpdateStatus)
	{
		const auto& manager = VulkanManager::GetManager();
		manager->UpdateTextureDescriptorSet(GetDescriptorSet(), texs , beginBindingIndex);
	}
}

void DescriptorSet::UpdateTextureDescriptorSet(std::vector<std::shared_ptr<Texture2D>> texs, std::vector<VkSampler> samplers, int beginBindingIndex)
{
	if (_bUpdateStatus)
	{
		const auto& manager = VulkanManager::GetManager();
		manager->UpdateTextureDescriptorSet(GetDescriptorSet(), texs, samplers, beginBindingIndex);
	}
}

void DescriptorSet::UpdateStoreTextureDescriptorSet(std::vector<class Texture2D*> textures, int beginBindingIndex)
{
	if (_bUpdateStatus)
	{
		const auto& manager = VulkanManager::GetManager();
		manager->UpdateStoreTextureDescriptorSet(GetDescriptorSet(), textures, beginBindingIndex);
	}
}

void DescriptorSet::RefreshDescriptorSet()
{
	VulkanObjectManager::Get()->MarkDescriptorSetUpdate(this);
}

const VkDescriptorSet& DescriptorSet::GetDescriptorSet()
{
	return _descriptorSets[_renderer->GetCurrentFrameIndex()];
}