#pragma once

#include "SourceCommon.h"

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <assert.h>

namespace vk2d {

namespace _internal {



class ThreadPool;
class ThreadSharedResource;
class ThreadPrivateResource;
struct ThreadSignal;



class Task {
	friend class ThreadPool;
	friend class ThreadSharedResource;

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
		ThreadPrivateResource					*	thread_resource )			= 0;

private:
	std::vector<uint32_t>							locked_to_threads			= {};
	uint64_t										task_index					= {};
	std::vector<uint64_t>							dependencies				= {};
	std::atomic_bool								is_running					= {};
};



// This tells a specific thread what to do immediately after the
// thread has been created and what to do before joining the thread.
class ThreadPrivateResource {
	friend class ThreadPool;
	friend void ThreadPoolWorkerThread(
		ThreadSharedResource		*	thread_shared_resource,
		ThreadPrivateResource		*	thread_private_resource,
		ThreadSignal				*	thread_signals
	);

public:
	ThreadPrivateResource()
	{};
	ThreadPrivateResource( const ThreadPrivateResource & other )	= delete;
	ThreadPrivateResource( ThreadPrivateResource && other )			= default;

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
	ThreadSignal()									= default;
	ThreadSignal( const ThreadSignal & other )
	{
		init_success	= other.init_success.load();
		init_error		= other.init_error.load();
		ready_to_join	= other.ready_to_join.load();
	}
	ThreadSignal( ThreadSignal && other )			= default;
	~ThreadSignal()									= default;

	std::atomic_bool		init_success			= {};
	std::atomic_bool		init_error				= {};
	std::atomic_bool		ready_to_join			= {};
};



class ThreadPool {
public:
	// thread_resources.size() tells the amount of threads in the pool
	// MAIN THREAD ONLY!
	// PARAMETER IS CONSUMED, VALUES ARE MOVED OUT, DO NOT USE AFTER THIS FUNCTION!
	ThreadPool(
		std::vector<std::unique_ptr<ThreadPrivateResource>>		&&	thread_resources );

	// MAIN THREAD ONLY!
	virtual												~ThreadPool();

	// ANY THREAD!
	// PARAMETER 'task' IS CONSUMED, VALUES ARE MOVED OUT, DO NOT USE AFTER THIS FUNCTION!
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

	// ANY THREAD!
	std::thread::id										GetThreadID(
		uint32_t										thread_index ) const;

	// ANY THREAD!
	bool												IsGood() const;

	// ANY THREAD!
	// Meant for shutdown. Can add up to a millisecond of wasted time, so not for runtime use.
	void												WaitIdle();

private:
	uint64_t											AddTask(
		std::unique_ptr<Task>							new_task );

	std::unique_ptr<ThreadSharedResource>				thread_shared_resource		= {};
	std::vector<std::unique_ptr<ThreadPrivateResource>>	thread_private_resources	= {};
	std::vector<ThreadSignal>							thread_signals				= {};
	std::vector<std::thread>							threads						= {};

	std::atomic_bool									shutting_down				= {};

	bool												is_good						= {};
};



} // _internal

} // vk2d
