
#include "core/SourceCommon.h"

#include "system/ThreadPool.h"



namespace vk2d {

namespace vk2d_internal {



void ThreadPoolWorkerThread(
	ThreadSharedResource		*	thread_shared_resource,
	ThreadPrivateResource		*	thread_private_resource,
	ThreadSignal				*	thread_signals
)
{
	auto success = thread_private_resource->ThreadBegin();
	if( !success ) {
		thread_signals->init_error		= true;
		thread_private_resource->ThreadEnd();
		thread_signals->ready_to_join	= true;
		return;
	} else {
		thread_signals->init_success	= true;
	}

	while( !thread_shared_resource->threads_should_exit ) {
		bool found_work		= false;
		if( auto task		= thread_shared_resource->FindWork( thread_private_resource ) ) {
			// There's more work to be done, let the other threads know about it too.
			thread_shared_resource->thread_wakeup.notify_one();
			( *task )( thread_private_resource );
			thread_shared_resource->TaskComplete( task );
			found_work		= true;
		}

		// If we found work previously there might be more waiting,
		// don't wait until we run out of things to do.
		if( !found_work ) {
			std::unique_lock<std::mutex> unique_lock( thread_shared_resource->thread_wakeup_mutex );
			thread_shared_resource->thread_wakeup.wait_for( unique_lock, std::chrono::milliseconds( 10 ) );	// periodically wake up and check for work
		}
	}

	thread_private_resource->ThreadEnd();
	thread_signals->ready_to_join		= true;
}

} // vk2d_internal

} // vk2d



vk2d::vk2d_internal::Task * vk2d::vk2d_internal::ThreadSharedResource::FindWork(
	ThreadPrivateResource 	*	thread_private_resource
)
{
	std::lock_guard<std::mutex> lock_guard( task_list_mutex );

	auto it = task_list.begin();
	while( it != task_list.end() ) {
		auto task = it->get();

		// If task is already running, skip it
		if( !task->IsRunning() ) {

			// Check if this thread is allowed to run this code
			if( !task->IsThreadLocked() ||
				std::any_of( task->GetThreadLocks().begin(), task->GetThreadLocks().end(),
					[ thread_private_resource, &task ]( uint32_t tl )
					{
						return thread_private_resource->GetThreadIndex() == tl;
					} ) ) {

				// Look for dependencies, if task is depending on another
				// task that isn't yet finished we should not execute it.
				// Finished tasks are removed from task_list.
				const auto & dependencies = task->GetDependencies();
				if( std::any_of( task_list.begin(), task_list.end(), [ &dependencies ](
					std::unique_ptr<Task> & t )
					{
						return std::any_of( dependencies.begin(), dependencies.end(), [ &t ]( uint64_t d )
							{
								return t->GetTaskIndex() == d;
							} );
					} ) ) {
					// Task is depending on some other previous task in the queue
					++it;
				} else {
						// Task not depending on any other active task and is not already running, run it
						task->is_running		= true;
						return task;
					}
			} else {
						// This thread is not allowed to run this code
						++it;
					}
		} else {
			// Task already running, move on to check the next one
			++it;
		}
	}
	// No work available
	return nullptr;
}

void vk2d::vk2d_internal::ThreadSharedResource::TaskComplete(
	Task	*	task
)
{
	std::lock_guard<std::mutex> lock_guard( task_list_mutex );
	auto it = task_list.begin();
	while( it != task_list.end() ) {
		if( it->get() == task ) {
			task_list.erase( it );
			return;
		}
		++it;
	}
}

bool vk2d::vk2d_internal::ThreadSharedResource::IsTaskListEmpty()
{
	std::lock_guard<std::mutex> lock_guard( task_list_mutex );
	return !task_list.size();
}

void vk2d::vk2d_internal::ThreadSharedResource::AddTask(
	std::unique_ptr<Task> new_task )
{
	std::lock_guard<std::mutex> lock_guard( task_list_mutex );
	task_list.emplace_back( std::move( new_task ) );
}



vk2d::vk2d_internal::ThreadPool::ThreadPool(
	std::vector<std::unique_ptr<ThreadPrivateResource>>	&&	thread_resources
)
{
	thread_signals.resize( thread_resources.size() );
	threads.reserve( thread_resources.size() );
	thread_shared_resource		= std::make_unique<ThreadSharedResource>();
	thread_private_resources	= std::move( thread_resources );
	for( size_t i = 0; i < thread_private_resources.size(); ++i ) {
		thread_private_resources[ i ]->thread_index		= uint32_t( i );
		threads.push_back( std::thread( ThreadPoolWorkerThread, thread_shared_resource.get(), thread_private_resources[ i ].get(), &thread_signals[ i ] ) );
	};
	for( size_t i = 0; i < threads.size(); ++i ) {
		auto & signal = thread_signals[ i ];
		while( !( signal.init_success || signal.init_error ) ) {
			std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
		};
		if( signal.init_error ) return;
	}

	is_good						= true;
}



vk2d::vk2d_internal::ThreadPool::~ThreadPool()
{
	shutting_down	= true;

	// Wait for all the work to be done.
	WaitIdle();

	// Signal all threads to exit.
	thread_shared_resource->threads_should_exit	= true;

	// Wait for all threads to be ready for joining.
	while( !std::all_of( thread_signals.begin(), thread_signals.end(), []( ThreadSignal & signal )
		{
			return signal.ready_to_join.load();
		} ) ) {

		// Keep signaling all threads until they're ready to join.
		thread_shared_resource->thread_wakeup.notify_all();
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	}

	// All threads ready to join
	for( auto & t : threads ) {
		t.join();
	}
}

std::thread::id vk2d::vk2d_internal::ThreadPool::GetThreadID( uint32_t thread_index ) const
{
	return threads[ thread_index ].get_id();
}

bool vk2d::vk2d_internal::ThreadPool::IsGood() const
{
	return is_good;
}

void vk2d::vk2d_internal::ThreadPool::WaitIdle()
{
	while( !thread_shared_resource->IsTaskListEmpty() ) {
		thread_shared_resource->thread_wakeup.notify_all();
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	}
	assert( thread_shared_resource->IsTaskListEmpty() );
}

std::atomic_uint64_t task_index_counter		= 0;

uint64_t vk2d::vk2d_internal::ThreadPool::AddTask(
	std::unique_ptr<Task> new_task
)
{
	if( !is_good ) return false;

	auto index	= new_task->task_index	= ++task_index_counter;
	thread_shared_resource->AddTask( std::move( new_task ) );
	thread_shared_resource->thread_wakeup.notify_one();

	return index;
}
