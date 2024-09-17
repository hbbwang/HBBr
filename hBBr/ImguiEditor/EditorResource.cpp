#include "EditorResource.h"

#include "Imgui/imgui.h"
#include "Imgui/backends/imgui_impl_sdl3.h"
#include "Imgui/backends/imgui_impl_vulkan.h"
#include "Texture2D.h"
#include "ImageTool.h"
#include "FileSystem.h"
#include "VMABuffer.h"

std::unique_ptr<EditorResource> EditorResource::_ptr;

void EditorResource::Init()
{
	_all_images.reserve(1024);

	HString configPath = FileSystem::GetConfigAbsPath();

	auto* manager = VulkanManager::GetManager();

	manager->CreateDescripotrSetLayout(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
		1, 
		_img_descriptorSetLayout, 
		VK_SHADER_STAGE_FRAGMENT_BIT);

	VkCommandBuffer cmdbuf;
	manager->AllocateCommandBuffer(manager->GetCommandPool(), cmdbuf);
	manager->BeginCommandBuffer(cmdbuf); 
	{
		_icon_eyeOpen = LoadTexture(32, 32, FileSystem::Append(FileSystem::GetConfigAbsPath(), "Theme/Icons/Icon_eye_open.png"), cmdbuf);
		_icon_eyeClose = LoadTexture(32, 32, FileSystem::Append(FileSystem::GetConfigAbsPath(), "Theme/Icons/Icon_eye_close.png"), cmdbuf);
		_icon_levelIcon = LoadTexture(32, 32, FileSystem::Append(FileSystem::GetConfigAbsPath(), "Theme/Icons/Icon_level.png"), cmdbuf);
		_icon_objectIcon = LoadTexture(32, 32, FileSystem::Append(FileSystem::GetConfigAbsPath(), "Theme/Icons/Icon_object.png"), cmdbuf);
		_icon_search = LoadTexture(32, 32, FileSystem::Append(FileSystem::GetConfigAbsPath(), "Theme/Icons/ICON_SEARCH.png"), cmdbuf);
	}
	manager->EndCommandBuffer(cmdbuf);
	manager->SubmitQueueImmediate({ cmdbuf });
	vkQueueWaitIdle(manager->GetGraphicsQueue());
	manager->FreeCommandBuffer(manager->GetCommandPool(), cmdbuf);

	//上传GPU完成，清理旧数据
	for (auto& i : _all_images)
	{
		i.imageData.reset();
		i.imageData = nullptr;
		i.vmaBuffer.reset();
		i.vmaBuffer = nullptr;
		if (i.stageImage != nullptr)
		{
			manager->VMADestroyImage(i.stageImage, i.stageImageAllocation);
		}
	}
}

void EditorResource::Release()
{
	auto* manager = VulkanManager::GetManager();
	manager->DestroyDescriptorSetLayout(_img_descriptorSetLayout);
	for (auto& i : _all_images)
	{
		//ImGui_ImplVulkan_RemoveTexture(i.descriptorSet);
		manager->FreeDescriptorSet(manager->GetDescriptorPool(), i.descriptorSet);
		i.descriptorSet = nullptr;
		manager->DestroyImageView(i.imageView);
		manager->VMADestroyImage(i.image, i.imageAllocation);
	}
	_ptr.reset();
}

EditorImage* EditorResource::LoadTexture(uint32_t w, uint32_t h, HString path, VkCommandBuffer cmdBuf)
{
	auto* manager = VulkanManager::GetManager();
	EditorImage result;
	result.imageData = ImageTool::LoadImage8Bit(path.c_str());
	
	bool bResizeImage = false;
	//期望大小不相同，需要缩放一下(必须比原图小或者相同)

	if (w > result.imageData->data_header.width)
	{
		w = result.imageData->data_header.width;
	}
	if (h > result.imageData->data_header.height)
	{
		h = result.imageData->data_header.height;
	}
	if (w < result.imageData->data_header.width)
		bResizeImage = true;
	if (h < result.imageData->data_header.width)
		bResizeImage = true;

	//需要缩小图像
	if (!bResizeImage)
	{
		manager->VMACreateImage(result.imageData->data_header.width, result.imageData->data_header.height,
			result.imageData->texFormat,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			result.image, result.imageAllocation, &result.imageAllocationInfo, 1, 1);

		result.vmaBuffer.reset(new VMABuffer(
			result.imageData->imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY, false, false, "Editor Load Texture Buffer"));

		manager->CmdCopyImageDataToTexture(cmdBuf, result.vmaBuffer.get(), result.image,
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, result.imageData.get());

		manager->CreateImageView(result.image, result.imageData->texFormat, VK_IMAGE_ASPECT_COLOR_BIT, result.imageView);
	}
	else
	{
		manager->VMACreateImage(
			result.imageData->data_header.width, result.imageData->data_header.height,
			result.imageData->texFormat,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			result.stageImage, result.stageImageAllocation, &result.stageImageAllocationInfo, 1, 1);

		result.vmaBuffer.reset(new VMABuffer(
			result.imageData->imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY, false, false, "Editor Load Texture Buffer"));

		manager->CmdCopyImageDataToTexture(cmdBuf, result.vmaBuffer.get(), result.stageImage,
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, result.imageData.get());

		//
		manager->VMACreateImage(w, h, result.imageData->texFormat,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			result.image, result.imageAllocation, &result.imageAllocationInfo, 1, 1);

		manager->Transition(cmdBuf, result.stageImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		manager->Transition(cmdBuf, result.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		manager->CmdColorBitImage(cmdBuf, result.stageImage, result.image,
			{ result.imageData->data_header.width, result.imageData->data_header.height },
			{ w,h });
		manager->Transition(cmdBuf, result.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		manager->CreateImageView(result.image, result.imageData->texFormat, VK_IMAGE_ASPECT_COLOR_BIT, result.imageView);

	}
	//result.descriptorSet = ImGui_ImplVulkan_AddTexture(Texture2D::GetSampler(TextureSampler_Linear_Clamp), result.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	{
		VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = manager->GetDescriptorPool();
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &_img_descriptorSetLayout;
		VkResult err = vkAllocateDescriptorSets(manager->GetDevice(), &alloc_info, &result.descriptorSet);
	}
	{
		VkDescriptorImageInfo desc_image[1] = {};
		desc_image[0].sampler = Texture2D::GetSampler(TextureSampler_Linear_Clamp);
		desc_image[0].imageView = result.imageView;
		desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkWriteDescriptorSet write_desc[1] = {};
		write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_desc[0].dstSet = result.descriptorSet;
		write_desc[0].descriptorCount = 1;
		write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_desc[0].pImageInfo = desc_image;
		vkUpdateDescriptorSets(manager->GetDevice(), 1, write_desc, 0, nullptr);
	}

	_all_images.push_back(result);

	return &_all_images[_all_images.size() - 1];
}
