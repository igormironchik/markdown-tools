/*
    Copyright (c) 2008-2024 Jan W. Krieger (<jan@jkrieger.de>)

    

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef jkqtpmathtools_H_INCLUDED
#define jkqtpmathtools_H_INCLUDED
#include "jkqtcommon/jkqtcommon_imexport.h"
#include "jkqtcommon/jkqtpstringtools.h"
#include <cmath>
#include <limits>
#include <QPoint>
#include <QPointF>
#include <QLineF>
#include <QRectF>
#include <vector>
#include <QString>
#include <functional>
#include <type_traits>
#include <QHashFunctions>

#ifdef max
#  undef max
#endif
#ifdef min
#  undef min
#endif

/*! \brief \f$ \pi=3.14159... \f$
    \ingroup jkqtptools_math_basic

*/
#ifdef M_PI
#  define JKQTPSTATISTICS_PI M_PI
#else
#  define JKQTPSTATISTICS_PI 3.14159265358979323846
#endif


/*! \brief \f$ \sqrt{2\pi}=2.50662827463 \f$
    \ingroup jkqtptools_math_basic

*/
#define JKQTPSTATISTICS_SQRT_2PI 2.50662827463


/*! \brief \f$ \mbox{ln}(10)=2.30258509299404568402... \f$
    \ingroup jkqtptools_math_basic

*/
#ifdef M_LN10
#  define JKQTPSTATISTICS_LN10 M_LN10
#else
#  define JKQTPSTATISTICS_LN10 2.30258509299404568402
#endif


/** \brief double-value NotANumber
 * \ingroup jkqtptools_math_basic
 */
#define JKQTP_DOUBLE_NAN (std::numeric_limits<double>::signaling_NaN())

/** \brief float-value NotANumber
 * \ingroup jkqtptools_math_basic
 */
#define JKQTP_FLOAT_NAN (std::numeric_limits<float>::signaling_NaN())

/** \brief double-value NotANumber
 * \ingroup jkqtptools_math_basic
 */
#define JKQTP_NAN JKQTP_DOUBLE_NAN

/** \brief double-value epsilon
 * \ingroup jkqtptools_math_basic
 */
#define JKQTP_DOUBLE_EPSILON (std::numeric_limits<double>::epsilon())

/** \brief float-value epsilon
 * \ingroup jkqtptools_math_basic
 */
#define JKQTP_FLOAT_EPSILON (std::numeric_limits<float>::epsilon())

/** \brief double-value NotANumber
 * \ingroup jkqtptools_math_basic
 */
#define JKQTP_EPSILON JKQTP_DOUBLE_EPSILON

/** \brief converts a boolean to a double, is used to convert boolean to double by JKQTPDatastore
 *  \ingroup jkqtptools_math_basic
 *
 * This function uses static_cast<double>() by default, but certain specializations (e.g. for bool) are
 * readily available.
 *
 * \callergraph
 */
template<typename T>
inline constexpr double jkqtp_todouble(const T& d) {
    return static_cast<double>(d);
}


/** \brief converts a boolean to a double, is used to convert boolean to double by JKQTPDatastore
 *  \ingroup jkqtptools_math_basic
 *
 * Specialisation of the generic template jkqtp_todouble() with (true -> 1.0, false -> 0.0)
 *
 * \callergraph
 */
template<>
inline constexpr double jkqtp_todouble(const bool& d) {
    return static_cast<double>((d)?1.0:0.0);
}


/** \brief round a double \a v using round() and convert it to a specified type T (static_cast!)
 *  \ingroup jkqtptools_math_basic
 *
 *  \tparam T a numeric datatype (int, double, ...)
 *  \param v the value to round and cast
 *
 *  this is equivalent to
 *  \code
 *     static_cast<T>(round(v));
 *  \endcode
 *
 * \callergraph
 */
template<typename T>
inline T jkqtp_roundTo(const double& v) {
    return static_cast<T>(round(v));
}

/** \brief round a double \a v using round() to a given number of decimal digits
 *  \ingroup jkqtptools_math_basic
 *
 *  \param v the value to round and cast
 *  \param decDigits number of decimal digits, i.e. precision of the result
 *
 *  this is equivalent to
 *  \code
 *     round(v * pow(10.0,(double)decDigits))/pow(10.0,(double)decDigits);
 *  \endcode
 *
 * \callergraph
 */
inline double jkqtp_roundToDigits(const double& v, const int decDigits) {
    const double fac=pow(10.0,(double)decDigits);
    return round(v * fac) / fac;
}

/** \brief round a double \a v using ceil() and convert it to a specified type T (static_cast!)
 *  \ingroup jkqtptools_math_basic
 *
 *  \tparam T a numeric datatype (int, double, ...)
 *  \param v the value to ceil and cast
 *
 *  this is equivalent to
 *  \code
 *     static_cast<T>(ceil(v));
 *  \endcode
 *
 * \callergraph
 */
template<typename T>
inline T jkqtp_ceilTo(const double& v) {
    return static_cast<T>(ceil(v));
}

/** \brief round a double \a v using trunc() and convert it to a specified type T (static_cast!)
 *  \ingroup jkqtptools_math_basic
 *
 *  \tparam T a numeric datatype (int, double, ...)
 *  \param v the value to trunc and cast
 *
 *  this is equivalent to
 *  \code
 *     static_cast<T>(trunc(v));
 *  \endcode
 *
 * \callergraph
 */
template<typename T>
inline T jkqtp_truncTo(const double& v) {
    return static_cast<T>(trunc(v));
}

/** \brief round a double \a v using floor() and convert it to a specified type T (static_cast!)
 *  \ingroup jkqtptools_math_basic
 *
 *  \tparam T a numeric datatype (int, double, ...)
 *  \param v the value to floor and cast
 *
 *  this is equivalent to
 *  \code
 *     static_cast<T>(floor(v));
 *  \endcode
 *
 * \callergraph
 */
template<typename T>
inline T jkqtp_floorTo(const double& v) {
    return static_cast<T>(floor(v));
}


/** \brief round a double \a v using round() and convert it to a specified type T (static_cast!).
 *         Finally the value is bounded to the range \a min ... \a max
 *  \ingroup jkqtptools_math_basic
 *
 *  \tparam T a numeric datatype (int, double, ...)
 *  \param min minimum output value
 *  \param v the value to round and cast
 *  \param max maximum output value
 *
 *  this is equivalent to
 *  \code
 *     qBound(min, static_cast<T>(round(v)), max);
 *  \endcode
 */
template<typename T>
inline T jkqtp_boundedRoundTo(T min, const double& v, T max) {
    return qBound(min, static_cast<T>(round(v)), max);
}

/** \brief round a double \a v using round() and convert it to a specified type T (static_cast!).
 *         Finally the value is bounded to the range \c std::numeric_limits<T>::min() ... \c std::numeric_limits<T>::max()
 *  \ingroup jkqtptools_math_basic
 *
 *  \tparam T a numeric datatype (int, double, ...)
 *  \param v the value to round and cast
 *
 *  this is equivalent to
 *  \code
 *     jkqtp_boundedRoundTo<T>(std::numeric_limits<T>::min(), v, std::numeric_limits<T>::max())
 *  \endcode
 */
template<typename T>
inline T jkqtp_boundedRoundTo(const double& v) {
    return jkqtp_boundedRoundTo<T>(std::numeric_limits<T>::min(), v, std::numeric_limits<T>::max());
}

/** \brief limits a value \a v to the given range \a min ... \a max
 *  \ingroup jkqtptools_math_basic
 *
 *  \tparam T a numeric datatype (int, double, ...)
 *  \param min minimum output value
 *  \param v the value to round and cast
 *  \param max maximum output value
 */
template<typename T>
inline T jkqtp_bounded(T min, T v, T max) {
    return (v<min) ? min : ((v>max)? max : v);
}

/** \brief limits a value \a v to the range of the given type \a T , i.e.  \c std::numeric_limits<T>::min() ... \c std::numeric_limits<T>::max()
 *  \ingroup jkqtptools_math_basic
 *
 *  \tparam T a numeric datatype (int, double, ...) for the output
 *  \tparam TIn a numeric datatype (int, double, ...) or the input \a v
 *  \param v the value to round and cast
 *
 *  \note As a special feature, this function detectes whether one of T or TIn are unsigned and then cmpares against a limit of 0 instead of \c std::numeric_limits<T>::min() .
 */
template<typename T, typename TIn>
inline T jkqtp_bounded(TIn v) {
    if (std::is_integral<T>::value && std::is_integral<TIn>::value && (std::is_signed<TIn>::value!=std::is_signed<T>::value)) {
        return (v<TIn(0)) ? T(0) : ((v>std::numeric_limits<T>::max())? std::numeric_limits<T>::max() : static_cast<T>(v));
    } else {
        return (v<std::numeric_limits<T>::min()) ? std::numeric_limits<T>::min() : ((v>std::numeric_limits<T>::max())? std::numeric_limits<T>::max() : static_cast<T>(v));
    }
}

/** \brief compare two floats \a a and \a b for euqality, where any difference smaller than \a epsilon is seen as equality
 *  \ingroup jkqtptools_math_basic */
inline bool jkqtp_approximatelyEqual(float a, float b, float epsilon=2.0f*JKQTP_FLOAT_EPSILON)
{
    return fabsf(a - b) <= epsilon;
}

/** \brief compare two doubles \a a and \a b for euqality, where any difference smaller than \a epsilon is seen as equality
 *  \ingroup jkqtptools_math_basic */
inline bool jkqtp_approximatelyEqual(double a, double b, double epsilon=2.0*JKQTP_DOUBLE_EPSILON)
{
    return fabs(a - b) <= epsilon;
}

/** \brief compare two floats \a a and \a b for uneuqality, where any difference smaller than \a epsilon is seen as equality
 *  \ingroup jkqtptools_math_basic */
inline bool jkqtp_approximatelyUnequal(float a, float b, float epsilon=2.0f*JKQTP_FLOAT_EPSILON)
{
    return fabsf(a - b) > epsilon;
}

/** \brief compare two doubles \a a and \a b for uneuqality, where any difference smaller than \a epsilon is seen as equality
 *  \ingroup jkqtptools_math_basic */
inline bool jkqtp_approximatelyUnequal(double a, double b, double epsilon=2.0*JKQTP_DOUBLE_EPSILON)
{
    return fabs(a - b) > epsilon;
}

/** \brief returns the given value \a v (i.e. identity function)
 *  \ingroup jkqtptools_math_basic */
template<typename T>
inline T jkqtp_identity(const T& v) {
    return v;
}

/** \brief returns the quare of the value \a v, i.e. \c v*v
 *  \ingroup jkqtptools_math_basic */
template<typename T>
inline T jkqtp_sqr(const T& v) {
    return v*v;
}


/*! \brief 4-th power of a number
    \ingroup jkqtptools_math_basic

*/
template <class T>
inline T jkqtp_pow4(T x) {
    const T xx=x*x;
    return xx*xx;
}

/*! \brief 5-th power of a number
    \ingroup jkqtptools_math_basic

*/
template <class T>
inline T jkqtp_pow5(T x) {
    const T xx=x*x;
    return xx*xx*x;
}

/*! \brief cube of a number
    \ingroup jkqtptools_math_basic

*/
template <class T>
inline T jkqtp_cube(T x) {
    return x*x*x;
}


/*! \brief calculates the sign of number \a x (-1 for x<0 and +1 for x>=0)
    \ingroup jkqtptools_math_basic
*/
template <class T>
inline T jkqtp_sign(T x) {
    if (x<0) return -1;
    else return 1;
}

/** \brief returns the inversely proportional value 1/\a v of \a v
 *  \ingroup jkqtptools_math_basic */
template<typename T>
inline T jkqtp_inverseProp(const T& v) {
    return T(1.0)/v;
}

/** \brief returns the inversely proportional value 1/\a v of \a v and ensures that \f$ |v|\geq \mbox{absMinV} \f$
 *  \ingroup jkqtptools_math_basic */
template<typename T>
inline T jkqtp_inversePropSave(const T& v, const T& absMinV) {
    T vv=v;
    if (fabs(vv)<absMinV) vv=jkqtp_sign(v)*absMinV;
    return T(1.0)/vv;
}

/** \brief returns the inversely proportional value 1/\a v of \a v and ensures that \f$ |v|\geq \mbox{absMinV} \f$, uses \c absMinV=std::numeric_limits<T>::epsilon()*100.0
 *  \ingroup jkqtptools_math_basic */
template<typename T>
inline T jkqtp_inversePropSaveDefault(const T& v) {
    return jkqtp_inversePropSave<T>(v, std::numeric_limits<T>::epsilon()*100.0);
}

#if defined(JKQtPlotter_HAS_j0) || defined(JKQtPlotter_HAS__j0) || defined(DOXYGEN)

    /*! \brief j0() function (without compiler issues)
        \ingroup jkqtptools_math_basic

    */
    inline double jkqtp_j0(double x) {
    #ifdef JKQtPlotter_HAS__j0
        return _j0(x);
    #elif defined(JKQtPlotter_HAS_j0)
        return j0(x);
    #endif
    }

    /*! \brief j1() function (without compiler issues)
        \ingroup jkqtptools_math_basic

    */
    inline double jkqtp_j1(double x) {
    #ifdef JKQtPlotter_HAS__j0
        return _j1(x);
    #elif defined(JKQtPlotter_HAS_j0)
        return j1(x);
    #endif
    }
#endif

#if defined(JKQtPlotter_HAS_jn) || defined(JKQtPlotter_HAS__jn) || defined(DOXYGEN)
    /*! \brief jn() function (without compiler issues)
        \ingroup jkqtptools_math_basic

    */
    inline double jkqtp_jn(int n, double x) {
    #ifdef JKQtPlotter_HAS__jn
        return _jn(n,x);
    #elif defined(JKQtPlotter_HAS_jn)
        return jn(n,x);
    #endif
    }
#endif


#if defined(JKQtPlotter_HAS_y0) || defined(JKQtPlotter_HAS__y0) || defined(DOXYGEN)
    /*! \brief y0() function (without compiler issues)
        \ingroup jkqtptools_math_basic

    */
    inline double jkqtp_y0(double x) {
    #ifdef JKQtPlotter_HAS__y0
        return _y0(x);
    #elif defined(JKQtPlotter_HAS_y0)
        return y0(x);
    #endif
    }

    /*! \brief y1() function (without compiler issues)
        \ingroup jkqtptools_math_basic

    */
    inline double jkqtp_y1(double x) {
    #ifdef JKQtPlotter_HAS__y0
        return _y1(x);
    #elif defined(JKQtPlotter_HAS_y0)
        return y1(x);
    #endif
    }
#endif
#if defined(JKQtPlotter_HAS_yn) || defined(JKQtPlotter_HAS__yn) || defined(DOXYGEN)
    /*! \brief yn() function (without compiler issues)
        \ingroup jkqtptools_math_basic

    */
    inline double jkqtp_yn(int n, double x) {
    #ifdef JKQtPlotter_HAS__yn
        return _yn(n,x);
    #elif defined(JKQtPlotter_HAS_yn)
        return yn(n,x);
    #endif
    }
#endif


/** \brief calculate the distance between two QPointF points
 *  \ingroup jkqtptools_math_basic
 *
 */
inline double jkqtp_distance(const QPointF& p1, const QPointF& p2){
    return sqrt(jkqtp_sqr<double>(p1.x()-p2.x())+jkqtp_sqr<double>(p1.y()-p2.y()));
}

/** \brief calculate the distance between two QPoint points
 *  \ingroup jkqtptools_math_basic
 *
 */
inline double jkqtp_distance(const QPoint& p1, const QPoint& p2){
    return sqrt(jkqtp_sqr<double>(p1.x()-p2.x())+jkqtp_sqr<double>(p1.y()-p2.y()));
}

/** \brief check whether the dlotaing point number is OK (i.e. non-inf, non-NAN)
 * \ingroup jkqtptools_math_basic
 */
template <typename T>
inline bool JKQTPIsOKFloat(T v) {
    return std::isfinite(v)&&(!std::isinf(v))&&(!std::isnan(v));
}

inline bool JKQTPIsOKFloat(const QPointF& v) {
    return JKQTPIsOKFloat<qreal>(v.x()) && JKQTPIsOKFloat<qreal>(v.y());
}

inline bool JKQTPIsOKFloat(const QLineF& v) {
    return JKQTPIsOKFloat<qreal>(v.x1()) && JKQTPIsOKFloat<qreal>(v.x2()) && JKQTPIsOKFloat<qreal>(v.y1()) && JKQTPIsOKFloat<qreal>(v.y2());
}

inline bool JKQTPIsOKFloat(const QRectF& v) {
    return JKQTPIsOKFloat<qreal>(v.x()) && JKQTPIsOKFloat<qreal>(v.x()) && JKQTPIsOKFloat<qreal>(v.width()) && JKQTPIsOKFloat<qreal>(v.height());
}

/** \brief evaluates a gaussian propability density function
 * \ingroup jkqtptools_math_basic
 *
 * \f[ f(x,\mu, \sigma)=\frac{1}{\sqrt{2\pi\sigma^2}}\cdot\exp\left(-\frac{(x-\mu)^2}{2\sigma^2}\right) \f]
 */
inline double jkqtp_gaussdist(double x, double mu=0.0, double sigma=1.0) {
    return exp(-0.5*jkqtp_sqr(x-mu)/jkqtp_sqr(sigma))/sqrt(2.0*JKQTPSTATISTICS_PI*sigma*sigma);
}

/*! \brief evaluate a polynomial \f$ f(x)=\sum\limits_{i=0}^Pp_ix^i \f$ with \f$ p_i \f$ taken from the range \a firstP ... \a lastP
    \ingroup jkqtptools_math_basic

    \tparam PolyItP iterator for the polynomial coefficients
    \param x where to evaluate
    \param firstP points to the first polynomial coefficient \f$ p_1 \f$ (i.e. the offset with \f$ x^0 \f$ )
    \param lastP points behind the last polynomial coefficient  \f$ p_P \f$
    \return value of polynomial \f$ f(x)=\sum\limits_{i=0}^Pp_ix^i \f$ at location \a x

*/
template <class PolyItP>
inline double jkqtp_polyEval(double x, PolyItP firstP, PolyItP lastP) {
    double v=0.0;
    double xx=1.0;
    for (auto itP=firstP; itP!=lastP; ++itP) {
        v=v+(*itP)*xx;
        xx=xx*x;
    }
    return v;
}


/*! \brief a C++-functor, which evaluates a polynomial
    \ingroup jkqtptools_math_basic
*/
struct JKQTCOMMON_LIB_EXPORT JKQTPPolynomialFunctor {
        std::vector<double> P;
        template <class PolyItP>
        inline JKQTPPolynomialFunctor(PolyItP firstP, PolyItP lastP) {
            for (auto itP=firstP; itP!=lastP; ++itP) {
                P.push_back(*itP);
            }
        }
        inline double operator()(double x) const { return jkqtp_polyEval(x, P.begin(), P.end()); }

};

/*! \brief returns a C++-functor, which evaluates a polynomial
    \ingroup jkqtptools_math_basic

    \tparam PolyItP iterator for the polynomial coefficients
    \param firstP points to the first polynomial coefficient \f$ p_1 \f$ (i.e. the offset with \f$ x^0 \f$ )
    \param lastP points behind the last polynomial coefficient  \f$ p_P \f$
*/
template <class PolyItP>
inline std::function<double(double)> jkqtp_generatePolynomialModel(PolyItP firstP, PolyItP lastP) {
    return JKQTPPolynomialFunctor(firstP, lastP);
}

/*! \brief Generates a LaTeX string for the polynomial model with the coefficients \a firstP ... \a lastP
    \ingroup jkqtptools_math_basic

    \tparam PolyItP iterator for the polynomial coefficients
    \param firstP points to the first polynomial coefficient \f$ p_1 \f$ (i.e. the offset with \f$ x^0 \f$ )
    \param lastP points behind the last polynomial coefficient  \f$ p_P \f$
    */
template <class PolyItP>
QString jkqtp_polynomialModel2Latex(PolyItP firstP, PolyItP lastP) {
    QString str="f(x)=";
    size_t p=0;
    for (auto itP=firstP; itP!=lastP; ++itP) {
        if (p==0) str+=jkqtp_floattolatexqstr(*itP, 3);
        else {
            if (*itP>=0) str+="+";
            str+=QString("%2{\\cdot}x^{%1}").arg(p).arg(jkqtp_floattolatexqstr(*itP, 3));
        }
        p++;
    }
    return str;
}



/*! \brief Calculates a factorial \f$ n!=n\cdot(n-1)\cdot(n-2)\cdot...\cdot2\cdot1 \f$
    \ingroup jkqtptools_math_basic

    */
template <class T=int>
inline T jkqtp_factorial(T n) {
    T result = 1;
    for (T i =1; i <= n; i++){
        result = result*i;
    }
    return result;
}


/*! \brief Calculates a combination \f$ \left(\stackrel{n}{k}\right)=\frac{n!}{k!\cdot(n-k)!} \f$
    \ingroup jkqtptools_math_basic

    */
template <class T=int>
inline T jkqtp_combination(T n, T k) {
    if (n==k) return 1;
    if (k==0) return 1;
    if (k>n) return 0;
    return jkqtp_factorial(n)/(jkqtp_factorial(k)*jkqtp_factorial(n-k));
}



/*! \brief creates a functor that evaluates the Bernstein polynomial \f$ B_i^n(t):=\left(\stackrel{n}{i}\right)\cdot t^i\cdot(1-t)^{n-1},\ \ \ \ 0\leq i\leq n \f$
    \ingroup jkqtptools_math_basic

    */
template <class T>
std::function<T(T)> jkqtp_makeBernstein(int n, int i){
    if (n==0 && i==0) return [=](T t) { return 1; };
    if (n==1 && i==0) return [=](T t) { return (1.0-t); };
    if (n==1 && i==1) return [=](T t) { return t; };
    if (n==2 && i==0) return [=](T t) { return jkqtp_sqr(1.0-t); };
    if (n==2 && i==1) return [=](T t) { return T(2.0)*t*(1.0-t); };
    if (n==2 && i==2) return [=](T t) { return jkqtp_sqr(t); };
    if (n==3 && i==0) return [=](T t) { return T(1)*jkqtp_cube(1.0-t); };
    if (n==3 && i==1) return [=](T t) { return T(3)*t*jkqtp_sqr(1.0-t); };
    if (n==3 && i==2) return [=](T t) { return T(3)*jkqtp_sqr(t)*(1.0-t); };
    if (n==3 && i==3) return [=](T t) { return T(1)*jkqtp_cube(t); };
    if (n==4 && i==0) return [=](T t) { return T(1)*jkqtp_pow4(1.0-t); };
    if (n==4 && i==1) return [=](T t) { return T(4)*t*jkqtp_cube(1.0-t); };
    if (n==4 && i==2) return [=](T t) { return T(6)*jkqtp_sqr(t)*jkqtp_sqr(1.0-t); };
    if (n==4 && i==3) return [=](T t) { return T(4)*jkqtp_cube(t)*(1.0-t); };
    if (n==4 && i==4) return [=](T t) { return T(1)*jkqtp_pow4(t); };
    const T fac=jkqtp_combination<int64_t>(n,i);
    return [=](T t) { return fac*pow(t,i)*pow(1.0-t,n-i); };
}


/*! \brief calculate the grwates common divisor (GCD) of \a a and \a b
    \ingroup jkqtptools_math_basic

    */
JKQTCOMMON_LIB_EXPORT uint64_t jkqtp_gcd(uint64_t a, uint64_t b);


/*! \brief calculates numeratur integer part \a intpart , \a num and denominator \a denom of a fraction, representing a given floating-point number \a input
    \ingroup jkqtptools_math_basic

    */
JKQTCOMMON_LIB_EXPORT void jkqtp_estimateFraction(double input, int &sign, uint64_t &intpart, uint64_t& num, uint64_t& denom, unsigned int precision=9);

/*! \brief returns the reversed containter \a l
    \ingroup jkqtptools_math_basic

    */
template <class T>
inline T jkqtp_reversed(const T& l) {
    T reversed_l;
    reversed_l.reserve(l.size());
    std::reverse_copy(l.begin(), l.end(), std::back_inserter(reversed_l));
    return reversed_l;
}

/*! \brief can be used to build a hash-values from several hash-values
    \ingroup jkqtptools_math_basic

    \code
        std::size_t seed=0;
        jkqtp_hash_combine(seed, valA);
        jkqtp_hash_combine(seed, valB);
        //...
        // finally seed contains the combined hash
    \endcode

*/
template <class T>
inline void jkqtp_hash_combine(std::size_t& seed, const T& v)
{
    const auto hsh=::qHash(v,0);
    seed ^= hsh + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

/*! \brief can be used to build a hash-values from several hash-values
    \ingroup jkqtptools_math_basic

    \code
        std::size_t seed=0;
        jkqtp_combine_hash(seed, qHash(valA));
        jkqtp_combine_hash(seed, qHash(valB));
        //...
        // finally seed contains the combined hash
    \endcode

*/
inline void jkqtp_combine_hash(std::size_t& seed,  std::size_t hsh)
{
    seed ^= hsh + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

#endif // jkqtpmathtools_H_INCLUDED
