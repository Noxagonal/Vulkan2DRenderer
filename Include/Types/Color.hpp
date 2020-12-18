#pragma once

#include "Core/Common.h"

#include <initializer_list>



namespace vk2d {



/// @brief		This is a collection of 4 color channels in order of RGBA.
/// @tparam		T
///				Data type per color channel.
template<typename T>
class ColorBase
{
public:

	T r			= {};
	T g			= {};
	T b			= {};
	T a			= {};

	ColorBase()										= default;
	ColorBase( const vk2d::ColorBase<T> & other )	= default;
	ColorBase( vk2d::ColorBase<T> && other )		= default;
	ColorBase( const std::initializer_list<T> & elements )
	{
		assert( elements.size() <= 4 );
		auto e = elements.begin();
		if( e ) r = *e++;
		if( e ) g = *e++;
		if( e ) b = *e++;
		if( e ) a = *e++;
	}
	ColorBase( T r, T g, T b, T a ) :
		r( r ),
		g( g ),
		b( b ),
		a( a )
	{};

	vk2d::ColorBase<T> & operator=( const vk2d::ColorBase<T> & other )	= default;
	vk2d::ColorBase<T> & operator=( vk2d::ColorBase<T> && other )		= default;

	static constexpr ColorBase<T> WHITE() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ max, max, max, max };
		} else { 
			return ColorBase<T>{ 1.0, 1.0, 1.0, 1.0 };
		}
	};
	
	static constexpr ColorBase<T> BLACK() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ 0, 0, 0, max };
		} else { 
			return ColorBase<T>{ 0.0, 0.0, 0.0, 1.0 };
		}
	};
	
	static constexpr ColorBase<T> GREY() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ max / 2, max / 2, max / 2, max };
		} else { 
			return ColorBase<T>{ 0.5, 0.5, 0.5, 1.0 };
		}
	};
	
	static constexpr ColorBase<T> RED() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ max, 0, 0, max };
		} else { 
			return ColorBase<T>{ 1.0, 0.0, 0.0, 1.0 };
		}
	};
	static constexpr ColorBase<T> SCARLET() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ max, (52 * max) / 255, (28 * max) / 255, max };
		} else { 
			return ColorBase<T>{ 1.0, 0.203, 0.109, 1.0 };
		}
	};
	static constexpr ColorBase<T> PINK() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ max, (105 * max) / 255, (180 * max) / 255, max };
		} else { 
			return ColorBase<T>{ 1.0, 0.411, 0.705, 1.0 };
		}
	};
	static constexpr ColorBase<T> MAGENTA() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ max, 0, max, max };
		} else { 
			return ColorBase<T>{ 1.0, 0.0, 1.0, 1.0 };
		}
	};
	static constexpr ColorBase<T> PURPLE() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ (160 * max) / 255, (32 * max) / 255, (240 * max) / 255, max };
		} else { 
			return ColorBase<T>{ 0.627, 0.125, 0.941, 1.0 };
		}
	};
	static constexpr ColorBase<T> VIOLET() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ (238 * max) / 255, (130 * max) / 255, (238 * max) / 255, max };
		} else { 
			return ColorBase<T>{ 0.933, 0.509, 0.933, 1.0 };
		}
	};
	
	static constexpr ColorBase<T> GREEN() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ 0, max, 0, max };
		} else { 
			return ColorBase<T>{ 0.0, 1.0, 0.0, 1.0 };
		}
	};
	static constexpr ColorBase<T> LIME() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ (50 * max) / 255, (205 * max) / 255, (50 * max) / 255, max };
		} else { 
			return ColorBase<T>{ 0.196, 0.803, 0.196, 1.0 };
		}
	};
	static constexpr ColorBase<T> OLIVE() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ (107 * max) / 255, (142 * max) / 255, (35 * max) / 255, max };
		} else { 
			return ColorBase<T>{ 0.419, 0.556, 0.137, 1.0 };
		}
	};
	
	static constexpr ColorBase<T> BLUE() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ 0, 0, max, max };
		} else { 
			return ColorBase<T>{ 0.0, 0.0, 1.0, 1.0 };
		}
	};
	static constexpr ColorBase<T> NAVY() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ 0, 0, max / 2, max };
		} else { 
			return ColorBase<T>{ 0.0, 0.0, 0.5, 1.0 };
		}
	};
	static constexpr ColorBase<T> CYAN() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ 0, max, max, max };
		} else { 
			return ColorBase<T>{ 0.0, 1.0, 1.0, 1.0 };
		}
	};
	static constexpr ColorBase<T> TEAL() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ 0, max / 2, max / 2, max };
		} else { 
			return ColorBase<T>{ 0.0, 0.5, 0.5, 1.0 };
		}
	};
	
	static constexpr ColorBase<T> YELLOW() {
		if constexpr (std::is_integral_v<T>) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ max, max, 0, max };
		} else { 
			return ColorBase<T>{ 1.0, 1.0, 0.0, 1.0 };
		}
	};
	
};

/// @brief		float per color channel.
using Colorf			= vk2d::ColorBase<float>;

/// @brief		double per color channel.
using Colord			= vk2d::ColorBase<double>;

/// @brief		uint8_t per color channel.
using Color8			= vk2d::ColorBase<uint8_t>;

/// @brief		uint16_t per color channel.
using Color16			= vk2d::ColorBase<uint16_t>;

/// @brief		uint32_t per color channel.
using Color32			= vk2d::ColorBase<uint32_t>;

} // vk2d
