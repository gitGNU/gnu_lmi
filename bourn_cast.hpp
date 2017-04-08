// Numeric stinted cast, across whose bourn no value is returned.
//
// Copyright (C) 2017 Gregory W. Chicares.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// http://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#ifndef bourn_cast_hpp
#define bourn_cast_hpp

#include "config.hpp"

#include "rtti_lmi.hpp"                 // lmi::TypeInfo [demangling]

#include <cmath>                        // isinf(), isnan(), signbit()
#include <limits>
#include <sstream>
#include <stdexcept>

/// Numeric stinted cast, across whose bourn no value is returned.
///
/// Perform a static_cast between numeric types, but throw if the
/// value cannot be preserved. For example:
///   bourn_cast<unsigned int>( 1);        // Returns 1U.
///   bourn_cast<unsigned int>(-1);        // Throws.
///   bourn_cast<bool>(INT_MAX);           // Throws: out of range.
///   bourn_cast<float>((double)INFINITY); // Returns infinity.
///   bourn_cast<int>  ((double)INFINITY); // Throws.
///   bourn_cast<unsigned int>(3.0);       // Returns 3U.
///   bourn_cast<unsigned int>(3.14);      // Throws: 3.14 != 3.0U.
///
/// Both From and To must be types for which std::numeric_limits is
/// specialized. Integral-to-floating conversion is highly unlikely
/// to exceed bounds, but may lose precision. Floating-to-integral
/// conversion is extremely unlikely to preserve value, in which case
/// an exception is thrown; but bourn_cast is appropriate for casting
/// an already-rounded integer-valued floating value to another type.
///
/// bourn_cast<>() is intended as a simple and correct replacement for
/// boost::numeric_cast<>(), which does the wrong thing in some cases:
///   http://lists.nongnu.org/archive/html/lmi/2017-03/msg00127.html
///   http://lists.nongnu.org/archive/html/lmi/2017-03/msg00128.html
/// It behaves the same way as boost::numeric_cast<>() except that,
/// instead of quietly truncating, it throws on floating-to-integral
/// conversions that would not preserve value.
///
/// Facilities provided by <limits> are used to the exclusion of
/// <type_traits> functions such as
///   is_arithmetic()
///   is_floating_point()
///   is_integral()
///   is_signed()
///   is_unsigned()
/// so that UDTs with std::numeric_limits specializations can work
/// as expected.

template<typename To, typename From>
#if 201402L < __cplusplus
constexpr
#endif // 201402L < __cplusplus
inline To bourn_cast(From from)
{
    using   to_traits = std::numeric_limits<To  >;
    using from_traits = std::numeric_limits<From>;
    static_assert(  to_traits::is_specialized, "");
    static_assert(from_traits::is_specialized, "");

    static_assert(  to_traits::is_integer ||   to_traits::is_iec559, "");
    static_assert(from_traits::is_integer || from_traits::is_iec559, "");

#if defined __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wsign-compare"
#   if 5 <= __GNUC__
#       pragma GCC diagnostic ignored "-Wbool-compare"
#   endif // 5 <= __GNUC__
#endif // defined __GNUC__

    // Floating to floating.
    //
    // Handle special cases first:
    //  - infinities are interconvertible: no exception wanted;
    //  - C++11 [4.8/1] doesn't require static_cast to DTRT for NaNs;
    // then convert iff within range. Alternatively, a case could be
    // made for converting out-of-range values to infinity, e.g.,
    //   (float)(DBL_MAX) --> INFINITY
    // citing IEEE 754-2008 [5.4.2] "conversion ... to a narrower format
    // ... shall be rounded as specified in Clause 4" and [4.3.1] "an
    // infinitely precise result [exceeding the normalized maximum] shall
    // round to [infinity]", and C99 [F.2.1] "conversions for floating
    // types provide the IEC 60559 conversions between floating-point
    // precisions"; however, C++11 [4.8.1] still says this is undefined
    // behavior, and such a conversion is unlikely to be intentional.
    if(!to_traits::is_integer && !from_traits::is_integer)
        {
        if(std::isnan(from))
            return to_traits::quiet_NaN();
        if(std::isinf(from))
            return
                std::signbit(from)
                ? -to_traits::infinity()
                :  to_traits::infinity()
                ;
        if(from < to_traits::lowest())
            throw std::runtime_error("Cast would transgress lower limit.");
        if(to_traits::max() < from)
            throw std::runtime_error("Cast would transgress upper limit.");
        return static_cast<To>(from);
        }

    // Integral to floating.
    if(!to_traits::is_integer && from_traits::is_integer)
        {
        if(from < to_traits::lowest())
            throw std::runtime_error("Cast would transgress lower limit.");
        if(to_traits::max() < from)
            throw std::runtime_error("Cast would transgress upper limit.");
        return static_cast<To>(from);
        }

    // Floating to integral.
    //
    // Assume integral types have a two's complement representation.
    // Ones' complement might be handled thus [untested]:
    //  - if(from < to_traits::lowest())
    //  + if(from <= From(to_traits::lowest()) - 1)
    if(to_traits::is_integer && !from_traits::is_integer)
        {
        if(std::isnan(from))
            throw std::runtime_error("Cannot cast NaN to integral.");
        if(from < to_traits::lowest())
            throw std::runtime_error("Cast would transgress lower limit.");
        if(From(to_traits::max()) + 1 <= from)
            throw std::runtime_error("Cast would transgress upper limit.");
        To const r = static_cast<To>(from);
        if(r != from)
            {
            lmi::TypeInfo from_type(typeid(From));
            lmi::TypeInfo   to_type(typeid(To  ));
            std::ostringstream oss;
            oss.setf(std::ios_base::fixed, std::ios_base::floatfield);
            oss
                << "Cast from " << from << " [" << from_type << "]"
                << " to "       << r    << " [" << to_type   << "]"
                << " would not preserve value."
                ;
            throw std::runtime_error(oss.str());
            }
        return r;
        }

    // Integral to integral.
    //
    // Converts between integral types that may differ in size and
    // signedness, iff the value is between the maximum and minimum
    // values permitted for the target (To) type. Because of the
    // properties of integers, conversion between integral types
    // either preserves the notional value, or throws.
    //
    // The underlying idea is discussed here:
    //   https://groups.google.com/forum/#!original/comp.std.c++/WHu6gUiwXkU/ZyV_ejRrXFYJ
    // and here:
    //   http://www.two-sdg.demon.co.uk/curbralan/code/numeric_cast/numeric_cast.hpp
    // and embodied in Kevlin Henney's original boost:numeric_cast,
    // distributed under the GPL-compatible Boost Software License.
    if(to_traits::is_integer && from_traits::is_integer)
        {
        if(! to_traits::is_signed && from < 0)
            throw std::runtime_error("Cannot cast negative to unsigned.");
        if(from_traits::is_signed && from < to_traits::lowest())
            throw std::runtime_error("Cast would transgress lower limit.");
        if(to_traits::max() < from)
            throw std::runtime_error("Cast would transgress upper limit.");
        return static_cast<To>(from);
        }

#if defined __GNUC__
#   pragma GCC diagnostic pop
#endif // defined __GNUC__
}

#endif // bourn_cast_hpp

