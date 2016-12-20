// Rounding.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#ifndef round_to_hpp
#define round_to_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_float.hpp>

#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

// Round a floating-point number to a given number of decimal places,
// following a given rounding style.

// Power-of-ten scaling factors are best represented in the maximum
// available precision, which is indicated by type 'max_prec_real'.
//
// Changing this typedef lets you use a nonstandard type or class with
// greater precision if desired.
//
// Alternatively, suppose your hardware offers an extended format,
// but you can't or don't take advantage of it--either your compiler
// uses the same representation for double and long double, or you
// set the hardware not to do calculations in extended precision.
// If the compiler nonetheless treats double and long double as
// distinct types, then it might generate extra machine code to
// convert between those types. You could prevent that by changing
// this typedef to double.
typedef long double max_prec_real;

// Any modern C++ compiler provides std::rint().
#define LMI_HAVE_RINT

namespace detail
{
#if defined __MINGW32__
// Returns 'r' raised to the 'n'th power. The sgi stl provides a faster
// implementation as an extension (although it does not seem to work
// with negative powers). Because this template function is called only
// by the round_to constructor, efficiency here is not important in the
// contemplated typical case where a round_to object is created once
// and used to round many numbers. Defectively fails to check for
// overflow or undeflow, but the round_to ctor does do that check.
template<typename RealType>
RealType perform_pow(RealType r, int n)
{
    if(0 == n)
        {
        return RealType(1.0);
        }
    if(n < 0)
        {
        // Successive division by 'r' would lose precision at each step
        // when 'r' is exactly representable but its reciprocal is not,
        // and division is much slower than multiplication on some
        // machines, so instead calculate the positive power and take
        // its reciprocal.
        return RealType(1.0) / perform_pow(r, -n);
        }
    else
        {
        RealType z = r;
        while(--n)
            {
            z *= r;
            }
        return z;
        }
}

#else  // !defined __MINGW32__

// Unlike the kludges above, these are defined inline to avoid
// penalizing compliant compilers.

template<typename RealType>
inline RealType perform_pow(RealType r, int n)
{
    return std::pow(r, n);
}

#endif // !defined __MINGW32__
} // namespace detail

inline rounding_style& default_rounding_style()
{
    static rounding_style default_style = r_to_nearest;
    return default_style;
}

namespace detail
{
// Auxiliary rounding functions: one for each supported rounding style.
// These functions avoid changing the hardware rounding mode as long
// as the library functions they call do not change it.

// Perform no rounding at all.
template<typename RealType>
RealType round_not(RealType r)
{
    return r;
}

// Round up.
template<typename RealType>
RealType round_up(RealType r)
{
#if defined LMI_HAVE_RINT
    RealType i_part = std::rint(r);
    if(i_part < r)
        {
        // Suppose the value of 'i_part' is not exactly representable
        // in type 'RealType'. Then '++' doesn't increment it; it adds
        // unity, which doesn't change the value. That's OK though: in
        // that case, this code is unreachable.
        i_part++;
        }
    return i_part;
#else  // !defined LMI_HAVE_RINT
    return std::ceil(r);
#endif // !defined LMI_HAVE_RINT
}

// Round down.
template<typename RealType>
RealType round_down(RealType r)
{
#if defined LMI_HAVE_RINT
    RealType i_part = std::rint(r);
    if(r < i_part)
        {
        i_part--;
        }
    return i_part;
#else  // !defined LMI_HAVE_RINT
    return std::floor(r);
#endif // !defined LMI_HAVE_RINT
}

// Truncate.
template<typename RealType>
RealType round_trunc(RealType r)
{
#if defined LMI_HAVE_RINT
    RealType i_part = std::rint(r);
    RealType f_part = r - i_part;
    // Consider the integer part 'i_part' and the fractional part
    // 'f_part': the integer part is the final answer if
    //   both parts have the same sign (drop the fractional part), or
    //   the fractional part is zero (it doesn't matter), or
    //   the integer part is zero (so ignore the fractional part).
    // If integer and fractional parts have opposite signs
    //   (one positive and one negative) then add or subtract unity to
    //   get the next integer in the direction of zero.
    if(RealType(0) < i_part && f_part < RealType(0))
        {
        i_part--;
        }
    else if(RealType(0) < f_part && i_part < RealType(0))
        {
        i_part++;
        }
    return i_part;
#else  // !defined LMI_HAVE_RINT
    RealType x = std::floor(std::fabs(r));
    return (0.0 <= r) ? x : -x;
#endif // !defined LMI_HAVE_RINT
}

// Round to nearest using bankers method.
template<typename RealType>
RealType round_near(RealType r)
{
#if defined LMI_HAVE_RINT
    RealType i_part = std::rint(r);
#else  // !defined LMI_HAVE_RINT
//  To return immediately with this value:
//    return (RealType(0) < r) ? std::floor(r + 0.5) : std::ceil(r -0.5);
//  would be incorrect, because halfway cases must be rounded to even.
    RealType i_part =
        (RealType(0) < r)
            ? std::floor(r + 0.5)
            : std::ceil (r - 0.5)
            ;
    // This 'i_part' needn't equal the value that std::rint() would
    // return, as long as both produce the same correct result after
    // adjustment below.
#endif // !defined LMI_HAVE_RINT
    RealType f_part = r - i_part;
    RealType abs_f_part = std::fabs(f_part);

    // If      |fractional part| <  .5, ignore it;
    // else if |fractional part| == .5, ignore it if integer part is even;
    // else add sgn(fractional part).
    if
        (
           (RealType(0.5) < abs_f_part)
        || (
              RealType(0.5) == abs_f_part
           && i_part
                  != RealType(2)
                  *  std::floor(RealType(0.5) * i_part)
           )
        )
        {
        if(f_part < RealType(0))
            {
            i_part--;
            }
        else if(RealType(0) < f_part)
            {
            i_part++;
            }
        }
    return i_part;
}

template<typename RealType>
RealType erroneous_rounding_function(RealType)
{
    throw std::logic_error("Erroneous rounding function.");
}
} // namespace detail

template<typename RealType>
class round_to
    :public std::unary_function<RealType, RealType>
{
    BOOST_STATIC_ASSERT(boost::is_float<RealType>::value);

  public:
    round_to();
    round_to(int decimals, rounding_style style);
    round_to(round_to const&);

    round_to& operator=(round_to const&);

    bool operator==(round_to const&) const;
    RealType operator()(RealType r) const;

    int decimals() const;
    rounding_style style() const;

  private:
    typedef RealType (*rounding_function_t)(RealType);
    rounding_function_t select_rounding_function(rounding_style) const;

    int decimals_;
    rounding_style style_;
    max_prec_real scale_fwd_;
    max_prec_real scale_back_;
    rounding_function_t rounding_function_;
};

/// This default ctor serves only to render the class DefaultConstructible.
/// The object it creates throws on use.

template<typename RealType>
round_to<RealType>::round_to()
    :decimals_          (0)
    ,style_             (r_indeterminate)
    ,scale_fwd_         (1.0)
    ,scale_back_        (1.0)
    ,rounding_function_ (detail::erroneous_rounding_function)
{
}

// Naran used const data members, reasoning that a highly optimizing
// compiler could then calculate std::pow(10.0, n) at compile time.
// Not all compilers do this. None available to the author do.
//
// Is this a design flaw? Const data members require initialization in
// the initializer-list, so this test detects a domain error only after
// it has produced the side effect of setting 'errno'. Thus, the strong
// guarantee is lost, and only the basic guarantee is provided.
//
// The guarantee could be strengthened by not throwing at all. That
// would be consistent with other math functions. But it's a shame to
// write new code that forces the user to check 'errno'.
//
// TODO ?? The data members were made non-const after profiling showed
// no penalty on four available compilers (not including Naran's).
// The code should now be reworked to provide the strong guarantee.

// Division by an exact integer value should have slightly better
// accuracy in some cases. But profiling shows that multiplication by
// the reciprocal stored in scale_back_ makes a realistic application
// that performs a lot of rounding run about four percent faster with
// all compilers tested. TODO ?? The best design decision would be
// clearer if we quantified the effect on accuracy.

template<typename RealType>
round_to<RealType>::round_to(int decimals, rounding_style a_style)
    :decimals_          (decimals)
    ,style_             (a_style)
    ,scale_fwd_         (detail::perform_pow(max_prec_real(10.0), decimals))
    ,scale_back_        (max_prec_real(1.0) / scale_fwd_)
    ,rounding_function_ (select_rounding_function(a_style))
{
/*
// TODO ?? This might improve accuracy slightly, but would prevent
// the data members from being const.
    if(0 <= decimals)
        {
        scale_fwd_  = detail::perform_pow(max_prec_real(10.0), decimals);
        scale_back_ = max_prec_real(1.0) / scale_fwd_;
        }
    else
        {
        scale_back_ = detail::perform_pow(max_prec_real(10.0), -decimals);
        scale_fwd_  = max_prec_real(1.0) / scale_back_;
        }
*/

    // This throws only if all use of the function object is invalid.
    // Even if it doesn't throw, there are numbers that it cannot round
    // without overflow, for instance
    //    std::numeric_limits<RealType>::max()
    // rounded to
    //    std::numeric_limits<RealType>::max_exponent10
    // decimals.
    if
        (  decimals < std::numeric_limits<RealType>::min_exponent10
        ||            std::numeric_limits<RealType>::max_exponent10 < decimals
        )
        {
        throw std::domain_error("Invalid number of decimals.");
        }
}

template<typename RealType>
round_to<RealType>::round_to(round_to const& z)
    :decimals_          (z.decimals_         )
    ,style_             (z.style_            )
    ,scale_fwd_         (z.scale_fwd_        )
    ,scale_back_        (z.scale_back_       )
    ,rounding_function_ (z.rounding_function_)
{
}

template<typename RealType>
round_to<RealType>& round_to<RealType>::operator=(round_to const& z)
{
    decimals_          = z.decimals_         ;
    style_             = z.style_            ;
    scale_fwd_         = z.scale_fwd_        ;
    scale_back_        = z.scale_back_       ;
    rounding_function_ = z.rounding_function_;
    return *this;
}

template<typename RealType>
bool round_to<RealType>::operator==(round_to const& z) const
{
    return decimals() == z.decimals() && style() == z.style();
}

// Profiling shows that inlining this member function makes a
// realistic application that performs a lot of rounding run about
// five percent faster with gcc.
template<typename RealType>
inline RealType round_to<RealType>::operator()(RealType r) const
{
    return static_cast<RealType>
        (rounding_function_(static_cast<RealType>(r * scale_fwd_)) * scale_back_
        );
}

template<typename RealType>
int round_to<RealType>::decimals() const
{
    return decimals_;
}

template<typename RealType>
rounding_style round_to<RealType>::style() const
{
    return style_;
}

// Choose the auxiliary rounding function indicated by the argument.
template<typename RealType>
typename round_to<RealType>::rounding_function_t
round_to<RealType>::select_rounding_function(rounding_style const a_style) const
{
#if defined LMI_HAVE_RINT
    if
        (  a_style == default_rounding_style()
        && a_style != r_indeterminate
        )
        {
        return std::rint;
        }
#endif // defined LMI_HAVE_RINT

    switch(a_style)
        {
        case r_toward_zero:
            {
            return detail::round_trunc;
            }
        case r_to_nearest:
            {
            return detail::round_near;
            }
        case r_upward:
            {
            return detail::round_up;
            }
        case r_downward:
            {
            return detail::round_down;
            }
        case r_current:
            {
            return std::rint;
            }
        case r_not_at_all:
            {
            return detail::round_not;
            }
        default:
            {
            throw std::domain_error("Invalid rounding style.");
            }
        }
}

#undef LMI_HAVE_RINT

#endif // round_to_hpp

