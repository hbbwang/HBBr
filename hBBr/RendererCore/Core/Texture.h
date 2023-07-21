#pragma once
//基层HObject,管理对象
#include "Common.h"
#include <memory>
#include <vector>
#include <iostream>
#include "HString.h"

#include "vulkan.h"

class Texture
{
public:
	Texture();
	__forceinline VkImage GetTexture()const {
		return _iamge;
	}
private:
	VkImage _iamge;
};

class FrameBufferTexture
{
public:

	FrameBufferTexture();

	std::vector<Texture*> _frameBufferTextures;
};