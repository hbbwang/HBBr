#pragma once
#include "Common.h"
#include <memory>
#include <map>

enum class SceneTextureDesc {
	SceneColor = 0,
	SceneDepth = 1,
	FinalColor = 2,
	GBuffer0 = 3,
	GBuffer1 = 4,
	GBuffer2 = 5,
};

class VulkanRenderer;
class PassBase;
class Texture2D;

class SceneTexture
{
public:
	SceneTexture(VulkanRenderer* renderer);
	~SceneTexture()
	{
		_sceneTexture.clear();
	}
	bool UpdateTextures();

	HBBR_API inline std::shared_ptr<Texture2D> GetTexture(SceneTextureDesc desc)
	{
		auto it = _sceneTexture.find(desc);
		if (it != _sceneTexture.end())
		{
			return it->second;
		}
		return nullptr;
	}

private:

	std::map<SceneTextureDesc, std::shared_ptr<Texture2D>> _sceneTexture;

	class VulkanRenderer* _renderer;

};