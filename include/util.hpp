#include <type_traits>
#ifndef UTIL_HPP
#define UTIL_HPP 1

#include <variant>
#include <cstddef>

#if UNICODE == 1
using screenchr = wchar_t;
#else
using screenchr = char;
#endif

/*Shamelessly stolen from stackoverflow
 * https://stackoverflow.com/questions/52303316/get-index-by-type-in-stdvariant
 * by Barry*/
template <typename> struct tag { };
template <typename T, typename V>
struct get_index;
template <typename T, typename... Ts> 
struct get_index<T, std::variant<Ts...>>
    : std::integral_constant<size_t, std::variant<tag<Ts>...>(tag<T>()).index()>
{ };

template<typename T, typename variant_type>
struct is_variant : std::false_type {};
template<typename T, typename ... Vs>
struct is_variant<T, std::variant<Vs...>>
   : std::disjunction<std::is_same<T, Vs>...> {};
template<typename T, typename variant_type>
constexpr bool is_variant_v = is_variant<T, variant_type>::value;

template<typename T, unsigned base, unsigned p>
struct cxpow { static constexpr T value = (T)base * cxpow<T, base, p - 1>::value; };
template<typename T, unsigned base>
struct cxpow<T, base, 0> { static constexpr T value = 1; };
template<typename T, unsigned base, unsigned p>
constexpr T cxpow_v = cxpow<T, base, p>::value;

template<typename T, unsigned base, int p>
struct cxrt { static constexpr T value = (T)base / cxpow<T, base, -p>::value; };
template<typename T, unsigned base, int p>
constexpr T cxrt_v = cxrt<T, base, p>::value;

#endif
