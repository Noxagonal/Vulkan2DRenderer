
#include <Vulkan2DRenderer.h>

constexpr double PI				= 3.14159265358979323846;
constexpr double RAD			= PI * 2.0;



// Debugging only...
#include <../Source/Header/ThreadPool.h>
#include <../Source/VK2D/ThreadPool.cpp>

#include <string>
#include <iostream>


std::mutex iostream_mutex;


int main()
{
	{
		// This is only testing the code, full thread pool system will
		// likely not be visible to the library user.

		// Create thread pool with multiple different types of threads.
		// This is a rather complicated system which allows some tasks
		// to be locked to certain threads, useful when handling vulkan
		// device resources. Tasks can have dependencies to previously
		// submitted tasks, useful when breaking down operations into
		// smaller steps allowing code execution to proceed overall
		// instead of something getting stuck processing something huge.

		std::unique_ptr<vk2d::ThreadPool> thread_pool {};

		// Create a type of thread.
		class thread_resource_test : public vk2d::ThreadPrivateResource {
		public:
			void			ThreadBegin()
			{
				// Gets run when thread is started, useful for
				// Vulkan initializations.
			};
			void			ThreadEnd()
			{
				// Similarly, gets run when thread is about to
				// exit, useful for Vulkan cleanup.
			};

			// thread resources here
			std::string		str		= "Testing...";
		};

		// Create thread pool, you'll need to move a vector of unique_ptrs
		// to the thread pool constructor. The list of thread resources is
		// consumed by the thread pool constructor, this way we can initialize
		// all the resources outside of the thread pool object.
		// All unique_ptrs need to be derived from vk2d::ThreadPrivateResource.
		// You can of course use multiple objects from same thread resource type.
		// 'thread_resources'.size() tells how many threads will be created.
		std::vector<std::unique_ptr<vk2d::ThreadPrivateResource>> thread_resources;
		thread_resources.push_back( std::make_unique<thread_resource_test>() );
		thread_resources.push_back( std::make_unique<thread_resource_test>() );
		thread_resources.push_back( std::make_unique<thread_resource_test>() );
		thread_resources.push_back( std::make_unique<thread_resource_test>() );

		thread_pool = std::make_unique<vk2d::ThreadPool>( std::move( thread_resources ) );
		std::cout << "Thread 0: " << thread_pool->GetThreadID( 0 ) << "\n";
		std::cout << "Thread 1: " << thread_pool->GetThreadID( 1 ) << "\n";
		std::cout << "Thread 2: " << thread_pool->GetThreadID( 2 ) << "\n";
		std::cout << "Thread 3: " << thread_pool->GetThreadID( 3 ) << "\n" << std::endl;

		{
			// Create a new task type, need to be derived from vk2d::Task.
			// You can also lock the task to a certain thread, if you need
			// to ensure that task uses resourses from only a specific thread.
			class MyTask : public vk2d::Task {
			public:
				MyTask( std::chrono::milliseconds delay = std::chrono::milliseconds( 0 ), std::string message = "" ) :
					delay( delay ),
					message( message )
				{};
				void operator()( vk2d::ThreadPrivateResource * thread_resource )
				{
					// Operator() is scheduled to be executed.
					// You'll get a vk2d::ThreadPrivateResource pointer to access thread resources.
					auto tr = dynamic_cast<thread_resource_test*>( thread_resource );
					if( tr ) {
						// Simulate some workload
						std::this_thread::sleep_for( delay );

						std::lock_guard<std::mutex> lock_guard( iostream_mutex );
						std::cout << tr->str << " From thread: " << tr->GetThreadIndex() << ". " << message << std::endl;
					}
				}

				std::chrono::milliseconds	delay;
				std::string					message;
			};

			// Second task to print out that all previous threads have finished execution.
			class MyTask2 : public vk2d::Task {
			public:
				MyTask2( std::string message ) : message( message )
				{};
				void operator()( vk2d::ThreadPrivateResource * thread_resource )
				{
					// We'll set this thread to wait for all other threads.
					std::cout << "All tasks completed: from thread: " << thread_resource->GetThreadIndex() << ". " << message << std::endl;
				}

				std::string message;
			};

			std::vector<uint64_t> dependencies;
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(  100 ), "Should run on thread 0 only." ), { 0 } ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(  200 ), "Should run on thread 0 only." ), { 0 } ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(  300 ), "Should run on thread 0 only." ), { 0 } ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(  400 ), "Should run on thread 0 only." ), { 0 } ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(  100 ), "Should run on thread 1 only." ), { 1 } ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(  200 ), "Should run on thread 1 only." ), { 1 } ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(  300 ), "Should run on thread 1 only." ), { 1 } ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(  400 ), "Should run on thread 1 only." ), { 1 } ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(    0 ), "Should run on any thread." ) ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(    0 ), "Should run on any thread." ) ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(    0 ), "Should run on any thread." ) ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(    0 ), "Should run on any thread." ) ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(    0 ), "Should run only on threads 0 and 1." ), { 0, 1 } ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(    0 ), "Should run only on threads 0 and 1." ), { 0, 1 } ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(    0 ), "Should run only on threads 0 and 1." ), { 0, 1 } ) );
			dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(    0 ), "Should run only on threads 0 and 1." ), { 0, 1 } ) );
			std::vector<uint64_t> chained_dependencies;
			chained_dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(  500 ), "Chained, should run after unchained: Should run on thread 2 or 3." ), { 2, 3 }, dependencies ) );
			chained_dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds( 1000 ), "Chained, should run after unchained: Should run on thread 2 or 3." ), { 2, 3 }, dependencies ) );
			chained_dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds( 1500 ), "Chained, should run after unchained: Should run on thread 2 or 3." ), { 2, 3 }, dependencies ) );
			chained_dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds( 2000 ), "Chained, should run after unchained: Should run on thread 2 or 3." ), { 2, 3 }, dependencies ) );
			chained_dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(    0 ), "Chained, should run after unchained: Should run on thread 2 or 3." ), { 2, 3 }, dependencies ) );
			chained_dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(    0 ), "Chained, should run after unchained: Should run on thread 2 or 3." ), { 2, 3 }, dependencies ) );
			chained_dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(    0 ), "Chained, should run after unchained: Should run on thread 2 or 3." ), { 2, 3 }, dependencies ) );
			chained_dependencies.push_back( thread_pool->ScheduleTask( std::make_unique<MyTask>( std::chrono::milliseconds(    0 ), "Chained, should run after unchained: Should run on thread 2 or 3." ), { 2, 3 }, dependencies ) );
			thread_pool->ScheduleTask( std::make_unique<MyTask2>( "Chained, should run last: Should run on thread 1." ), { 1 }, chained_dependencies );
		}

		// At this point thread pool gets destroyed, so it'll automatically synchronize all threads.
	}








	vk2d::RendererCreateInfo renderer_create_info {};
	auto renderer = vk2d::CreateRenderer( renderer_create_info );
	if( !renderer ) return -1;

	vk2d::WindowCreateInfo window_create_info {};
	window_create_info.width	= 800;
	window_create_info.height	= 600;
	auto window = renderer->CreateWindowOutput( window_create_info );

	size_t frame_counter = 0;
	while( true ) {
		++frame_counter;
		if( !window->BeginRender() ) return -1;

		window->Draw_PieBox(
			false,
			{ -0.95f, -0.5f },
			{ -0.05f, +0.5f },
			frame_counter / 100.0f,
			std::sin( frame_counter / 123.0f ) / 2.0f + 0.5f
		);

		window->Draw_Pie(
			false,
			{ +0.05f, -0.5f },
			{ +0.95f, +0.5f },
			frame_counter / 100.0f,
			std::sin( frame_counter / 123.0f ) / 2.0f + 0.5f
		);

		if( !window->EndRender() ) return -1;
	}

	return 0;
}
