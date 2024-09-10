#pragma once

// Simple tuple implementation for Arduino boards.
// Since the standard library is not available, we need to implement our own tuple.
// This is a simplified version of the standard tuple, with only the necessary parts.
// Source:
// https://stackoverflow.com/questions/4041447/how-is-stdtuple-implemented


// Tuple element
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
struct Tuple : public TupleImpl<0, Elements...> {
    // Size of the tuple
    static constexpr size_t size() {
        return sizeof...(Elements);
    }
};

// Get item from tuple
template <size_t Index, typename Head, typename... Tail>
constexpr Head& get_item(TupleImpl<Index, Head, Tail...>& tuple) {
    return tuple.TupleElement<Index, Head>::value;
}

template <size_t Index, typename Tuple>
constexpr auto get_tuple_item(Tuple& tuple) 
-> decltype(get_item<Index>(tuple)) 
{
    return get_item<Index>(tuple);
}