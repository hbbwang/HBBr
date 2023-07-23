#pragma once
//PassManager类，管理所有渲染或者计算Pass
#include "Common.h"
#include "Texture.h"
#include <vector>
#include <map>

class PassBase;
enum class SceneTextureDesc {
	SceneColor = 0,
	DepthStencil = 1,
};

struct SceneTexture
{
	SceneTexture()
	{
		auto sceneColor = Texture::CreateTexture2D(1, 1, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "SceneColor");
		auto sceneDepth = Texture::CreateTexture2D(1, 1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "SceneDepth");
		_sceneTexture.insert(std::make_pair(SceneTextureDesc::SceneColor, sceneColor));
		_sceneTexture.insert(std::make_pair(SceneTextureDesc::DepthStencil, sceneDepth));
	}
	~SceneTexture() 
	{
		_sceneTexture.clear();
	}
	std::map<SceneTextureDesc, std::shared_ptr<Texture>> _sceneTexture;
};

class PassManager
{
public:
	void PassesInit();
	void PassesUpdate();
	void PassesRelease();
private:
	std::map<HString, std::shared_ptr<PassBase>> _passes;
	std::shared_ptr < SceneTexture> _sceneTextures;
};
