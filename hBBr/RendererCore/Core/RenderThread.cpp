#include "RenderThread.h"
#include "VulkanRenderer.h"

std::mutex RenderThread::_renderThreadMutex;

void RunRenderThread(RenderThread* thread)
{
	while (!thread->IsRelease())
	{
		RenderThreadLockGuard
		for (auto& i : VulkanRenderer::_renderers)
		{
			i.second->Render();
		}
	}
}

RenderThread::RenderThread()
{
	_bRelease = false;
	_thread = std::thread(RunRenderThread,this);
}

RenderThread::~RenderThread()
{
	_bRelease = true;
	//Wait render thread stop.
	_thread.join();
}
