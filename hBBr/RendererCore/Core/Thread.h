#pragma once

#include <thread>
#include <mutex>
#include <functional>
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

class ThreadTask
{
public:
	static ThreadTask* CreateTask(std::function<void()> task);

	/* Force stop */
	inline void Release()
	{
		delete this;
	}

	/* Wait to stop */
	inline void Wait() 
	{ 
		_thread.join(); 
		delete this;
	}

private:
	ThreadTask(std::function<void()> task);

	std::function<void()> _task;

	std::thread _thread;
};
