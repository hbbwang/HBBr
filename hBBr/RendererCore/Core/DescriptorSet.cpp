#include "DescriptorSet.h"
#include "VulkanManager.h"
#include "VulkanRenderer.h"
#include "Texture2D.h"
#include <vector>
#include "VulkanObjectManager.h"
#include "ConsoleDebug.h"

DescriptorSet::DescriptorSet(class VulkanRenderer* renderer)
{
	_renderer = renderer;
}

DescriptorSet::~DescriptorSet()
{
	const auto& manager = VulkanManager::GetManager();
	for (auto& i : _descriptorSets)
	{
		manager->FreeDescriptorSet(manager->GetDescriptorPool(), i);
	}	
	if (_layout)
	{
		manager->DestroyDescriptorSetLayout(_layout);
	}
	for (auto& i : _buffers)
	{
		i.second.reset();
	}
	_buffers.clear();
}

void DescriptorSet::CreateBinding(VkDescriptorType type, VkShaderStageFlags shaderStageFlags)
{
	_descriptorTypes.push_back(type);
	_shaderStageFlags.push_back(shaderStageFlags);
}

void DescriptorSet::CreateBindings(uint32_t bindingCount, VkDescriptorType type, VkShaderStageFlags shaderStageFlags)
{
	std::vector<VkDescriptorType>	new_descriptorTypes(bindingCount, type);
	std::vector<VkShaderStageFlags>	new_shaderStageFlags(bindingCount, shaderStageFlags);
	_descriptorTypes.insert(_descriptorTypes.end(), new_descriptorTypes.begin(), new_descriptorTypes.end());
	_shaderStageFlags.insert(_shaderStageFlags.end(), new_shaderStageFlags.begin(), new_shaderStageFlags.end());
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

void DescriptorSet::BuildDescriptorSetLayout()
{
	const auto& manager = VulkanManager::GetManager();
	
	for (auto& i : _bNeedUpdate)
	{
		i.resize(_descriptorTypes.size());
	}

	manager->CreateDescripotrSetLayout(_descriptorTypes, _shaderStageFlags, _layout);
	RefreshDescriptorSetAllBinding();
}

void DescriptorSet::BufferMapping(uint32_t bindingIndex, void* data, VkDeviceSize offset, VkDeviceSize dataSize, VkCommandBuffer cmdBuf)
{
	auto it = _buffers.find(bindingIndex);
	if (_buffers.end() != it)
	{
		it->second->Mapping(data, offset, dataSize, cmdBuf);
	}
}

void DescriptorSet::UpdateBufferDescriptorSet(uint32_t bindingIndex, VkDeviceSize offset, VkDeviceSize range)
{
	auto it = _buffers.find(bindingIndex);
	if (_bNeedUpdate[_renderer->GetCurrentFrameIndex()][bindingIndex] == 1 && _buffers.end() != it)
	{
		_bNeedUpdate[_renderer->GetCurrentFrameIndex()][bindingIndex] = 0;
		const auto& manager = VulkanManager::GetManager();
		manager->UpdateBufferDescriptorSet(
			it->second->GetBuffer(),
			GetDescriptorSet(),
			_descriptorTypes[bindingIndex],
			bindingIndex, offset, range);
	}
}

void DescriptorSet::UpdateDescriptorSetWholeBuffer(uint32_t bindingIndex)
{
	auto it = _buffers.find(bindingIndex);
	if (_bNeedUpdate[_renderer->GetCurrentFrameIndex()][bindingIndex] == 1 && _buffers.end() != it)
	{
		_bNeedUpdate[_renderer->GetCurrentFrameIndex()][bindingIndex] = 0;
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
	bool bTextureReset = false;
	for (auto& i : texs)
	{
		if (i.texture->_bReset)
		{
			bTextureReset = true;
			break;
		}
	}
	if (_bNeedUpdate[_renderer->GetCurrentFrameIndex()][beginBindingIndex] == 1 || bTextureReset)
	{
		_bNeedUpdate[_renderer->GetCurrentFrameIndex()][beginBindingIndex] = 0;
		const auto& manager = VulkanManager::GetManager();
		manager->UpdateTextureDescriptorSet(GetDescriptorSet(), texs , beginBindingIndex);
	}
}

void DescriptorSet::UpdateTextureDescriptorSet(std::vector<std::shared_ptr<Texture2D>> texs, std::vector<VkSampler> samplers, int beginBindingIndex)
{
	bool bTextureReset = false;
	for (auto& i : texs)
	{
		if (i->_bReset)
		{
			bTextureReset = true;
			break;
		}
	}
	if (_bNeedUpdate[_renderer->GetCurrentFrameIndex()][beginBindingIndex] == 1 || bTextureReset)
	{
		_bNeedUpdate[_renderer->GetCurrentFrameIndex()][beginBindingIndex] = 0;
		const auto& manager = VulkanManager::GetManager();
		manager->UpdateTextureDescriptorSet(GetDescriptorSet(), texs, samplers, beginBindingIndex);
	}
}

void DescriptorSet::UpdateStoreTextureDescriptorSet(std::vector<class Texture2D*> textures, int beginBindingIndex)
{
	bool bTextureReset = false;
	for (auto& i : textures)
	{
		if (i->_bReset)
		{
			bTextureReset = true;
			break;
		}
	}
	if (_bNeedUpdate[_renderer->GetCurrentFrameIndex()][beginBindingIndex] == 1  || bTextureReset)
	{
		_bNeedUpdate[_renderer->GetCurrentFrameIndex()][beginBindingIndex] = 0;
		const auto& manager = VulkanManager::GetManager();
		manager->UpdateStoreTextureDescriptorSet(GetDescriptorSet(), textures, beginBindingIndex);
	}
}

void DescriptorSet::RefreshDescriptorSet(uint32_t bindingIndex)
{
	for (auto& i : _bNeedUpdate)
		i[bindingIndex] = 1;
}

void DescriptorSet::RefreshDescriptorSetAllBinding()
{
	for (auto& i : _bNeedUpdate)
		memset(i.data(), 1, sizeof(uint8_t) * _descriptorTypes.size());
}

const VkDescriptorSet& DescriptorSet::GetDescriptorSet()
{
	if (_descriptorSets[_renderer->GetCurrentFrameIndex()] == nullptr)
	{
		const auto& manager = VulkanManager::GetManager();
		manager->AllocateDescriptorSet(manager->GetDescriptorPool(), _layout, _descriptorSets[_renderer->GetCurrentFrameIndex()]);
	}
	return _descriptorSets[_renderer->GetCurrentFrameIndex()];
}