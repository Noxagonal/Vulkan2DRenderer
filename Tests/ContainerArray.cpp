
#include <Types/Array.hpp>

#include <iostream>
#include <functional>

using namespace std;
using namespace vk2d;



// Don't process throwing functions in debug mode as these also assert.
#if defined( _DEBUG ) || defined( DEBUG )
#define VK2D_TEST_DONT_PROCESS_THROWING
#endif



template<typename T>
std::ostream& operator<<( std::ostream & os, const std::vector<T> & v )
{
	auto vs = std::size( v );
	if( vs ) {
		os << "[";
		for( size_t i = 0; i < vs - 1; ++i ) {
			os << v[ i ] << ", ";
		}
		os << v.back() << "]";
	} else {
		os << "[]";
	}
	return os;
}



template<typename T>
bool Compare( const T & t1, const T & t2 )
{
	if( t1 == t2 ) return true;
	return false;
}

template<typename T>
bool Compare( std::vector<T> & t1, std::vector<T> & t2 )
{
	return std::equal( t1.begin(), t1.end(), t2.begin() );
}

template<typename LambdaT, typename ReturnT>
void Test( LambdaT & lambda, bool should_throw, ReturnT expected_return )
{
	#ifdef VK2D_TEST_DONT_PROCESS_THROWING
	if( should_throw ) return;
	#endif
	try {
		auto ret = lambda();
		if( should_throw ) {
			cout << "Test: Exception was expected but didn't happen.";
			exit( -1 );
		}
		if( !Compare<ReturnT>( ret, expected_return ) ) {
			cout << "Test: Lambda returned " << ret << ". Was expecting: " << expected_return;
			exit( -1 );
		}
	} catch ( const exception & e ) {
		if( !should_throw ) {
			cout << "Test: Unexpected exception: " << e.what();
			exit( -1 );
		}
	} catch (...) {
		if( !should_throw ) {
			cout << "Test: Unexpected unknown exception.";
			exit( -1 );
		}
	}
}



int32_t constructed_counter = 0;

int main()
{
	cout << "Testing vk2d::Array container class.\n\n";

	{
		cout << "Basic type array assignments:\n";

		Test( []()
			{
				using A = Array<uint32_t>;
				A a;
				A b {};
				A c = {};
				A d = A();
				A e = A {};
				return vector<size_t> { size( a ), size( b ), size( c ), size( d ), size( e ) };
			}, false, vector<size_t>{ 0, 0, 0, 0, 0 }
		);
		Test( []()
			{
				using A = Array<uint32_t>;
				A a;
				A b( 1 );
				A c { 5, 10 };
				A d = { 5, 10, 20 };
				A e = A( 4 );
				A f = A { 5, 10, 20, 50, 100 };
				return vector<size_t> { size( a ), size( b ), size( c ), size( d ), size( e ), size( f ) };
			}, false, vector<size_t>{ 0, 1, 2, 3, 4, 5 }
		);

		Test( []()
			{
				Array<uint32_t> a { 5, 10, 20 };
				return vector<uint32_t>{ a.Front(), a.Back() };
			}, false, vector<uint32_t>{ 5, 20 }
		);

		Test( []()
			{
				Array<uint32_t> a;
				a.PushBack( 5 );
				a.PushBack( 10 );
				a.PushBack( 20 );
				return vector<uint32_t>{ a };
			}, false, vector<uint32_t>{ 5, 10, 20 }
		);
		Test( []()
			{
				Array<uint32_t> a;
				a.PushFront( 5 );
				a.PushFront( 10 );
				a.PushFront( 20 );
				return vector<uint32_t>{ a };
			}, false, vector<uint32_t>{ 20, 10, 5 }
		);
		Test( []()
			{
				Array<uint32_t> a;
				a.PushFront( 5 );
				a.PushBack( 10 );
				a.PushFront( 20 );
				a.PushBack( 50 );
				a.PushFront( 100 );
				return vector<uint32_t>{ a };
			}, false, vector<uint32_t>{ 100, 20, 5, 10, 50 }
		);

		Test( []()
			{
				Array<uint32_t> a { 5, 10, 20 };
				Array<uint32_t> b { 50, 100, 200 };
				a += b;
				a += { 500 };
				a += { 1000, 2000 };
				return vector<uint32_t>{ a };
			}, false, vector<uint32_t>{ 5, 10, 20, 50, 100, 200, 500, 1000, 2000 }
		);
	}
	{
		cout << "Array Exceptions:\n";

		Test( []()
			{
				Array<uint32_t> a {};
				return a.Front();
			}, true, 0
		);
		Test( []()
			{
				Array<uint32_t> a { 0 };
				return a.Front();
			}, false, 0
		);
		Test( []()
			{
				Array<uint32_t> a {};
				return a.Back();
			}, true, 0
		);
		Test( []()
			{
				Array<uint32_t> a { 0 };
				return a.Back();
			}, false, 0
		);
	}
	{
		cout << "Array conversions:\n";

		Test( []()
			{
				vector<uint32_t> v1 { 5, 10, 20 };
				Array<uint32_t> a1 { 50 };
				vector<uint32_t> v2;
				a1 = v1;
				v2 = a1;
				return v2;
			}, false, vector<uint32_t>{ 5, 10, 20 }
		);
	}
	{
		struct Simple
		{
			size_t v1;
			uint32_t v2;
			float v3;
			double v4;
		};

		Test( []()
			{
				using A = Array<Simple>;
				A a;
				A b {};
				A c = {};
				A d = Array<Simple>();
				A e = Array<Simple> {};
				return vector<size_t>{ size( a ), size( b ), size( c ), size( d ), size( e ) };
			}, false, vector<size_t>{ 0, 0, 0, 0, 0 }
		);
		Test( []()
			{
				using A = Array<Simple>;
				A a { { 5, 10, 20.0f, 50.0 }, {} };
				return vector<size_t>{ size( a ), size_t( a[ 0 ].v1 ), size_t( a[ 0 ].v2 ), size_t( a[ 0 ].v3 ), size_t( a[ 0 ].v4 ) };
			}, false, vector<size_t>{ 2, 5, 10, 20, 50 }
		);
		Test( []()
			{
				using A = Array<Simple>;
				A a { { 5, 10, 20.0f, 50.0 }, {} };
				A b = a;
				a[ 0 ].v1 = 600;
				return vector<size_t>{ size( b ), size_t( b[ 0 ].v1 ), size_t( b[ 0 ].v2 ), size_t( b[ 0 ].v3 ), size_t( b[ 0 ].v4 ), size_t( a[ 0 ].v1 ), size_t( a[ 0 ].v2 ), size_t( a[ 0 ].v3 ), size_t( a[ 0 ].v4 ) };
			}, false, vector<size_t>{ 2, 5, 10, 20, 50, 600, 10, 20, 50 }
		);
		Test( []()
			{
				using A = Array<Simple>;
				A a { { 5, 10, 20.0f, 50.0 }, {} };
				A b { { 700 } };
				b = a;
				a[ 0 ].v1 = 600;
				return vector<size_t>{ size( b ), size_t( b[ 0 ].v1 ), size_t( b[ 0 ].v2 ), size_t( b[ 0 ].v3 ), size_t( b[ 0 ].v4 ), size_t( a[ 0 ].v1 ), size_t( a[ 0 ].v2 ), size_t( a[ 0 ].v3 ), size_t( a[ 0 ].v4 ) };
			}, false, vector<size_t>{ 2, 5, 10, 20, 50, 600, 10, 20, 50 }
		);
		Test( []()
			{
				using A = Array<Simple>;
				A a { { 5, 10, 20.0f, 50.0 }, {} };
				A b = std::move( a );
				return vector<size_t>{ size( b ), size_t( b[ 0 ].v1 ), size_t( b[ 0 ].v2 ), size_t( b[ 0 ].v3 ), size_t( b[ 0 ].v4 ) };
			}, false, vector<size_t>{ 2, 5, 10, 20, 50 }
		);
	}
	{
		struct MoveableOnly
		{
			MoveableOnly( size_t value ) : v1( value ) {};
			MoveableOnly( const MoveableOnly & other ) = delete;
			MoveableOnly( MoveableOnly && other ) = default;
			MoveableOnly & operator=( const MoveableOnly & other ) = delete;
			MoveableOnly & operator=( MoveableOnly && other ) = default;
			size_t v1 = {};
		};

		cout << "Moveable only objects:\n";

		Test( []()
			{
				using A = Array<MoveableOnly>;
				A a;
				for( size_t i = 0; i < 500; ++i ) {
					a.PushBack( MoveableOnly( i ) );
				}
				return vector<size_t>{ size( a ), a[ 0 ].v1, a[ 1 ].v1, a[ 2 ].v1 };
			}, false, vector<size_t>{ 500, 0, 1, 2 }
		);
		Test( []()
			{
				using A = Array<MoveableOnly>;
				A a;
				a.PushBack( MoveableOnly( 10 ) );
				a.PushBack( MoveableOnly( 11 ) );
				a.PushBack( MoveableOnly( 12 ) );
				// A b = a; // Not allowed.
				A b = std::move( a );
				return vector<size_t>{ size( b ), b[ 0 ].v1, b[ 1 ].v1, b[ 2 ].v1 };
			}, false, vector<size_t>{ 3, 10, 11, 12 }
		);
	}
	{
		struct CopyableOnly
		{
			CopyableOnly( size_t value ) : v1( value ) {};
			CopyableOnly( const CopyableOnly & other ) = default;
			CopyableOnly( CopyableOnly && other ) = delete;
			CopyableOnly & operator=( const CopyableOnly & other ) = default;
			CopyableOnly & operator=( CopyableOnly && other ) = delete;
			size_t v1;
		};

		cout << "Copyable only objects:\n";

		Test( []()
			{
				using A = Array<CopyableOnly>;
				A a;
				for( size_t i = 0; i < 500; ++i ) {
					a.PushBack( CopyableOnly( i ) );
				}
				return vector<size_t>{ size( a ), a[ 0 ].v1, a[ 1 ].v1, a[ 2 ].v1 };
			}, false, vector<size_t>{ 500, 0, 1, 2 }
		);
		Test( []()
			{
				using A = Array<CopyableOnly>;
				A a;
				a.PushBack( CopyableOnly( 10 ) );	// These could use move constructor but use copy constructor instead.
				a.PushBack( CopyableOnly( 11 ) );
				a.PushBack( CopyableOnly( 12 ) );
				A b = a;
				// A b = std::move( a ); // Can be done, however it'll only use copy constructor.
				return vector<size_t>{ size( b ), b[ 0 ].v1, b[ 1 ].v1, b[ 2 ].v1 };
			}, false, vector<size_t>{ 3, 10, 11, 12 }
		);
	}
	{
		cout << "Construction/Destruction testing:\n\n";

		struct CtorDtorCounted
		{
			CtorDtorCounted()
			{
				++constructed_counter;
				cout << " | Construct -> Default\n";
			}
			CtorDtorCounted( const CtorDtorCounted & other )
			{
				++constructed_counter;
				cout << " | Construct -> Copy\n";
			}
			CtorDtorCounted( CtorDtorCounted && other )
			{
				++constructed_counter;
				cout << " | Construct -> Move\n";
			}
			~CtorDtorCounted()
			{
				--constructed_counter;
				cout << " / Destruct\n";
			}
			CtorDtorCounted & operator=( const CtorDtorCounted & other )
			{
				cout << " = Assign    -> Copy\n";
				return *this;
			}
			CtorDtorCounted & operator=( CtorDtorCounted && other )
			{
				cout << " = Assign    -> Move\n";
				return *this;
			}
		};

		Test( []()
			{
				constructed_counter = 0;
				{
					using A = Array<CtorDtorCounted>;
					A a;
				}
				cout << "\n";
				return constructed_counter;
			}, false, 0
		);
		Test( []()
			{
				constructed_counter = 0;
				{
					using A = Array<CtorDtorCounted>;
					A a;
					a.PushBack( CtorDtorCounted() );
					a.PushBack( CtorDtorCounted() );
					a.PushBack( CtorDtorCounted() );
					a.PushBack( CtorDtorCounted() );
				}
				cout << "\n";
				return constructed_counter;
			}, false, 0
		);
		Test( []()
			{
				constructed_counter = 0;
				{
					using A = Array<CtorDtorCounted>;
					A a;
					a.PushFront( CtorDtorCounted() );
					a.PushFront( CtorDtorCounted() );
					a.PushFront( CtorDtorCounted() );
					a.PushFront( CtorDtorCounted() );
				}
				cout << "\n";
				return constructed_counter;
			}, false, 0
		);
		Test( []()
			{
				constructed_counter = 0;
				{
					using A = Array<CtorDtorCounted>;
					A a;
					a.EmplaceBack();
					a.EmplaceBack();
					a.EmplaceBack();
					a.EmplaceBack();
				}
				cout << "\n";
				return constructed_counter;
			}, false, 0
		);
		Test( []()
			{
				constructed_counter = 0;
				{
					using A = Array<CtorDtorCounted>;
					A a;
					a.EmplaceFront();
					a.EmplaceFront();
					a.EmplaceFront();
					a.EmplaceFront();
				}
				cout << "\n";
				return constructed_counter;
			}, false, 0
		);
		Test( []()
			{
				constructed_counter = 0;
				{
					using A = Array<CtorDtorCounted>;
					A a;
					a.EmplaceBack();
					a.EmplaceBack();
					a.EmplaceBack();
					A b = a;
					A c;
					c = b;
				}
				cout << "\n";
				return constructed_counter;
			}, false, 0
		);
		Test( []()
			{
				constructed_counter = 0;
				{
					using A = Array<CtorDtorCounted>;
					A a;
					a.EmplaceBack();
					a.EmplaceBack();
					a.EmplaceBack();
					A b;
					b += { a[ 0 ] };
					b += {};
					b += a;
				}
				cout << "\n";
				return constructed_counter;
			}, false, 0
		);
		Test( []()
			{
				constructed_counter = 0;
				{
					using A = Array<CtorDtorCounted>;
					A a( 5 );
				}
				cout << "\n";
				return constructed_counter;
			}, false, 0
		);
		Test( []()
			{
				constructed_counter = 0;
				{
					using A = Array<CtorDtorCounted>;
					A a { {}, {}, {} };
				}
				cout << "\n";
				return constructed_counter;
			}, false, 0
		);
		Test( []()
			{
				constructed_counter = 0;
				{
					using A = Array<CtorDtorCounted>;
					A a( 10 );
					a.PopBack();
					a.PopBack();
					a.PopFront();
					a.PopFront();
					a.PopFront();
					a.PopBack();
					a.PopFront();
					a.PopBack();
				}
				cout << "\n";
				return constructed_counter;
			}, false, 0
		);
		Test( []()
			{
				constructed_counter = 0;
				{
					using A = Array<CtorDtorCounted>;
					A a( 3 );
					a.PopBack();
					a.PopBack();
					a.PopBack();
				}
				cout << "\n";
				return constructed_counter;
			}, false, 0
		);
		Test( []()
			{
				constructed_counter = 0;
				{
					using A = Array<CtorDtorCounted>;
					A a( 3 );
					a.PopFront();
					a.PopFront();
					a.PopFront();
				}
				cout << "\n";
				return constructed_counter;
			}, false, 0
		);
	}

	cout << "\n";

	return 0;
}
