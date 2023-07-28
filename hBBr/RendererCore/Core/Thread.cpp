#include "Thread.h"
#include "VulkanRenderer.h"

ThreadTask* ThreadTask::CreateTask(std::function<void()> task)
{
	return new ThreadTask(task);
}

ThreadTask::ThreadTask(std::function<void()> task)
{
	_thread = std::thread(task);
}
