#pragma once



#include <core/Common.hpp>

#include <containers/Rect2.hpp>
#include <containers/Color.hpp>

#include <vector>

#include <sstream>
#include <ostream>
#include <iomanip>



template<typename T>
std::ostream					&	operator<<(
	std::ostream				&	os,
	const glm::vec<2, T>		&	v
)
{
	return os << "[" << v.x << ", " << v.y << "]";
}

template<typename T>
std::ostream					&	operator<<(
	std::ostream				&	os,
	const glm::vec<3, T>		&	v
)
{
	return os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
}

template<typename T>
std::ostream					&	operator<<(
	std::ostream				&	os,
	const glm::vec<4, T>		&	v
)
{
	return os << "[" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "]";
}

template<typename T>
std::ostream					&	operator<<(
	std::ostream				&	os,
	const vk2d::ColorBase<T>	&	v
)
{
	return os << "[" << v.r << ", " << v.g << ", " << v.b << ", " << v.a << "]";
}



template<typename T>
std::ostream					&	operator<<(
	std::ostream				&	os,
	const vk2d::Rect2Base<T>	&	v
)
{
	return os << "[" << v.top_left << ", " << v.bottom_right << "]";
}



template<typename T>
std::ostream					&	operator<<(
	std::ostream				&	os,
	vk2d::Array<T>				&	v
)
{
	auto vs = std::size( v );
	if( vs ) {
		os << "[";
		for( size_t i = 0; i < vs - 1; ++i ) {
			os << v[ i ] << ", ";
		}
		os << v.Back() << "]";
	} else {
		os << "[]";
	}
	return os;
}

template<typename T>
std::ostream					&	operator<<(
	std::ostream				&	os,
	const std::vector<T>		&	v
	)
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
std::ostream					&	operator<<(
	std::ostream				&	os,
	const glm::mat<2, 2, T>		&	v
)
{
	auto field_lenght = 5;

	auto value_str = [field_lenght]( T value ) -> std::string
	{
		std::stringstream tss;
		tss << value;
		auto str = tss.str().substr( 0, field_lenght );
		if( str.back() == '.' ) str = str.substr( 0, field_lenght - 1 );
		return str;
	};

	std::stringstream ss;
	ss << "[";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 0 ][ 0 ] ) << ",";
	ss << std::setw( field_lenght + 2 ) << value_str( v[ 1 ][ 0 ] ) << " ]\n";

	ss << "[";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 0 ][ 1 ] ) << ",";
	ss << std::setw( field_lenght + 2 ) << value_str( v[ 1 ][ 1 ] ) << " ]\n";

	return os << ss.str();
}

template<typename T>
std::ostream					&	operator<<(
	std::ostream				&	os,
	const glm::mat<3, 3, T>		&	v
)
{
	auto field_lenght = 5;

	auto value_str = [field_lenght]( T value ) -> std::string
	{
		std::stringstream tss;
		tss << value;
		auto str = tss.str().substr( 0, field_lenght );
		if( str.back() == '.' ) str = str.substr( 0, field_lenght - 1 );
		return str;
	};

	std::stringstream ss;
	ss << "[";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 0 ][ 0 ] ) << ",";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 1 ][ 0 ] ) << ",";
	ss << std::setw( field_lenght + 2 ) << value_str( v[ 2 ][ 0 ] ) << " ]\n";

	ss << "[";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 0 ][ 1 ] ) << ",";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 1 ][ 1 ] ) << ",";
	ss << std::setw( field_lenght + 2 ) << value_str( v[ 2 ][ 1 ] ) << " ]\n";

	ss << "[";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 0 ][ 2 ] ) << ",";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 1 ][ 2 ] ) << ",";
	ss << std::setw( field_lenght + 2 ) << value_str( v[ 2 ][ 2 ] ) << " ]";

	return os << ss.str();
}

template<typename T>
std::ostream					&	operator<<(
	std::ostream				&	os,
	const glm::mat<4, 4, T>		&	v
)
{
	auto field_lenght = 5;

	auto value_str = [field_lenght]( T value ) -> std::string
	{
		std::stringstream tss;
		tss << value;
		auto str = tss.str().substr( 0, field_lenght );
		if( str.back() == '.' ) str = str.substr( 0, field_lenght - 1 );
		return str;
	};

	std::stringstream ss;
	ss << "[";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 0 ][ 0 ] ) << ",";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 1 ][ 0 ] ) << ",";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 2 ][ 0 ] ) << ",";
	ss << std::setw( field_lenght + 2 ) << value_str( v[ 3 ][ 0 ] ) << " ]\n";

	ss << "[";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 0 ][ 1 ] ) << ",";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 1 ][ 1 ] ) << ",";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 2 ][ 1 ] ) << ",";
	ss << std::setw( field_lenght + 2 ) << value_str( v[ 3 ][ 1 ] ) << " ]\n";

	ss << "[";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 0 ][ 2 ] ) << ",";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 1 ][ 2 ] ) << ",";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 2 ][ 2 ] ) << ",";
	ss << std::setw( field_lenght + 2 ) << value_str( v[ 3 ][ 2 ] ) << " ]\n";

	ss << "[";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 0 ][ 3 ] ) << ",";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 1 ][ 3 ] ) << ",";
	ss << std::setw( field_lenght + 1 ) << value_str( v[ 2 ][ 3 ] ) << ",";
	ss << std::setw( field_lenght + 2 ) << value_str( v[ 3 ][ 3 ] ) << " ]\n";

	return os << ss.str();
}
