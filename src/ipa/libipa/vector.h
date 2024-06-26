/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Paul Elder <paul.elder@ideasonboard.com>
 *
 * Vector and related operations
 */
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>

#include <libcamera/base/log.h>
#include <libcamera/base/span.h>

#include "libcamera/internal/yaml_parser.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(Vector)

namespace ipa {

#ifndef __DOXYGEN__
template<typename T, unsigned int Rows,
	 std::enable_if_t<std::is_arithmetic_v<T>> * = nullptr>
#else
template<typename T, unsigned int Rows>
#endif /* __DOXYGEN__ */
class Vector
{
public:
	constexpr Vector() = default;

	constexpr Vector(const std::array<T, Rows> &data)
	{
		for (unsigned int i = 0; i < Rows; i++)
			data_[i] = data[i];
	}

	int readYaml(const libcamera::YamlObject &yaml)
	{
		if (yaml.size() != Rows) {
			LOG(Vector, Error)
				<< "Wrong number of values in vector: expected "
				<< Rows << ", got " << yaml.size();
			return -EINVAL;
		}

		unsigned int i = 0;
		for (const auto &x : yaml.asList()) {
			auto value = x.get<T>();
			if (!value) {
				LOG(Vector, Error) << "Failed to read vector value";
				return -EINVAL;
			}

			data_[i++] = *value;
		}

		return 0;
	}

	const T &operator[](size_t i) const
	{
		ASSERT(i < data_.size());
		return data_[i];
	}

	T &operator[](size_t i)
	{
		ASSERT(i < data_.size());
		return data_[i];
	}

#ifndef __DOXYGEN__
	template<bool Dependent = false, typename = std::enable_if_t<Dependent || Rows >= 1>>
#endif /* __DOXYGEN__ */
	constexpr T x() const
	{
		return data_[0];
	}

#ifndef __DOXYGEN__
	template<bool Dependent = false, typename = std::enable_if_t<Dependent || Rows >= 2>>
#endif /* __DOXYGEN__ */
	constexpr T y() const
	{
		return data_[1];
	}

#ifndef __DOXYGEN__
	template<bool Dependent = false, typename = std::enable_if_t<Dependent || Rows >= 3>>
#endif /* __DOXYGEN__ */
	constexpr T z() const
	{
		return data_[2];
	}

	constexpr Vector<T, Rows> operator-() const
	{
		Vector<T, Rows> ret;
		for (unsigned int i = 0; i < Rows; i++)
			ret[i] = -data_[i];
		return ret;
	}

	constexpr Vector<T, Rows> operator-(const Vector<T, Rows> &other) const
	{
		Vector<T, Rows> ret;
		for (unsigned int i = 0; i < Rows; i++)
			ret[i] = data_[i] - other[i];
		return ret;
	}

	constexpr Vector<T, Rows> operator+(const Vector<T, Rows> &other) const
	{
		Vector<T, Rows> ret;
		for (unsigned int i = 0; i < Rows; i++)
			ret[i] = data_[i] + other[i];
		return ret;
	}

	constexpr T operator*(const Vector<T, Rows> &other) const
	{
		T ret = 0;
		for (unsigned int i = 0; i < Rows; i++)
			ret += data_[i] * other[i];
		return ret;
	}

	constexpr Vector<T, Rows> operator*(T factor) const
	{
		Vector<T, Rows> ret;
		for (unsigned int i = 0; i < Rows; i++)
			ret[i] = data_[i] * factor;
		return ret;
	}

	constexpr Vector<T, Rows> operator/(T factor) const
	{
		Vector<T, Rows> ret;
		for (unsigned int i = 0; i < Rows; i++)
			ret[i] = data_[i] / factor;
		return ret;
	}

	constexpr double length2() const
	{
		double ret = 0;
		for (unsigned int i = 0; i < Rows; i++)
			ret += data_[i] * data_[i];
		return ret;
	}

	constexpr double length() const
	{
		return std::sqrt(length2());
	}

private:
	std::array<T, Rows> data_;
};

template<typename T, unsigned int Rows>
bool operator==(const Vector<T, Rows> &lhs, const Vector<T, Rows> &rhs)
{
	for (unsigned int i = 0; i < Rows; i++) {
		if (lhs[i] != rhs[i])
			return false;
	}

	return true;
}

template<typename T, unsigned int Rows>
bool operator!=(const Vector<T, Rows> &lhs, const Vector<T, Rows> &rhs)
{
	return !(lhs == rhs);
}

} /* namespace ipa */

#ifndef __DOXYGEN__
template<typename T, unsigned int Rows>
std::ostream &operator<<(std::ostream &out, const ipa::Vector<T, Rows> &v)
{
	out << "Vector { ";
	for (unsigned int i = 0; i < Rows; i++) {
		out << v[i];
		out << ((i + 1 < Rows) ? ", " : " ");
	}
	out << " }";

	return out;
}
#endif /* __DOXYGEN__ */

} /* namespace libcamera */
