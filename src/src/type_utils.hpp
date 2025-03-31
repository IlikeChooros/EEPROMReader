#pragma once

/*

This is basic implementation of something similiar to 'type_traits' in C++11
Has only essential parts, for the library to work.

*/

#include "namespaces.hpp"

BEGIN_DETAIL_TEEPROM_NAMESPACE

// Remove reference from a type
template <typename T>
struct remove_reference { typedef T type; };

template <typename T>
struct remove_reference<T&> { typedef T type; };

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;


// Is base of
// From: https://en.cppreference.com/w/cpp/types/is_base_of

template <bool B>
struct bool_constant {
    static constexpr bool value = B;
};

struct true_type : public bool_constant<true> {};
struct false_type : public bool_constant<false> {};

template <typename Base>
true_type test_ptr_conv(const volatile Base*);

template <typename>
false_type test_ptr_conv(const volatile void*);

template <typename Base, typename Derived>
auto test_base_of(int) 
-> decltype(test_ptr_conv<Base>(static_cast<Derived*>(nullptr)));

template <typename, typename>
auto test_base_of(...) -> true_type;

template <typename Base, typename Derived>
struct is_base_of  : 
    public bool_constant<decltype(test_base_of<Base, Derived>(0))::value> 
{};

// Check if the type is a base of another type
template <typename Base, typename Derived>
constexpr bool isBaseOf() { 
    return is_base_of<Base, remove_reference_t<Derived>>::value;
}


END_DETAIL_TEEPROM_NAMESPACE