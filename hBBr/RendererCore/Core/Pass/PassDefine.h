#pragma once
#include <memory>
#include "PassBase.h"

/* Opaque pass define */
class OpaquePass :public GraphicsPass
{
public:
	struct PassUniformBuffer
	{
		glm::mat4 View;
		glm::mat4 View_Inv;
		glm::mat4 Projection;
		glm::mat4 Projection_Inv;
		glm::mat4 ViewProj;
		glm::mat4 ViewProj_Inv;
		glm::vec4 ScreenInfo; // screen xy,z near,w zfar
		glm::vec4 CameraPos_GameTime;
		glm::vec4 CameraDirection;
		glm::mat4 WorldMatrix;
	};
	struct ObjectUniformBuffer
	{
		glm::mat4 WorldMatrix;
	};

	OpaquePass(VulkanRenderer* renderer) :GraphicsPass(renderer) {}
	~OpaquePass();

	virtual void PassInit()override;
	virtual void PassBuild()override;
	virtual void PassUpdate()override;
private:
	std::shared_ptr<class DescriptorSet> _descriptorSet_pass;
	std::shared_ptr<class DescriptorSet> _descriptorSet_obj;
	std::shared_ptr<class Buffer>_vertexBuffer;
	std::shared_ptr<class Buffer>_indexBuffer;
	PassUniformBuffer _passUniformBuffer;
};

/* Imgui pass define */
class ImguiPass :public GraphicsPass
{
public:
	ImguiPass(VulkanRenderer* renderer) :GraphicsPass(renderer) {}
	~ImguiPass();
	virtual void PassInit()override;
	virtual void PassBuild()override;
	virtual void PassUpdate()override;
private:
	void ShowPerformance();
};
