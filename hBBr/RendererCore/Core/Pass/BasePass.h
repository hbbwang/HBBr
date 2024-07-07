#pragma once
#include <memory>
#include "PassBase.h"

/* Opaque pass define */
class BasePass :public GraphicsPass
{
public:

	BasePass(class PassManager* manager) :GraphicsPass(manager) {}
	virtual ~BasePass();

	virtual void PassInit()override;
	virtual void PassUpdate()override;
	virtual void PassReset()override;
private:

	void SetupBasePassAndDraw(Pass p);
	//Contain uniform buffers {pass,obj,vsmat,tex} and textures
	VkPipelineLayout _pipelineLayout_p_o_vsm_t = VK_NULL_HANDLE;
	//Contain uniform buffers {pass,obj,psmat,tex} and textures
	VkPipelineLayout _pipelineLayout_p_o_psm_t = VK_NULL_HANDLE;
	//Contain uniform buffers {pass,obj,vspsmat,tex} and textures
	VkPipelineLayout _pipelineLayout_p_o_vspsm_t = VK_NULL_HANDLE;
	//Contain uniform buffers {pass,obj,vsmat}
	VkPipelineLayout _pipelineLayout_p_o_vsm = VK_NULL_HANDLE;
	//Contain uniform buffers {pass,obj,psmat}
	VkPipelineLayout _pipelineLayout_p_o_psm = VK_NULL_HANDLE;
	//Contain uniform buffers {pass,obj,vspsmat}
	VkPipelineLayout _pipelineLayout_p_o_vspsm = VK_NULL_HANDLE;
	//Contain uniform buffers {pass,obj}
	VkPipelineLayout _pipelineLayout_p_o = VK_NULL_HANDLE;
	//Contain uniform buffers {pass,obj,tex} and textures
	VkPipelineLayout _pipelineLayout_p_o_t = VK_NULL_HANDLE;

	std::vector<PipelineObject*> pipelineTemps;

	std::shared_ptr<class DescriptorSet> _opaque_descriptorSet_pass;
	std::shared_ptr<class DescriptorSet> _opaque_descriptorSet_obj;
	std::shared_ptr<class DescriptorSet> _opaque_descriptorSet_mat_vs;
	std::shared_ptr<class DescriptorSet> _opaque_descriptorSet_mat_ps;
	std::map<class MaterialPrimitive*,std::vector<TextureDescriptorSet>> _descriptorSet_tex;
	std::shared_ptr<class Buffer>_opaque_vertexBuffer;
	std::shared_ptr<class Buffer>_opaque_indexBuffer;
};