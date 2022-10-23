#pragma once

#include "core/Common.h"

#include <initializer_list>
#include <algorithm>



namespace vk2d {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		This is a collection of 4 color channels in order of RGBA.
/// 
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

	constexpr ColorBase()									= default;

	template<typename OtherT>
	constexpr ColorBase(
		const vk2d::ColorBase<OtherT> & other
	)
	{
		*this = other;
	}

	//constexpr ColorBase( const std::initializer_list<T> & elements )
	//{
	//	auto s = elements.size();
	//	assert( s <= 4 );
	//	auto e = elements.begin();
	//	r = ( s >= 1 ) ? *e++ : T{};
	//	g = ( s >= 2 ) ? *e++ : T{};
	//	b = ( s >= 3 ) ? *e++ : T{};
	//	a = ( s >= 4 ) ? *e++ : T{};
	//}
	constexpr ColorBase( T r, T g, T b, T a ) :
		r( r ),
		g( g ),
		b( b ),
		a( a )
	{};

	template<typename OtherT>
	constexpr vk2d::ColorBase<T> & operator=( const vk2d::ColorBase<T> & other )
	{
		if constexpr( std::is_same_v<T, OtherT> ) {
			r = other.r;
			g = other.g;
			b = other.b;
			a = other.a;
		} else {
			*this = ConvertTo<T>( other );
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Add color channels directly by another color and apply to itself.
	///
	///				eg. when using vk2d::Colorf {1.0, 1.0, 0.5, 0.5} + {0.0, 2.0, 0.5, 0.5} will result in {1.0, 3.0, 1.0, 1.0}
	///				color.
	/// 
	/// @tparam		OtherT
	///				Another type can be used for the other parameter. No extra operations are done to
	///				types when added together, eg. Add 0.5 float to 128 integral value will result in
	///				128.5, which is then rounded down to 128, in this case you'd have to pre-multiply
	///				the float type color yourself.
	/// @param[in]	other
	///				Other color to multiply this with.
	/// @return		Itself with multiplied color value.
	template<typename OtherT>
	constexpr vk2d::ColorBase<T> & operator+=( const vk2d::ColorBase<OtherT> & other )
	{
		auto other_converted = ConvertTo<T>( other );
		r += T( other_converted.r );
		g += T( other_converted.g );
		b += T( other_converted.b );
		a += T( other_converted.a );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Multiply color channels directly by another color and apply to itself.
	///
	///				This is useful when you wish to disable some color channels or intensify them. For example, when using
	///				vk2d::Colorf, {1.0, 0.0, 5.0, 1.0} * {1.0, 5.0, 2.0, 2.0} will result in {1.0, 0.0, 10.0, 2.0}.
	/// 
	/// @tparam		OtherT
	///				Another type can be used for the other parameter. Other type is converted to this type first.
	/// 
	/// @param[in]	other
	///				Other color to multiply this with.
	/// 
	/// @return		Itself with multiplied color value.
	template<typename OtherT>
	constexpr vk2d::ColorBase<T> & operator*=( const vk2d::ColorBase<OtherT> & other )
	{
		auto other_converted = ConvertTo<T>( other );
		r = T( r * other_converted.r );
		g = T( g * other_converted.g );
		b = T( b * other_converted.b );
		a = T( a * other_converted.a );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///	@brief		Add color channels directly by another color.
	///
	///				For example when using vk2d::Colorf {1.0, 1.0, 0.5, 0.5} + {0.0, 2.0, 0.5, 0.5} will result in
	///				{1.0, 3.0, 1.0, 1.0} color.
	/// 
	/// @tparam		OtherT
	///				Another type can be used for the other parameter. Other type is converted to this type first.
	/// 
	/// @param[in]	other
	///				Other color to multiply this with.
	/// @return		Itself with multiplied color value.
	template<typename OtherT>
	constexpr vk2d::ColorBase<T> operator+( const vk2d::ColorBase<OtherT> & other )
	{
		vk2d::ColorBase<T> ret = *this;
		ret += other;
		return ret;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Multiply color channels directly by another color.
	///
	///				This is useful when you wish to disable some color channels or intensify them. For example, when using
	///				vk2d::Colorf, {1.0, 0.0, 5.0, 1.0} * {1.0, 5.0, 2.0, 2.0} will result in {1.0, 0.0, 10.0, 2.0}.
	/// 
	/// @tparam		OtherT
	///				Another type can be used for the other parameter. Useful if primary color is integer type and you wish to
	///				multiply it with float or double value.
	/// 
	/// @param[in]	other
	///				Other color to multiply with.
	/// 
	/// @return		New multiplied color value.
	template<typename OtherT>
	constexpr vk2d::ColorBase<T> operator*( const vk2d::ColorBase<OtherT> & other )
	{
		vk2d::ColorBase<T> ret = *this;
		ret *= other;
		return ret;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Color blend using alpha of the input color to determind blend amount and return resulting color.
	///
	///				This works exactly the same way as the output render color blending. For example blending color: <br>
	///				{0.25, 1.0, 0.5, 0.5} with: <br>
	///				{1.0, 0.25, 0.8, 0.8} will result: <br>
	///				{0.85, 0.4, 0.74, 0.9} <br>
	///				Note that resulting alpha channel gets blended differently from the color channels. Resulting color is clamped
	///				to <tt>0.0 - 1.0</tt> range for float values or <tt>0 - (integer positive maximum)</tt> for integer types. <br>
	///				Blending works for integer types as well, for example vk2d::Color8 channel value 128 is considered equal to
	///				vk2d::Colorf channel value 0.5 by this function. This makes blending between integer colors easy.
	/// 
	///				Values are clamped if this type is an integral type.
	/// 
	/// @tparam		OtherT
	///				Another type can be used for the other parameter. Useful when you want to mix integer colors with a float for
	///				example.
	/// 
	/// @param[in]	other
	///				Second color that this color will get blended with.
	/// 
	/// @return		New alpha blended color between this and other.
	template<typename OtherT>
	constexpr vk2d::ColorBase<T> BlendUsingAlpha( const vk2d::ColorBase<OtherT> & other )
	{
		auto this_floating = ConvertTo<double>( *this );
		auto other_floating = ConvertTo<double>( other );
		auto final_floating = ColorBase<double>();

		if constexpr( std::is_floating_point_v<T> ) {
			final_floating.r = this_floating.r * ( 1.0 - other_floating.a ) + other_floating.r * other_floating.a;
			final_floating.g = this_floating.g * ( 1.0 - other_floating.a ) + other_floating.g * other_floating.a;
			final_floating.b = this_floating.b * ( 1.0 - other_floating.a ) + other_floating.b * other_floating.a;
			final_floating.a = this_floating.a + other_floating.a * ( 1.0 - this_floating.a );
		} else {
			constexpr auto minimum_clamp = std::is_signed_v<T> ? -1.0 : 0.0;
			constexpr auto maximum_clamp = 1.0;
			final_floating.r = std::clamp( this_floating.r * ( 1.0 - other_floating.a ) + other_floating.r * other_floating.a, minimum_clamp, maximum_clamp );
			final_floating.g = std::clamp( this_floating.g * ( 1.0 - other_floating.a ) + other_floating.g * other_floating.a, minimum_clamp, maximum_clamp );
			final_floating.b = std::clamp( this_floating.b * ( 1.0 - other_floating.a ) + other_floating.b * other_floating.a, minimum_clamp, maximum_clamp );
			final_floating.a = std::clamp( this_floating.a + other_floating.a * ( 1.0 - this_floating.a ), minimum_clamp, maximum_clamp );
		}

		return ConvertTo<T>( final_floating );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief		Linear color blending on all channels using single value.
	///
	///				Values are clamped if this type is an integral type.
	///
	/// @tparam		OtherT
	///				Another type can be used for the other parameter. Useful when you want to mix integer colors with a float for
	///				example.
	/// 
	/// @param[in]	other
	///				Second color that this color will get blended with.
	/// 
	/// @param[in]	amount
	///				Blending amount, if OtherT is float or double then 0.0 will return the original color and if this is 1.0 then
	///				other color is returned, values inbetween 0.0 and 1.0 return a color that's linearly interpolated between this
	///				and other. <br>
	/// 
	/// @return		A new linearly interpolated color value between this and other.
	template<typename OtherT>
	constexpr vk2d::ColorBase<T> BlendLinear( const vk2d::ColorBase<OtherT> & other, double amount )
	{
		auto this_floating = ConvertTo<double>( *this );
		auto other_floating = ConvertTo<double>( other );
		auto final_floating = ColorBase<double>();

		if constexpr( std::is_floating_point_v<T> ) {
			final_floating.r = this_floating.r * ( 1.0 - amount ) + other_floating.r * amount;
			final_floating.g = this_floating.g * ( 1.0 - amount ) + other_floating.g * amount;
			final_floating.b = this_floating.b * ( 1.0 - amount ) + other_floating.b * amount;
			final_floating.a = this_floating.a * ( 1.0 - amount ) + other_floating.a * amount;
		} else {
			constexpr auto minimum_clamp = std::is_signed_v<T> ? -1.0 : 0.0;
			constexpr auto maximum_clamp = 1.0;
			final_floating.r = std::clamp( this_floating.r * ( 1.0 - amount ) + other_floating.r * amount, minimum_clamp, maximum_clamp );
			final_floating.g = std::clamp( this_floating.g * ( 1.0 - amount ) + other_floating.g * amount, minimum_clamp, maximum_clamp );
			final_floating.b = std::clamp( this_floating.b * ( 1.0 - amount ) + other_floating.b * amount, minimum_clamp, maximum_clamp );
			final_floating.a = std::clamp( this_floating.a * ( 1.0 - amount ) + other_floating.a * amount, minimum_clamp, maximum_clamp );
		}

		return ConvertTo<T>( final_floating );
	}

	static constexpr ColorBase<T> WHITE()
	{
		return PREDEFINED_COLOR(1.0, 1.0, 1.0);
	};

	static constexpr ColorBase<T> BLACK()
	{
		return PREDEFINED_COLOR(0.0, 0.0, 0.0);
	};

	static constexpr ColorBase<T> GREY()
	{
		return PREDEFINED_COLOR(0.5, 0.5, 0.5);
	};

	static constexpr ColorBase<T> RED()
	{
		return PREDEFINED_COLOR(1.0, 0.0, 0.0);
	};

	static constexpr ColorBase<T> SCARLET()
	{
		return PREDEFINED_COLOR(1.0, 0.203, 0.109);
	};

	static constexpr ColorBase<T> PINK()
	{
		return PREDEFINED_COLOR(1.0, 0.411, 0.705);
	};

	static constexpr ColorBase<T> MAGENTA()
	{
		return PREDEFINED_COLOR(1.0, 0.0, 1.0);
	};

	static constexpr ColorBase<T> PURPLE()
	{
		return PREDEFINED_COLOR(0.627, 0.125, 0.941);
	};

	static constexpr ColorBase<T> VIOLET()
	{
		return PREDEFINED_COLOR(0.933, 0.509, 0.933);
	};

	static constexpr ColorBase<T> GREEN()
	{
		return PREDEFINED_COLOR(0.0, 1.0, 0.0);
	};

	static constexpr ColorBase<T> LIME()
	{
		return PREDEFINED_COLOR(0.196, 0.803, 0.196);
	};

	static constexpr ColorBase<T> OLIVE()
	{
		return PREDEFINED_COLOR(0.419, 0.556, 0.137);
	};

	static constexpr ColorBase<T> BLUE()
	{
		return PREDEFINED_COLOR(0.0, 0.0, 1.0);
	};

	static constexpr ColorBase<T> NAVY()
	{
		return PREDEFINED_COLOR(0.0, 0.0, 0.5);
	};

	static constexpr ColorBase<T> CYAN()
	{
		return PREDEFINED_COLOR(0.0, 1.0, 1.0);
	};

	static constexpr ColorBase<T> TEAL()
	{
		return PREDEFINED_COLOR(0.0, 0.5, 0.5);
	};

	static constexpr ColorBase<T> YELLOW()
	{
		return PREDEFINED_COLOR(1.0, 1.0, 0.0);
	};

private:
	template<
		typename ToT,
		typename FromT
	>
	static constexpr ColorBase<ToT> ConvertTo(
		const ColorBase<FromT> & from
	)
	{
		static_assert( std::is_integral_v<ToT> || std::is_floating_point_v<ToT>, "To type must be either integral or flaoating point type." );
		static_assert( std::is_integral_v<FromT> || std::is_floating_point_v<FromT>, "From type must be either integral or flaoating point type." );

		if constexpr( std::is_same_v<ToT, FromT> ) return from;

		auto ret = ColorBase<ToT>();
		if constexpr( std::is_floating_point_v<ToT> && std::is_integral_v<FromT> ) {
			// From integral to floating point.
			auto from_max = std::numeric_limits<FromT>::max();
			ret.r = ToT( from.r ) / from_max;
			ret.g = ToT( from.g ) / from_max;
			ret.b = ToT( from.b ) / from_max;
			ret.a = ToT( from.a ) / from_max;
		}

		if constexpr( std::is_integral_v<ToT> && std::is_floating_point_v<FromT> ) {
			// From floating point to integral.
			constexpr auto to_max = std::numeric_limits<ToT>::max();
			constexpr auto minimum_clamp = FromT( std::is_signed_v<ToT> ? -1.0 : 0.0 );
			constexpr auto maximum_clamp = FromT( 1.0 );
			ret.r = ToT( std::clamp( from.r, minimum_clamp, maximum_clamp ) * to_max );
			ret.g = ToT( std::clamp( from.g, minimum_clamp, maximum_clamp ) * to_max );
			ret.b = ToT( std::clamp( from.b, minimum_clamp, maximum_clamp ) * to_max );
			ret.a = ToT( std::clamp( from.a, minimum_clamp, maximum_clamp ) * to_max );
		}

		if constexpr( std::is_integral_v<ToT> && std::is_integral_v<FromT> ) {
			// From integral to integral.
			auto as_floating = ConvertTo<double>( from );
			ret = ConvertTo<ToT>( as_floating );
		}

		if constexpr( std::is_floating_point_v<T> && std::is_floating_point_v<FromT> ) {
			// From floating point to floating point.
			ret.r = T( from.r );
			ret.g = T( from.g );
			ret.b = T( from.b );
			ret.a = T( from.a );
		}

		return ret;
	}

	static constexpr ColorBase<T> PREDEFINED_COLOR(double r, double g, double b)
	{
		if constexpr( std::is_integral_v<T> ) {
			T max = std::numeric_limits<T>::max();
			return ColorBase<T>{
				T( r * max ),
				T( g * max ),
				T( b * max ),
				T( 1.0 * max )
			};
		} else {
			return ColorBase<T>{ T( r ), T( g ), T( b ), T( 1.0 ) };
		}
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		float per color channel.
using Colorf			= vk2d::ColorBase<float>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		double per color channel.
using Colord			= vk2d::ColorBase<double>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		uint8_t per color channel.
using Color8			= vk2d::ColorBase<uint8_t>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		uint16_t per color channel.
using Color16			= vk2d::ColorBase<uint16_t>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		uint32_t per color channel.
using Color32			= vk2d::ColorBase<uint32_t>;



} // vk2d
