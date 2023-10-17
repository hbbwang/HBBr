#pragma once
#include <memory>
#include "PassBase.h"

/* Opaque pass define */
class BasePass :public GraphicsPass
{
public:

	BasePass(VulkanRenderer* renderer) :GraphicsPass(renderer) {}
	virtual ~BasePass();

	virtual void PassInit()override;
	virtual void PassUpdate()override;
private:
	void SetupBasePassAndDraw(Pass p, class DescriptorSet* pass , class DescriptorSet* obj , class DescriptorSet* mat, class Buffer* vb , class Buffer* ib);

#pragma region OpaquePass
	std::shared_ptr<class DescriptorSet> _opaque_descriptorSet_pass;
	std::shared_ptr<class DescriptorSet> _opaque_descriptorSet_obj;
	std::shared_ptr<class DescriptorSet> _opaque_descriptorSet_mat;
	std::shared_ptr<class Buffer>_opaque_vertexBuffer;
	std::shared_ptr<class Buffer>_opaque_indexBuffer;
#pragma endregion
};