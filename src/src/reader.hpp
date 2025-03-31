#pragma once

// TODO: Add support for Arduino boards with this
// #include <type_traits>

#include "settings.hpp"
#include "eeprom.hpp"
#include "tuple.hpp"
#include "type_utils.hpp"

BEGIN_DETAIL_TEEPROM_NAMESPACE

template <typename... Types>
using tuple_t = Tuple<Types...>;

template <size_t Index, typename Tuple>
constexpr auto getTupleItem(Tuple& tuple) noexcept
-> decltype(get_tuple_item<Index>(tuple))
{
    return get_tuple_item<Index>(tuple);
}

template <typename T>
constexpr T& refrence(T& t) noexcept {
    return static_cast<T&>(t);
}

/**
 * @brief Returns true if the index is out of bounds
 */
template <typename Tuple, size_t Index>
constexpr bool checkIndexOutOfBounds() noexcept {
    return Index >= Tuple::size();
}

// ------------------ EEPROM Fields ------------------

struct BaseEEPROMField {};

// EEROMFields stores an array of data in given type (or a single string)
template <typename T, size_t N>
struct EEPROMFields : public BaseEEPROMField {
    T data[N];
};

// EEPROMField stores a single data in given type
template <typename T>
struct EEPROMField : public EEPROMFields<T, 1>, public BaseEEPROMField {};

// EEPROMFieldArray stores a 2D array of data in given type (used mainly for strings)
template <typename T, size_t N, size_t Len>
struct EEPROMFieldsArray : public BaseEEPROMField {
    T data[N][Len];
};

// EEPROMStrings stores an array of strings in EEPROM
template <size_t N, size_t Len>
struct EEPROMStrings : public EEPROMFieldsArray<char, N, Len> {};

// EEPROMString stores a single string in EEPROM (this is a C-style string)
struct EEPROMString : public BaseEEPROMField {
    String data; // This is possible because of the operator[] overloading in the String class
};


// ------------------ EEPROM Tuple Related Code ------------------

// Get the size of the field (in bytes)
template <typename Field>
inline size_t getFieldSize(Field& field) noexcept 
{
    static_assert(
        // std::is_base_of<BaseEEPROMField, Field>::value,
        isBaseOf<BaseEEPROMField, remove_reference_t<Field>>(),
        "getFieldSize: Field must be of type EEPROMField, EEPROMString, or EEPROMFields"
    );
    return sizeof(field.data);
}

// Specialization for strings
template<>
inline size_t getFieldSize<EEPROMString>(EEPROMString& field) noexcept 
{
    return field.data.length() + 1; // +1 for the null terminator
}

// Get total byte size of the tuple
template<typename Tuple, size_t Index = 0>
inline size_t getTupleBytesize(Tuple& tuple) noexcept
{
    if constexpr (!checkIndexOutOfBounds<Tuple, Index>()){
        return getFieldSize(getTupleItem<Index>(tuple)) + getTupleBytesize<Tuple, Index + 1>(tuple);
    }
    return 0;
}

/**
 * @brief Check if the address is out of bounds
 * @tparam Tuple The tuple 
 * @tparam Index The index of the tuple
 */
template <typename Tuple, size_t Index>
inline bool checkAddressOutOfBounds(Tuple& tuple, size_t& address) noexcept {
    return address + getFieldSize(getTupleItem<Index>(tuple)) >= EEPROM_CLASS.get_eeprom_size();
}

/**
 * @brief Put a field to EEPROM
 * @tparam Field The field must have a `data` field
 */
template <typename Field>
inline void putToEEPROM(Field& field, size_t& address) noexcept {
    EEPROM_CLASS.put(address, field.data);
    address += getFieldSize(field);
}

// Specialization for strings
template <>
inline void putToEEPROM<EEPROMString>(EEPROMString& field, size_t& address) noexcept {
    EEPROM_CLASS.writeString(address, field.data.c_str());
    address += getFieldSize(field);
}

/**
 * @brief Puts a tuple of data to EEPROM, up to the size of the tuple or until the address is out of bounds
 * @tparam Tuple The tuple 
 * @return True if all data was saved successfully, false otherwise
 */
template <typename Tuple, size_t Index = 0>
inline bool putTupleToEEPROM(Tuple& tuple, size_t& address = 0) noexcept 
{
    // If the index is less than the size of the tuple, put the data to EEPROM
    if constexpr (!checkIndexOutOfBounds<Tuple, Index>()){
        // If the address is out of bounds, return
        if (checkAddressOutOfBounds<Tuple, Index>(tuple, address)){
            return false;
        }
        putToEEPROM(getTupleItem<Index>(tuple), address);
        return putTupleToEEPROM<Tuple, Index + 1>(tuple, address); // Recursively call itself
    }
    return true;
}

/**
 * @brief Read a field from EEPROM
 * @tparam Field The field must have `data` and `size` fields
 */
template <typename Field>
inline void readFromEEPROM(Field& field, size_t& address) noexcept {
    EEPROM_CLASS.get(address, field.data);
    address += getFieldSize(field);
}

// Specialization for strings
template <>
inline void readFromEEPROM<EEPROMString>(EEPROMString& field, size_t& address) noexcept {
    field.data  = EEPROM_CLASS.readString(address);
    address    += getFieldSize(field);
}


/**
 * @brief Reads a tuple of data from EEPROM
 * @tparam Tuple The tuple to be stored in EEPROM, each element of the tuple must have a `data` field
 * @return True if all data was read successfully, false otherwise
 */
template <typename Tuple, size_t Index = 0>
inline bool readTupleFromEEPROM(Tuple& tuple, size_t& address = 0) noexcept 
{
    // If the index is less than the size of the tuple, put the data to EEPROM
    if constexpr (!checkIndexOutOfBounds<Tuple, Index>()){
        // If the address is out of bounds, return
        if (checkAddressOutOfBounds<Tuple, Index>(tuple, address)){
            return false;
        }
        readFromEEPROM(getTupleItem<Index>(tuple), address);
        return readTupleFromEEPROM<Tuple, Index + 1>(tuple, address); // Recursively call itself
    }
    return true;
}


// Verify all tuple fields
template <typename Tuple, size_t Index = 0>
inline void verifyFields(Tuple& tuple) noexcept
{
    if constexpr (!checkIndexOutOfBounds<Tuple, Index>())
    {
        // If the field is a string, check if the size is correct
        static_assert(isBaseOf<BaseEEPROMField, decltype(getTupleItem<Index>(tuple))>(), 
            "Field failure: all fields must be of type EEPROMField, EEPROMString, or EEPROMFields");

        // Go through the rest of the tuple
        verifyFields<Tuple, Index + 1>(tuple);
    }
}

END_DETAIL_TEEPROM_NAMESPACE

// Shortcut for EEPROMString
using EStr = TEEPROM_NAMESPACE::EEPROMString;

// Shortcut for EEPROMField
template <typename Field>
using EF = TEEPROM_NAMESPACE::EEPROMField<Field>;

// Shortcut for EEPROMFields
template <typename Field, size_t N>
using EFs = TEEPROM_NAMESPACE::EEPROMFields<Field, N>;

// Shortcut for EEPROMFieldsArray (2D array N x Len of data)
template <typename T, size_t N, size_t Len>
using EFArr = TEEPROM_NAMESPACE::EEPROMFieldsArray<T, N, Len>;



// ------------------ EEPROM Reader ------------------

/**
 * @brief Provides memory storage for EEPROM, works similar to std::tuple, only 1 instance is allowed at a time
 * @tparam size The size of the EEPROM memory to be used
 * @tparam Fields The fields to be stored in EEPROM, should be of type EEPROMField, EEPROMString, or EEPROMFields
 * 
 * ### Attention
 * 
 * All fields internally are storing data as an array, even if the size is 1. So when 
 * accessing the data with `get`, you are essentially calling: `std::get<index>(tuple).data[value_index]`,
 * where value_index is 0 by default. If the desired field is a string, or it should be interpreted as an array,
 * call `get_data` instead of `get`.
 * 
 * #### Example usage:
 * 
 * // Create a reader with 512 bytes of EEPROM memory, with an integer field, a string field, and an array of 4 floats
 * 
 * ```
 * 
 * Teeprom<512, EEPROMField<int>, EEPROMString, EEPROMFields<float, 4>> reader;
 * 
 * reader.get<0>() = 10;
 * 
 * reader.get_data<1>() = "Hello, World!"; // This is an Arduino String
 * 
 * reader.get<2>(1) = 1.0;
 * 
 * reader.save(); // Commit the changes to EEPROM
 * 
 * ```
 * 
 * Then, to load the data:
 * 
 * ```
 * 
 * Teeprom<512, EEPROMField<int>, EEPROMString, EEPROMFields<float, 4>> reader;
 * 
 * reader.load(); // Load the data from EEPROM
 * 
 * Serial.println(reader.get<0>());
 * 
 * Serial.println(reader.get_data<1>());
 * 
 * Serial.println(reader.get<2>(1));
 * 
 * ```
 */
template <size_t Size, typename... Fields>
class Teeprom 
{
    TEEPROM_NAMESPACE::tuple_t<Fields...> M_fields;
public:

    static_assert(
        (sizeof...(Fields) > 0),
        "At least one field must be provided"
    );

    static_assert(
        Size > 0,
        "Teeprom Size must be greater than 0"
    );

    typedef uint8_t* pointer;

    // C'tors
    Teeprom() 
    {
        TEEPROM_NAMESPACE::verifyFields(M_fields); // Verify the fields
        EEPROM_CLASS.begin(Size); // Set the size of the EEPROM memory
    }

    Teeprom(const Teeprom&) = delete;
    Teeprom& operator=(const Teeprom&) = delete;
    ~Teeprom()
    {
        EEPROM_CLASS.end(); // End the EEPROM memory
    }


    /**
     * @brief Get the size of the EEPROM memory, specified by the user
     */
    static constexpr size_t eeprom_size() noexcept
    {
        return Size;
    }

    /**
     * @brief Get the total size of the tuple in bytes
     */
    inline size_t bytesize() noexcept
    {
        return TEEPROM_NAMESPACE::getTupleBytesize(M_fields);
    }

    /**
     * @brief Check if given template arguments can fit into specified EEPROM size
     * @return True if valid
     */
    inline bool valid(size_t startAddress = 0) noexcept
    {
        return (startAddress < Size) 
            && ((Size - startAddress) >= TEEPROM_NAMESPACE::getTupleBytesize(M_fields));
    }

    /**
     * @brief Loads the data from EEPROM, with a given start address
     * @return True if all data was read successfully, false otherwise (the data will be correct up to the point of failure)
     */
    inline bool load(size_t startAddress = 0) noexcept
    {
        return TEEPROM_NAMESPACE::readTupleFromEEPROM(M_fields, startAddress);
    }

    /**
     * @brief Commits the data to EEPROM, with a given start address
     * @return True if all data was saved successfully, false otherwise
     */
    inline bool save(size_t startAddress = 0) noexcept
    {
        return TEEPROM_NAMESPACE::putTupleToEEPROM(M_fields, startAddress) 
            && EEPROM_CLASS.commit();
    }

    /**
     * @brief Get the buffer pointer to the underlying EEPROM memory,
     * may not work on Arduino boards (returns nullptr), since data is not copied to a buffer
     */
    inline pointer data() noexcept
    {
        return EEPROM_CLASS.getDataPtr();
    }

    /**
     * @brief Get the EEPROM class instance
     * @return The EEPROM class instance
     */
    inline constexpr auto get_eeprom() noexcept 
    -> decltype(TEEPROM_NAMESPACE::refrence(
        EEPROM_CLASS))
    {
        return TEEPROM_NAMESPACE::refrence(
            EEPROM_CLASS);
    }


    /*
    
    Get the field structure from the tuple, with a given index.
    
    */
    template <size_t index>
    inline auto get_field() noexcept
    -> decltype(TEEPROM_NAMESPACE::refrence(
        TEEPROM_NAMESPACE::getTupleItem<index>(M_fields)))
    {
        return TEEPROM_NAMESPACE::refrence(
            TEEPROM_NAMESPACE::getTupleItem<index>(M_fields));
    }

    /*
    Get the data stored in the EEPROMField. 
    Use it, when the field should be interpreted as an array of values, or a string.

    ### Example

    ```

    // Create a reader with 512 bytes of EEPROM memory,
    // String field, an array of 20 uint8_t, and an array of 4 floats, and a single int
    Teeprom<512, EStr, EFs<uint8_t, 20>, EFs<float, 4>, EF<int>> reader;

    reader.get_data<0>() = "Hello, World!"; // Set the string field

    uint8_t* arr = reader.get_data<1>(); // Get the array of uint8_t, memory is already allocated
    arr[0] = 10; // Set the first element to 10

    float* farr = reader.get_data<2>(); // Get the array of floats
    farr[0] = 1.0; // Set the first element to 1.0

    int* i = reader.get_data<3>(); // Get the single int
    i[0] = 10; // Set the int to 10

    // i[1] is illegal, since the size of the int field is 1

    reader.save(); // Save the data to EEPROM

    // ...
    
    // Later in the code

    Teeprom<512, EStr, EFs<uint8_t, 20>, EFs<float, 4>, EF<int>> loader;
    loader.load(); // Load the data from EEPROM

    String str = loader.get_data<0>(); // Get the string field
    uint8_t* arr = loader.get_data<1>(); // Get the array of uint8_t
    float* farr = loader.get_data<2>(); // Get the array of floats
    int* i = loader.get_data<3>(); // Get the single int

    ```
     */
    template <size_t index>
    inline auto get_data() noexcept
    -> decltype(TEEPROM_NAMESPACE::refrence(
        TEEPROM_NAMESPACE::getTupleItem<index>(M_fields).data))
    {
        return TEEPROM_NAMESPACE::refrence(
            TEEPROM_NAMESPACE::getTupleItem<index>(M_fields).data);
    }

    /*
    Get the data point from the tuple, with a given index. 
    If the field is a string, or it should be interpreted as an array, call `get_data` instead of `get`.
    It's user's responsibility to make sure that the `value_index` is not out of bounds.

    ### Example

    ```

    // Create a reader with 512 bytes of EEPROM memory,
    // with an integer field, a string field, and an array of 20 chars
    Teeprom<512, EF<int>, EFs<char, 20>, EStr> reader;

    reader.get<0>() = 10;  // Set the integer field to 10, same as reader.get<0>(0)

    // This is illegal, and will throw out_of_range exception, because
    // the integer field's size is 1 (all values are stored as arrays, in this case an array of 1 int)
    // reader.get<0>(1) = 5;

    auto cstr = reader.get_data<1>(); // Get the string field, memory is already allocated (up to 20 chars)
    strcpy(cstr, "Hello, World!"); // Copy the string to the allocated memory

    // This won't work, since we are working on pointers
    // reader.get_data<1>() = "Hello, World!";

    // Set the string field to the allocated memory
    reader.get_data<2>() = "Arduino String"; // This is valid, since the String class has `operator=` overloaded


    // Also this is possible:
    int* arr = reader.get_data<0>();
    arr[0] = 10;

    reader.save(); // Save the data to EEPROM

    // ...

    // Later in the code

    Teeprom<512, EF<int>, EFs<char, 20>, EStr> loader;
    loader.load(); // Load the data from EEPROM

    Serial.println(loader.get<0>()); // Get the integer field
    Serial.println(loader.get_data<2>()) // Get the string field
    // ... etc

    ```
     */
    template <size_t Index>
    inline auto get(size_t value_index = 0) noexcept
    -> decltype(TEEPROM_NAMESPACE::refrence(
        TEEPROM_NAMESPACE::getTupleItem<Index>(M_fields).data[0])) 
    { 
        return TEEPROM_NAMESPACE::refrence(
            TEEPROM_NAMESPACE::getTupleItem<Index>(M_fields).data[value_index]);
    }
};