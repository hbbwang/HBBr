#pragma once
#include <memory>
#include <vector>
#include <map>
#include "PassBase.h"
#include "HGuid.h"
#include "HRect.h"

struct HDRI2CubeVertexData
{
	glm::vec3 Pos;
	glm::vec3 Normal;
};

struct HDRI2CubeUniformBuffer
{
	glm::mat4 Model;
	glm::mat4 ViewPro;
	glm::vec3 CamPos;
	float XDegree;
	float ZDegree;
};

class HDRI2Cube :public GraphicsPass
{
public:
	HDRI2Cube(HString imagePath);
	virtual ~HDRI2Cube();
	virtual void PassInit()override;
	void PassExecute();
	uint32_t _cubeMapFaceSize = 1024;
	HDRI2CubeUniformBuffer _uniformBuffer[6];
private:
	PipelineIndex CreatePipeline();
	PipelineIndex _pipelineIndex;
	std::shared_ptr<class Buffer>_vertexBuffer;
	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout _ubDescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout _texDescriptorSetLayout = VK_NULL_HANDLE;
	std::shared_ptr<class DescriptorSet> ub_descriptorSet[6];
	std::shared_ptr<class DescriptorSet> tex_descriptorSet[6];

	std::shared_ptr<class Texture2D>_outputFace[6];
	std::shared_ptr<Texture2D> _hdriTexture;
};
