#pragma once
#include <thread>
#include <mutex>
#include <functional>
class VulkanRenderer;
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
