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

	void SetupPassAndDraw(Pass p);

	std::vector<PipelineObject*> _pipelineTemps;

	std::shared_ptr<class DescriptorSet> _opaque_descriptorSet_pass;
	std::shared_ptr<class DescriptorSet> _opaque_descriptorSet_obj;
	std::shared_ptr<class DescriptorSet> _opaque_descriptorSet_mat_vs;
	std::shared_ptr<class DescriptorSet> _opaque_descriptorSet_mat_ps;
	
	std::map<class MaterialPrimitive*,std::vector<TextureDescriptorSet>> _descriptorSet_tex;
	std::shared_ptr<class Buffer>_opaque_vertexBuffer;
	std::shared_ptr<class Buffer>_opaque_indexBuffer;

};