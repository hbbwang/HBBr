#pragma once

#include "Texture2D.h"

class TextureCube : public Texture2D
{
	friend class VulkanManager;
	friend class ContentManager;
public:
	TextureCube();

	~TextureCube();

	HBBR_API static std::shared_ptr<TextureCube> CreateTextureCube(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, HString textureName = "TextureCube", uint32_t miplevel = 1);

	HBBR_API static std::weak_ptr<TextureCube> LoadAsset(HGUID guid, VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	HBBR_API virtual void SaveAsset(HString path)override;
};
