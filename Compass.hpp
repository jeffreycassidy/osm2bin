/*
 * Compass.hpp
 *
 *  Created on: Jan 14, 2016
 *      Author: jcassidy
 */

#ifndef COMPASS_HPP_
#define COMPASS_HPP_

#include <string>
#include <stdexcept>

enum struct CompassIntercardinal 	: uint8_t { NE=0,SE,SW,NW };

struct CompassCardinal
{
	CompassCardinal() : m_dir(N){}

	explicit CompassCardinal(unsigned i) :
			m_dir(static_cast<Name>(i%4))
	{
		if (i > 4)
			throw std::logic_error("Invalid index given to CompassCardinal");
	}

	enum Name : uint8_t { N=0, E, S, W};
	Name m_dir;


	explicit operator unsigned () 	const { return static_cast<uint8_t>(m_dir); }
	operator Name() 				const { return m_dir; }
};




struct CompassPrincipal
{
	enum Name : uint8_t { N=0, NE, E, SE, S, SW, W, NW };

	/// Conversion from integral types requires explicit intent
	explicit CompassPrincipal(uint8_t u) :
			m_dir(Name(u)){}

	/// Widening conversion from CompassCardinal can always proceed
	CompassPrincipal(CompassCardinal c) :
		m_dir(	Name(	unsigned(c)<<1))
	{
	}

	CompassPrincipal(Name n) : m_dir(n){}

	/// Widening conversion from CompassIntercardinal can always proceed
	CompassPrincipal(CompassIntercardinal c)
	{
		m_dir = Name(	(static_cast<uint8_t>(c)<<1) | 1);
	}

	explicit operator unsigned() const { return static_cast<uint8_t>(m_dir); }

	operator CompassCardinal() const
	{
		unsigned u = unsigned(m_dir);
		if ((u%2) != 0)
			throw std::logic_error("Invalid conversion to cardinal point");
		return CompassCardinal(u>>1);
	}

	operator CompassIntercardinal() const
	{
		unsigned u = unsigned(m_dir);
		if ((u % 2) != 1)
			throw std::logic_error("Invalid conversion to intercardinal point");
		return CompassIntercardinal(u>>1);
	}

	CompassPrincipal operator+(int delta){ return CompassPrincipal((static_cast<uint8_t>(m_dir)+delta+8) % 8); }
	bool operator<(const CompassPrincipal rhs) const { return m_dir < rhs.m_dir; }

	bool operator!=(CompassPrincipal rhs) const { return rhs.m_dir != m_dir; }


	Name m_dir;

	static const std::string s_names[8];

	friend std::ostream& operator<<(std::ostream&,CompassPrincipal);
};




#endif /* COMPASS_HPP_ */
