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

	/// @brief		Add color channels directly by another color and apply to itself. eg. when using
	///				vk2d::Colorf {1.0, 1.0, 0.5, 0.5} + {0.0, 2.0, 0.5, 0.5} will result in
	///				{1.0, 3.0, 1.0, 1.0} color.
	/// @note		Multithreading: Any thread.
	/// @tparam		OtherT
	///				Another type can be used for the other parameter. No extra operations are done to
	///				types when added together, eg. Add 0.5 float to 128 integral value will result in
	///				128.5, which is then rounded down to 128, in this case you'd have to pre-multiply
	///				the float type color yourself.
	/// @param[in]	other
	///				Other color to multiply this with.
	/// @return		Itself with multiplied color value.
	template<typename OtherT>
	vk2d::ColorBase<T> & operator+=( const vk2d::ColorBase<OtherT> & other )
	{
		r += T( other.r );
		g += T( other.g );
		b += T( other.b );
		a += T( other.a );
	}

	/// @brief		Multiply color channels directly by another color and apply to itself. This is
	///				useful when you wish to disable some color channels or intensify them. For example,
	///				when using vk2d::Colorf, {1.0, 0.0, 5.0, 1.0} * {1.0, 5.0, 2.0, 2.0} will result in
	///				{1.0, 0.0, 10.0, 2.0}.
	/// @note		Multithreading: Any thread.
	/// @tparam		OtherT
	///				Another type can be used for the other parameter. Useful if primary color is integer
	///				type and you wish to multiply it with float or double value.
	/// @param[in]	other
	///				Other color to multiply this with.
	/// @return		Itself with multiplied color value.
	template<typename OtherT>
	vk2d::ColorBase<T> & operator*=( const vk2d::ColorBase<OtherT> & other )
	{
		r = T( r * other.r );
		g = T( g * other.g );
		b = T( b * other.b );
		a = T( a * other.a );
	}

	///	@brief		Add color channels directly by another color. eg.when using vk2d::Colorf
	///				{1.0, 1.0, 0.5, 0.5} + {0.0, 2.0, 0.5, 0.5} will result in {1.0, 3.0, 1.0, 1.0}
	///				color.
	/// @note		Multithreading: Any thread.
	/// @tparam		OtherT
	///				Another type can be used for the other parameter. No extra operations are done to
	///				types when added together, eg. Add 0.5 float to 128 integral value will result in
	///				128.5, which is then rounded down to 128, in this case you'd have to pre-multiply
	///				the float type color yourself.
	/// @param[in]	other
	///				Other color to multiply this with.
	/// @return		Itself with multiplied color value.
	template<typename OtherT>
	vk2d::ColorBase<T> operator+( const vk2d::ColorBase<OtherT> & other )
	{
		vk2d::ColorBase<T> ret = *this;
		ret += other;
		return ret;
	}

	/// @brief		Multiply color channels directly by another color. This is useful when you with to
	///				disable some color channels or intensify them. For example, when using vk2d::Colorf,
	///				{1.0, 0.0, 5.0, 1.0} * {1.0, 5.0, 2.0, 2.0} will result in {1.0, 0.0, 10.0, 2.0}.
	/// @note		Multithreading: Any thread.
	/// @tparam		OtherT
	///				Another type can be used for the other parameter. Useful if primary color is integer
	///				type and you wish to multiply it with float or double value.
	/// @param[in]	other
	///				Other color to multiply with.
	/// @return		New multiplied color value.
	template<typename OtherT>
	vk2d::ColorBase<T> operator*( const vk2d::ColorBase<OtherT> & other )
	{
		vk2d::ColorBase<T> ret = *this;
		ret *= other;
		return ret;
	}

	/// @brief		Color blend using alpha of the input color to determind blend amount and return
	///				resulting color.
	///
	///				This works exactly the same way as the output render color blending.
	///				For example blending color: <br>
	///				{0.25, 1.0, 0.5, 0.5} with: <br>
	///				{1.0, 0.25, 0.8, 0.8} will result: <br>
	///				{0.85, 0.4, 0.74, 0.9} <br>
	///				Note that resulting alpha channel gets blended differently from the color channels.
	///				Resulting color is clamped to <tt>0.0 - 1.0</tt> range for float values or
	///				<tt>0 - (integer positive maximum)</tt> for integer types. <br>
	///				Blending works for integer types as well, for example vk2d::Color8 channel value 128
	///				is considered equal to vk2d::Colorf channel value 0.5 by this function. This makes
	///				blending between integer colors easy.
	/// @note		Multithreading: Any thread.
	/// @tparam		OtherT
	///				Another type can be used for the other parameter. Useful when you want to mix integer
	///				colors with a float for example.
	/// @param[in]	other
	///				Second color that this color will get blended with.
	/// @return		New alpha blended color between this and other.
	template<typename OtherT>
	vk2d::ColorBase<T> BlendUsingAlpha( const vk2d::ColorBase<OtherT> & other )
	{
		double this_r;
		double this_g;
		double this_b;
		double this_a;

		double other_r;
		double other_g;
		double other_b;
		double other_a;

		if constexpr( std::is_integral_v<T> ) {
			T t_max = std::numeric_limits<T>::max();
			this_r = r / t_max;
			this_g = g / t_max;
			this_b = b / t_max;
			this_a = a / t_max;
		} else {
			this_r = double( r );
			this_g = double( g );
			this_b = double( b );
			this_a = double( a );
		}

		if constexpr( std::is_integral_v<OtherT> ) {
			OtherT ot_max = std::numeric_limits<OtherT>::max();
			other_r = other.r / ot_max;
			other_g = other.g / ot_max;
			other_b = other.b / ot_max;
			other_a = other.a / ot_max;
		} else {
			other_r = double( other.r );
			other_g = double( other.g );
			other_b = double( other.b );
			other_a = double( other.a );
		}

		double final_r = std::clamp( this_r * ( 1.0 - other_a ) + other_r * other_a,	0.0, 1.0 );
		double final_g = std::clamp( this_g * ( 1.0 - other_a ) + other_g * other_a,	0.0, 1.0 );
		double final_b = std::clamp( this_b * ( 1.0 - other_a ) + other_b * other_a,	0.0, 1.0 );
		double final_a = std::clamp( this_a + other_a * ( 1.0 - this_a ),				0.0, 1.0 );

		if constexpr( std::is_integral_v<T> ) {
			T t_max = std::numeric_limits<T>::max();
			return vk2d::ColorBase<T>( T( final_r * t_max ), T( final_g * t_max ), T( final_b * t_max ), T( final_a * t_max ) );
		} else {
			return vk2d::ColorBase<T>( T( final_r ), T( final_g ), T( final_b ), T( final_a ) );
		}
	}

	/// @brief		Linear color blending on all channels using single value.
	/// 
	/// @note		Multithreading: Any thread.
	/// @tparam		OtherT
	///				Another type can be used for the other parameter. Useful when you want to mix integer
	///				colors with a float for example.
	/// @param[in]	other
	///				Second color that this color will get blended with.
	/// @param[in]	amount
	///				Blending amount, if OtherT is float or double then 0.0 will return the original color
	///				and if this is 1.0 then other color is returned, values inbetween 0.0 and 1.0 return
	///				a color that's linearly interpolated between this and other. <br>
	///				If OtherT is integer type then 0 returns this color and integer maximum positive will
	///				return other, values inbetween 0 and integer-maximum-positive return a color that's
	///				linearly interpolated between this and other. For example if OtherT is uint8_t
	///				(vk2d::Color8) then 128 returns middle color between this and other. uint8_t value 128
	///				is considered to be equal to 0.5 float. uint16_t value 32768 would be considered equal
	///				to 0.5 float. int16_t value 16384 would be considered equal to 0.5 float and so on...
	/// @return     A new linearly interpolated color value between this and other.
	template<typename OtherT>
	vk2d::ColorBase<T> BlendLinear( const vk2d::ColorBase<OtherT> & other, OtherT amount )
	{
		double this_r;
		double this_g;
		double this_b;
		double this_a;

		double other_r;
		double other_g;
		double other_b;
		double other_a;

		double amount_f;

		if constexpr( std::is_integral_v<T> ) {
			T t_max = std::numeric_limits<T>::max();
			this_r = r / t_max;
			this_g = g / t_max;
			this_b = b / t_max;
			this_a = a / t_max;
		} else {
			this_r = double( r );
			this_g = double( g );
			this_b = double( b );
			this_a = double( a );
		}

		if constexpr( std::is_integral_v<OtherT> ) {
			OtherT ot_max = std::numeric_limits<OtherT>::max();
			other_r		= other.r / ot_max;
			other_g		= other.g / ot_max;
			other_b		= other.b / ot_max;
			other_a		= other.a / ot_max;
			amount_f	= amount  / ot_max;
		} else {
			other_r		= double( other.r );
			other_g		= double( other.g );
			other_b		= double( other.b );
			other_a		= double( other.a );
			amount_f	= double( amount );
		}

		double final_r = std::clamp( this_r * ( 1.0 - amount_f ) + other_r * amount_f, 0.0, 1.0 );
		double final_g = std::clamp( this_g * ( 1.0 - amount_f ) + other_g * amount_f, 0.0, 1.0 );
		double final_b = std::clamp( this_b * ( 1.0 - amount_f ) + other_b * amount_f, 0.0, 1.0 );
		double final_a = std::clamp( this_a * ( 1.0 - amount_f ) + other_a * amount_f, 0.0, 1.0 );

		if constexpr( std::is_integral_v<T> ) {
			T t_max = std::numeric_limits<T>::max();
			return vk2d::ColorBase<T>( T( final_r * t_max ), T( final_g * t_max ), T( final_b * t_max ), T( final_a * t_max ) );
		} else {
			return vk2d::ColorBase<T>( T( final_r ), T( final_g ), T( final_b ), T( final_a ) );
		}
	}

	static constexpr ColorBase<T> WHITE()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ max, max, max, max };
		} else {
			return ColorBase<T>{ 1.0, 1.0, 1.0, 1.0 };
		}
	};

	static constexpr ColorBase<T> BLACK()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ 0, 0, 0, max };
		} else {
			return ColorBase<T>{ 0.0, 0.0, 0.0, 1.0 };
		}
	};

	static constexpr ColorBase<T> GREY()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ max / 2, max / 2, max / 2, max };
		} else {
			return ColorBase<T>{ 0.5, 0.5, 0.5, 1.0 };
		}
	};

	static constexpr ColorBase<T> RED()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ max, 0, 0, max };
		} else {
			return ColorBase<T>{ 1.0, 0.0, 0.0, 1.0 };
		}
	};

	static constexpr ColorBase<T> SCARLET()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			// TODO: These potentially overflow, also VS complains: conversion from 'int' to 'T' requires a narrowing conversion.
			return ColorBase<T>{ max, ( 52 * max ) / 255, ( 28 * max ) / 255, max };
		} else {
			return ColorBase<T>{ 1.0, 0.203, 0.109, 1.0 };
		}
	};

	static constexpr ColorBase<T> PINK()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ max, ( 105 * max ) / 255, ( 180 * max ) / 255, max };
		} else {
			return ColorBase<T>{ 1.0, 0.411, 0.705, 1.0 };
		}
	};

	static constexpr ColorBase<T> MAGENTA()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ max, 0, max, max };
		} else {
			return ColorBase<T>{ 1.0, 0.0, 1.0, 1.0 };
		}
	};

	static constexpr ColorBase<T> PURPLE()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ ( 160 * max ) / 255, ( 32 * max ) / 255, ( 240 * max ) / 255, max };
		} else {
			return ColorBase<T>{ 0.627, 0.125, 0.941, 1.0 };
		}
	};

	static constexpr ColorBase<T> VIOLET()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ ( 238 * max ) / 255, ( 130 * max ) / 255, ( 238 * max ) / 255, max };
		} else {
			return ColorBase<T>{ 0.933, 0.509, 0.933, 1.0 };
		}
	};

	static constexpr ColorBase<T> GREEN()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ 0, max, 0, max };
		} else {
			return ColorBase<T>{ 0.0, 1.0, 0.0, 1.0 };
		}
	};

	static constexpr ColorBase<T> LIME()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ ( 50 * max ) / 255, ( 205 * max ) / 255, ( 50 * max ) / 255, max };
		} else {
			return ColorBase<T>{ 0.196, 0.803, 0.196, 1.0 };
		}
	};

	static constexpr ColorBase<T> OLIVE()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ ( 107 * max ) / 255, ( 142 * max ) / 255, ( 35 * max ) / 255, max };
		} else {
			return ColorBase<T>{ 0.419, 0.556, 0.137, 1.0 };
		}
	};

	static constexpr ColorBase<T> BLUE()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ 0, 0, max, max };
		} else {
			return ColorBase<T>{ 0.0, 0.0, 1.0, 1.0 };
		}
	};

	static constexpr ColorBase<T> NAVY()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ 0, 0, max / 2, max };
		} else {
			return ColorBase<T>{ 0.0, 0.0, 0.5, 1.0 };
		}
	};

	static constexpr ColorBase<T> CYAN()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ 0, max, max, max };
		} else {
			return ColorBase<T>{ 0.0, 1.0, 1.0, 1.0 };
		}
	};

	static constexpr ColorBase<T> TEAL()
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{ 0, max / 2, max / 2, max };
		} else {
			return ColorBase<T>{ 0.0, 0.5, 0.5, 1.0 };
		}
	};

	static constexpr ColorBase<T> YELLOW()
	{
		if constexpr( std::is_integral_v<T> ) {
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
