#pragma once

// Simple tuple implementation for Arduino boards.
// Since the standard library is not available, we need to implement our own tuple.

// Tuple implementation

template <size_t Index, typename Value>
struct TupleElement {
    Value value;
};

// Tuple implementation
template <size_t Index, typename... Elements>
struct TupleImpl;

// Specialization for the case when there are no elements left
template <size_t Index>
struct TupleImpl<Index> {};

// Specialization for the case when there is at least one element left
template <size_t Index, typename Head, typename... Tail>
struct TupleImpl<Index, Head, Tail...> : 
    public TupleElement<Index, Head>, 
    public TupleImpl<Index + 1, Tail...> 
    {};

// Tuple class
template <typename... Elements>
using Tuple = TupleImpl<0, Elements...>;

// Get item from tuple
template <size_t Index, typename Head, typename... Tail>
constexpr Head& get_item(TupleImpl<Index, Head, Tail...>& tuple) {
    return tuple.TupleElement<Index, Head>::value;
}