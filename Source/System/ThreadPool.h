#pragma once

#include "../Core/SourceCommon.h"



namespace vk2d {

namespace _internal {



class ThreadPool;
class ThreadPrivateResource;
class Task;
struct ThreadSignal;


// Make sure all accesses are either atomic or inside a critical sector.
// This is the main method of inter-thread communication.
class ThreadSharedResource {
public:

	vk2d::_internal::Task * FindWork(
		vk2d::_internal::ThreadPrivateResource	*	thread_private_resource );

	void TaskComplete(
		vk2d::_internal::Task					*	task );

	bool IsTaskListEmpty();

	void AddTask(
		std::unique_ptr<vk2d::_internal::Task> new_task );

	std::mutex											thread_wakeup_mutex;
	std::condition_variable								thread_wakeup;

	std::atomic_bool									threads_should_exit;

	std::mutex											task_list_mutex;
	std::deque<std::unique_ptr<vk2d::_internal::Task>>	task_list;
};


class Task {
	friend class vk2d::_internal::ThreadPool;
	friend class vk2d::_internal::ThreadSharedResource;

public:
	Task()
	{};
	Task( const Task & other )						= delete;
	Task( Task && other )							= default;

	virtual											~Task()
	{};

	inline const std::vector<uint32_t>			&	GetThreadLocks() const
	{
		return locked_to_threads;
	}

	inline bool										IsThreadLocked() const
	{
		return !!locked_to_threads.size();
	}

	inline uint64_t									GetTaskIndex() const
	{
		return task_index;
	}

	inline const std::vector<uint64_t>			&	GetDependencies() const
	{
		return dependencies;
	}

	inline bool										IsRunning() const
	{
		return is_running;
	}

	virtual void									operator()(
		vk2d::_internal::ThreadPrivateResource	*	thread_resource )			= 0;

private:
	std::vector<uint32_t>							locked_to_threads			= {};
	uint64_t										task_index					= {};
	std::vector<uint64_t>							dependencies				= {};
	std::atomic_bool								is_running					= {};
};



// This tells a specific thread what to do immediately after the
// thread has been created and what to do before joining the thread.
class ThreadPrivateResource {
	friend class vk2d::_internal::ThreadPool;
	friend void ThreadPoolWorkerThread(
		vk2d::_internal::ThreadSharedResource		*	thread_shared_resource,
		vk2d::_internal::ThreadPrivateResource		*	thread_private_resource,
		vk2d::_internal::ThreadSignal				*	thread_signals
	);

public:
	ThreadPrivateResource()
	{};
	ThreadPrivateResource( const vk2d::_internal::ThreadPrivateResource & other )		= delete;
	ThreadPrivateResource( vk2d::_internal::ThreadPrivateResource && other )			= default;

	virtual					~ThreadPrivateResource()
	{};

	// This is not thread id, just the index
	inline uint32_t			GetThreadIndex()
	{
		return thread_index;
	}

protected:
	// Ran at thread start before anything else.
	// Return true if succesful, false to terminate entire thread pool.
	// ThreadEnd() is ran afterwards regardless so you can rely that for cleanup.
	virtual	bool			ThreadBegin()			= 0;

	// Ran at thread pool termination, this is ran after all tasks have completed.
	// It's also run even if ThreadBegin() returns false,
	// so you can rely on this function for cleanup.
	virtual void			ThreadEnd()				= 0;

private:
	uint32_t				thread_index			= {};
};



// Used to pass signals between specific threads and the main thread
struct ThreadSignal {
	ThreadSignal()												= default;
	ThreadSignal( const vk2d::_internal::ThreadSignal & other )
	{
		init_success	= other.init_success.load();
		init_error		= other.init_error.load();
		ready_to_join	= other.ready_to_join.load();
	}
	ThreadSignal( vk2d::_internal::ThreadSignal && other )		= default;
	~ThreadSignal()												= default;

	std::atomic_bool		init_success						= {};
	std::atomic_bool		init_error							= {};
	std::atomic_bool		ready_to_join						= {};
};



class ThreadPool {
public:
	// thread_resources.size() tells the amount of threads in the pool
	// Main thread only!
	// 'thread_resources' IS CONSUMED!
	ThreadPool(
		std::vector<std::unique_ptr<vk2d::_internal::ThreadPrivateResource>>		&&	thread_resources );

	// MAIN THREAD ONLY!
	virtual												~ThreadPool();

	// Any thread.
	// 'unique_task' IS CONSUMED!
	// Returns task index that can be used for dependencies.
	template<typename T>
	uint64_t											ScheduleTask(
		std::unique_ptr<T>							&&	unique_task,
		const std::vector<uint32_t>					&	locked_to_threads			= {},
		const std::vector<uint64_t>					&	dependencies				= {} )
	{
		static_assert( std::is_base_of<Task, T>::value, "Task must be derived from 'Task' Class!" );

		if( shutting_down ) {
			assert( 0 && "Shouldn't be adding tasks when we're shutting down the thread pool." );
			return UINT64_MAX;
		}
		unique_task->locked_to_threads	= locked_to_threads;
		unique_task->dependencies		= dependencies;
		return AddTask( std::move( unique_task ) );
	}

	// Any thread.
	std::thread::id										GetThreadID(
		uint32_t										thread_index ) const;

	// Any thread.
	bool												IsGood() const;

	// Any thread.
	// Meant for shutdown. Can add up to a millisecond of wasted time.
	void												WaitIdle();

private:
	uint64_t											AddTask(
		std::unique_ptr<vk2d::_internal::Task>			new_task );

	std::unique_ptr<vk2d::_internal::ThreadSharedResource>					thread_shared_resource		= {};
	std::vector<std::unique_ptr<vk2d::_internal::ThreadPrivateResource>>	thread_private_resources	= {};
	std::vector<vk2d::_internal::ThreadSignal>								thread_signals				= {};
	std::vector<std::thread>												threads						= {};

	std::atomic_bool														shutting_down				= {};

	bool																	is_good						= {};
};



} // _internal

} // vk2d
