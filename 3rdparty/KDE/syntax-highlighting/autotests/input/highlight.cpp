#pragma once

#include <cassert>
#include <assert.h>
#include "assert.h"
#include "assert.hpp" // abc
#include "path/assert.hpp"
#include "assert.h"a
#include "assert.h" a
#include <cassert>a
#include <cassert> a
#include FOO() error
#include_next <cassert> a
#include_next <cassert> /* a
 */ b
#include PATH_IN_MACRO
#include PATH_IN_MACRO()
#include PATH_IN_MACRO(a, b)

#define SOME_VAR 1
#ifdef SOME_VAR

#define MULTILINE_MACRO one \
two \
three

# define MULTILINE_MACRO_TEXT                        \
    /* NOTE The contents of macro is too green :D */ \
    char const s[] = "a\\b"                          \
    "c\nd"                                           \
    std::uint##x##_t                                 \
    std::vector/**/<T>                               \
    std::chrono::/*milli*/seconds


# define VARIADIC(a, ...)          \
    f(a##a)                        \
    f(__VA_ARGS__)                 \
    f(#__VA_ARGS__)                \
    f(__VA_ARGS__)                 \
    f(0 __VA_OPT__(,) __VA_ARGS__) \
    x __VA_OPT__(= { __VA_ARGS__ })

# define MACRO() BAD \ ESCAPED

# error dds
# warning dds
# line 2 "file.cpp"
# define A(x, y) x##y x#y
// OK(L, a) -> L"a"
# define OK(x, y) x###y
# define BAD(x, y) x####y
# define A /* multi line
with comment */ expr
# define A /* multi line
with comment */
23
#else // x
#42 // gcc extension = #line 42

// error
#wrong
# wrong
#endif x
#if DS()
#else x
#else /* */x
#else /* x
y */ z
#endif

// check that _XXX defines work, bug 397766
#ifndef _HEADER_GUARD
#define _HEADER_GUARD 1
#endif
#ifdef _HEADER_GUARD
#if (_HEADER_GUARD >= 1)
#endif
#endif

static int g_global;

template<class T, typename U, template<class> class = std::is_pointer>
struct class1
  : private std::vector<T>, public U
{
    class1()
        try
        : _member1(xxx)
    {}
        catch(...)
    {}

    class1(class1&&) = default;

    ~class1()
    {}

    void foo() { return; }
    void foo() const { return; }
    void foo() noexcept { return; }
    void foo() const noexcept { return; }
    virtual void foo() const noexcept { return; }
    static void foo() { return; }
    constexpr static void foo() const
        noexcept(noexcept(std::is_pointer<U>::value)) override
    {
        xxx::template ttt<U>::type {};
        xxx.template get<U>();
        xxx.std::rdbuf();
        auto x = C<'a'> + y;
    }

    int operator->*(T (C::*m)(int));
    operator value_t ();

private:
protected:
public:
    value_type _member1; // NOTE internal ?
    value_type __internal;
    value_type internal__;
    value_type _M_internal;
    value_t member2_;
    value_type m_member3;

    static int s_static;
    static constexpr int s_static;
    static inline int s_static;
    static inline constexpr int s_static;
};

constexpr struct : xyz
{
    using xyz::xyz;
    using xyz::operator=;

    int a : 1;
    int b : 7;
} x {};

template<class T>
using is_pointer = std::is_pointer<T>::type;

template<class T>
constexpr auto is_pointer_v = std::is_pointer<T>::value;

uint64_t namespaces()
{
    std::vector<T>;
    boost::vector<T>;
    detail::vector<T>;
    details::vector<T>;
    aux::vector<T>;
    internals::vector<T>;
    other::vector<T>;
}

task<> tcp_echo_server() {
  char data[1024];
  for (;;) {
    size_t n = co_await socket.async_read_some(buffer(data));
    co_await async_write(socket, buffer(data, n));
  }
}

#if 1
    double foo(const A);
#else // else
    double foo(const A);
#endif // end

#if 0
    double foo(const A);
#else // else
    double foo(const A);
#endif // end

#if 1
    double foo(const A);
#elif 1
    double foo(const A);
#elif 0
    double foo(const A);
#endif // end

#if 0
    double foo(const A);
#elif 1
    double foo(const A);
#elif 0
    double foo(const A);
#endif // end

#if 0
    double foo(const A);
#elif a
    double foo(const A);
#elif 0
    double foo(const A);
#elif a
    double foo(const A);
#else // else
    double foo(const A);
#endif // end

#if 0 // blah blah
    double foo(const A);
#elif 1 // blah blah
    double foo(const A);
#else // else
    double foo(const A);
#endif // end

#if 0 || a
    double foo(const A);
#else // else
    double foo(const A);
#endif // end

#if 1 || a
    double foo(const A);
#else // else
    double foo(const A);
#endif // end

#if 0 && a
    double foo(const A);
#else // else
    double foo(const A);
#endif // end

#if 1 && a
    double foo(const A);
#else // else
    double foo(const A);
#endif // end

#if a
    double foo(const A);
#elif 0
    double foo(const A);
#endif // end

#if a
    double foo(const A);
#elif 1
    double foo(const A);
#endif // end

#if a
    double foo(const A);
#elif a
    double foo(const A);
#endif // end

int bar(void*p, void * pp)
{
# if 0
    double foo();
# else // else
    double foo();
# endif // end
}

#if abc 0
    double foo();
#endif // end

#if xxx
    double foo();
#else
    double foo();
#endif // end

#if xxx
    double foo();
#elif xxx // elseif
    double foo();
#elif xxx // elseif
    double foo();
#endif // end

// error
#
#d
# d
#if
#elif
#endif
#ifndef
#endif
#ifdef 0
#endif // end

static uint64_t intWithSuffix = 42ull;
static long intWithSuffixAndPrefix = 0b0101L;
static int octNum = 07232;
static int invalidOctNum = 09231;
static uint64_t hexNum = 0xDEADBEEF42;
static uint64_t invalidHexNum = 0xGLDFKG;
static char binNum = 0b0101010;

static int64_t intWithSuffix = -42LL;
static long intWithSuffixAndPrefix = -0b0101L;
static int octNum = -07232;
static int invalidOctNum = -09231;
static int64_t hexNum = -0xDEADBEEF42;
static int64_t invalidHexNum = -0xGLDFKG;
static char binNum = -0b0101010;

static uint64_t intWithSuffixWithSep = 4'2ull;
static long intWithSuffixAndPrefixWithSep = 0b0'10'1L;
static int octNumWithSep = 07'232;
static int invalidOctNumWithSep = 09'23'1;
static uint64_t hexNumWithSep = 0xD'EAD'BE'EF'42;
static uint64_t invalidHexNumWithSep = 0xGLD'FKG;
static char binNumWithSep = 0b0'1010'10;

static uint64_t invalidSep = 42'ull;
static uint64_t invalidSep = 42';

static double d1 = 42.;
static double d2 = .42;
static double d2a = -0.49;
static double d2b = -0.09;
static double d3 = 42.3e1;
static double d4 = .2e-12;
static double d5 = 32.e+12;
static double invalidD1 = 32.e+a12;
static float floatQualifier = 23.123f;
// Hexadecimal floating point (c++17)
static double d6 = 0x1ffp10;
static double d7 = 0X0p-1;
static double d8 = 0x1.p0;
static double d9 = 0xf.p-1L;
static double d10 = 0x0.123p-1;
static double d11 = 0xa.bp10l;
static double invalidD2 = 0x0.123p-a;
static float floatQualifier = 0xf.p-1f;

60min; // c++17
60.min;
60.3min;
0x1ffp10min;
2us
2d; // c++20
23._f
23._fd
2.3_f
2.3_fd
2._f
2._fd
2e4_f
2e4_fd

// error
23.fd
2e_fd
2e
1.y
1.0_E+2.0
1.0_E +2.0 // ok
1_p+2
1_p +2 // ok
4s.count()
4s. count()
4s .count() // ok

static bool yes = true;
static bool no = false;

// *char*
static const char     c1 = 'c';
static const char     c1a = u8'c'; // utf-8 char (c++17)
static const char16_t c1b = u'c';
static const char32_t c1c = U'c';
static const wchar_t  c1d = L'c';
static const char c2 = '\n';
static const char c2a = '\120'; // octal
static const char c2b = '\x1f'; // hex
static const char c2c = '\'';
static const char c2d = '\\';
static const wchar_t c2e = L'\x1ff'; // hex
static const wchar_t c2e = U'\x1fffffff'; // hex
// error
'\x123';
'\u1234';
'\U12345678';
U'\u12345';
u'\u123';
U'\U1234567';
U'\U123456789';
U'\x123456789';

// string
static const char* c3 = "string";
static const char* c4 = "\"string\n\t\012\x12\"";
static const char* c5 = "multiline \
    string";
static const char* c6 = "multifragment" "other""string";
static const char*      c6a = u8"string";
static const char16_t*  c6b = u"string";
static const char32_t*  c6c = U"string";
static const wchar_t*   c6d = L"string";
static const char* rawString1 = R"(string)";
static const char*      rawString1a = u8R"(string)";
static const char16_t*  rawString1b = uR"(string)";
static const char32_t*  rawString1c = UR"(string)";
static const wchar_t*   rawString1d = LR"(string\nstring)";
static const char* rawString2 = R"ab(string\nstring%s)ab ")ab";
static const char* rawString3 = R"ab(string
string%)ab";
" %d %df fd" U"ds %d" R"(a%d)";
"\x{12343} \x{1} \o{12121} \u{1221a} \N{LATIN CAPITAL LETTER A WITH MACRON}"
  "\x123xsk";
 u"\x123xsk";
// error
u8"\x123xsk";
"\x{123x43} \o{121821} \u{12x21as} \N{LATIN CAPITAL letter A WITH MACRON}"

// printf format
"a%10sa%-10sa%*sa%-10.*sa%-*.*sa"
"a%ca%*ca%%a%ia%da%.6ia%.0ia%+ia%ia%xa%#xa"
"a%.0fa%.32fa%05.2fa%.2fa%5.2fa%Ea%aa"
// std::format
"a{{:6}}a{}a{:6}a{:*<6}a{:6d}a"
"a{0:}a{0:+}a{0:-}a{0: }a"
"a{:+06d}a{:#06x}a{:<06}a"
"a{:{}}a{0:{1}.{2}f}a"
;

// UDL (c++11)

operator""_a(const char*);
operator ""_a(const char*);
operator "" _a(const char*);
// invalid suffix
operator "" a(const char*);
operator ""a(const char*);
operator""a(const char*);

"string"_s; // user
"string"s; // standard
"string"_s-b; // -b is not part of the string

// Macro

MY_XXX;
BOOST_XXX;
__STDC_VERSION__;
__TIME__;
__cplusplus;

// Attributes

[[noreturn]] void foo();
[[deprecated]] void foo();
[[deprecated("because")]] void foo();
void foo([[carries_dependency]] int);

[[opt(1), debug]]
[[using CC: opt(1), debug]] // c++17
[[using CC: CC::opt(1)]] // c++17

[[gnu::assume_aligned(3'2l,2)]] void* f();
[[using gnu: assume_aligned(3)]]
[[clang::assume_aligned(3)]]

void f([[maybe_unused]] int val)
{
    [[maybe_unused]] int x;
    switch (x = foo(); x) {
        case 1:
            [[fallthrough]];
        case XXX:
        case Class::foo():
            [[fallthrough]];
        default:
            ;
    }

    // c++17: fold expression
    (args + ... + (1 * 2));
    (v.push_back(args), ...);

    [[omp::parallel]] for (auto&& x : v)
        x;
    for (auto&& [first,second] : mymap) {
    }

    auto [x, y] = foo();

    [x = 1, =y](){};

    decltype((auto)) x = foo();
}

auto f() -> decltype(foo());

__attribute__((pure)) void f();

label:
goto label;


//BEGIN region
// TODO comment FIXME comment ### comment BUG comment
//END region

// \brief blah blah
/// \brief blah blah

/**
 * Doxygen
 * @param p1 text
 * \brief <b>bold text</b>
 * \dot
 * a -> b
 * \enddot
 *
 * \verbatim
 * <dummy>
 * \endverbatim
 * <html>text</html>
 */

#endif

// Some GNU macros, cross-referenced from gcc.xml to isocpp.xml
__GCC_ATOMIC_CHAR16_T_LOCK_FREE
__GNUC__
__linux__

// Digraphs (see bug #411508)
%:include <stdio.h>
int main()
<%
    [](){%>();
}
<:<:fallthrough]]

/**
 * multi-line comment
 */

/* comment */
{ { } {
//BEGIN
}
//END
}

#if 0
#elif 1
#else
#endif

#if 1
int x; // variable shall not be grey
#endif
#if defined (A)
int y; // variable shall not be grey
#elif defined (B)
int z; // variable shall not be grey
#endif

/*!
 * formula @f$a+b@f$ inside a block comment
 */

//! formula @f$a+b@f$ inside a inline comment
// formula @f$a+b@f$ inside a normal comment

/// \b sa
