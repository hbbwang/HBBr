#pragma once

#include <thread>
#include <mutex>

#define RenderThreadLockGuard  std::lock_guard<std::mutex>renderThreadLock(RenderThread::_renderThreadMutex);

class VulkanRenderer;
class RenderThread 
{
public:

	RenderThread();
	~RenderThread();

	inline void Release() { _bRelease = true; }

	inline bool IsRelease()const { return _bRelease; }

	static std::mutex _renderThreadMutex;

private:

	bool _bRelease;

	std::thread _thread;

};
