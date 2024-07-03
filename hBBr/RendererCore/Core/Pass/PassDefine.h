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

	void SetupBasePassAndDraw(Pass p, class DescriptorSet* pass , class DescriptorSet* obj , class DescriptorSet* mat, class Buffer* vb , class Buffer* ib);
	//Contain uniform buffers {pass,obj,mat} and textures
	VkPipelineLayout _pipelineLayout_p_o_m_t = VK_NULL_HANDLE;
	//Contain uniform buffers {pass,obj,mat}
	VkPipelineLayout _pipelineLayout_p_o_m = VK_NULL_HANDLE;
	//Contain uniform buffers {pass,obj}
	VkPipelineLayout _pipelineLayout_p_o = VK_NULL_HANDLE;
	//Contain uniform buffers {pass,obj} and textures
	VkPipelineLayout _pipelineLayout_p_o_t = VK_NULL_HANDLE;

	std::vector<PipelineObject*> pipelineTemps;

	std::shared_ptr<class DescriptorSet> _opaque_descriptorSet_pass;
	std::shared_ptr<class DescriptorSet> _opaque_descriptorSet_obj;
	std::shared_ptr<class DescriptorSet> _opaque_descriptorSet_mat;
	std::map<class MaterialPrimitive*,std::vector<TextureDescriptorSet>> _descriptorSet_tex;
	std::shared_ptr<class Buffer>_opaque_vertexBuffer;
	std::shared_ptr<class Buffer>_opaque_indexBuffer;
};

class PreCommandPass :public CommandPass
{
public:
	PreCommandPass(class PassManager* manager) :CommandPass(manager) {} 
	virtual ~PreCommandPass();
	virtual void PassInit()override;
	virtual void PassUpdate()override;
};
