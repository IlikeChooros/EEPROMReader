#pragma once

#include <type_traits>
#include <tuple>
#include <EEPROM.h>


// EEROMFields stores an array of data in given type (or a single string)
template <typename T, size_t N>
struct EEPROMFields {
    T data[N];
    const int size = sizeof(data);
};

// EEPROMField stores a single data in given type
template <typename T>
struct EEPROMField : public EEPROMFields<T, 1> {};

// EEPROMFieldArray stores a 2D array of data in given type (used mainly for strings)
template <typename T, size_t N, size_t Len>
struct EEPROMFieldsArray {
    T data[N][Len];
    const int size = sizeof(data);
};

// EEPROMStrings stores an array of strings in EEPROM
template <size_t N, size_t Len>
struct EEPROMStrings : public EEPROMFieldsArray<char, N, Len> {};

// EEPROMString stores a single string in EEPROM (this is a C-style string)
struct EEPROMString {
    String data; // This is possible because of the operator[] overloading in the String class
    int size = 0;
};

// Shortcut for EEPROMString
using EStr = EEPROMString;

// Shortcut for EEPROMField
template <typename Field>
using EF = EEPROMField<Field>;

// Shortcut for EEPROMFields
template <typename Field, size_t N>
using EFs = EEPROMFields<Field, N>;

// Shortcut for EEPROMFieldsArray (2D array N x Len of data)
template <typename T, size_t N, size_t Len>
using EFArr = EEPROMFieldsArray<T, N, Len>;

/**
 * @brief Put a field to EEPROM
 * @tparam Field The field must have a `data` field
 */
template <typename Field>
inline void putToEEPROM(Field& field, size_t& address){
    EEPROM.put(address, field.data);
    address += sizeof(field.data);
}

// Specialization for strings
template <>
inline void putToEEPROM<EEPROMString>(EEPROMString& field, size_t& address){
    field.size = EEPROM.writeString(address, field.data.c_str());
    address += field.size + 1;
}


/**
 * @brief Check if the address is out of bounds
 * @tparam Tuple The tuple to be stored in EEPROM, each element of the tuple must have a `data` field
 * @tparam Index The index of the tuple
 */
template <typename Tuple, size_t Index>
bool checkAddressOutOfBounds(Tuple& tuple, size_t& address){
    return address + std::get<Index>(tuple).size >= EEPROM.length();
}

// Check if the index is out of bounds
template <typename Tuple, size_t Index>
constexpr bool checkIndexOutOfBounds(){
    return Index >= std::tuple_size<Tuple>::value;
}

/**
 * @brief Puts a tuple of data to EEPROM
 * @tparam Tuple The tuple to be stored in EEPROM, each element of the tuple must have a `data` field
 */
template <typename Tuple, size_t Index = 0>
bool putTupleToEEPROM(Tuple& tuple, size_t& address = 0){
    // If the index is less than the size of the tuple, put the data to EEPROM
    if constexpr (!checkIndexOutOfBounds<Tuple, Index>()){
        // If the address is out of bounds, return
        if (checkAddressOutOfBounds<Tuple, Index>(tuple, address)){
            return false;
        }
        putToEEPROM(std::get<Index>(tuple), address);
        return putTupleToEEPROM<Tuple, Index + 1>(tuple, address); // Recursively call itself
    }
    return true;
}

/**
 * @brief Read a field from EEPROM
 * @tparam Field The field must have `data` and `size` fields
 */
template <typename Field>
inline void readFromEEPROM(Field& field, size_t& address){
    EEPROM.get(address, field.data);
    address += sizeof(field.data);
}

// Specialization for strings
template <>
inline void readFromEEPROM<EEPROMString>(EEPROMString& field, size_t& address){
    field.data = EEPROM.readString(address);
    field.size = field.data.length();
    address += field.size + 1;
}


/**
 * @brief Reads a tuple of data from EEPROM
 * @tparam Tuple The tuple to be stored in EEPROM, each element of the tuple must have a `data` field
 * @return True if all data was read successfully, false otherwise
 */
template <typename Tuple, size_t Index = 0>
bool readTupleFromEEPROM(Tuple& tuple, size_t& address = 0){
    // If the index is less than the size of the tuple, put the data to EEPROM
    if constexpr (!checkIndexOutOfBounds<Tuple, Index>()){
        // If the address is out of bounds, return
        if (checkAddressOutOfBounds<Tuple, Index>(tuple, address)){
            return false;
        }
        readFromEEPROM(std::get<Index>(tuple), address);
        return readTupleFromEEPROM<Tuple, Index + 1>(tuple, address); // Recursively call itself
    }
    return true;
}

/**
 * @brief Provides memory storage for EEPROM, works similar to std::tuple
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
 * EEPROMReader<512, EEPROMField<int>, EEPROMString, EEPROMFields<float, 4>> reader;
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
 * EEPROMReader<512, EEPROMField<int>, EEPROMString, EEPROMFields<float, 4>> reader;
 * 
 * reader.load(); // Load the data from EEPROM
 * 
 * Serial.println(reader.get<0>());
 * 
 * Serial.println(reader.get<1>());
 * 
 * Serial.println(reader.get<2>(1));
 * 
 * ```
 */
template <size_t size, typename... Fields>
class EEPROMReader 
{
    std::tuple<Fields...> fields;
public:
    static int instance_count;

    // Constructor, initializes the EEPROM memory, calls EEPROM.begin(size)
    EEPROMReader() 
    {
        if (instance_count > 0)
        {
            const char* msg = "Only one instance of EEPROMReader is allowed at a time";
            Serial.println(msg);
            throw std::runtime_error(msg);
        }

        EEPROM.begin(size);
        instance_count++;
    }

    EEPROMReader(EEPROMReader&& other)
    {
        fields = std::move(other.fields);
        instance_count++;
    }

    EEPROMReader(const EEPROMReader&) = delete;
    EEPROMReader& operator=(const EEPROMReader&) = delete;

    ~EEPROMReader()
    {
        EEPROM.end();
        instance_count--;
    }

    /**
     * @brief Loads the data from EEPROM, with a given start address
     * @return True if all data was read successfully, false otherwise (the data was loaded up to some point)
     */
    inline bool load(size_t startAddress = 0) noexcept
    {
        return readTupleFromEEPROM(fields, startAddress);
    }

    /**
     * @brief Commits the data to EEPROM, with a given start address
     * @return True if all data was saved successfully, false otherwise
     */
    inline bool save(size_t startAddress = 0) noexcept
    {
        return putTupleToEEPROM(fields, startAddress) && EEPROM.commit();
    }


    /*
    
    Get the field structure from the tuple, with a given index.
    
    */
    template <size_t index>
    inline auto get_field()
    -> decltype(std::get<index>(fields))
    {
        static_assert(std::tuple_size<decltype(fields)>::value > index, "get_field(): Index out of bounds");
        return std::get<index>(fields);
    }

    /*
    Get the data stored in the EEPROMField. 
    Use it, when the field should be interpreted as an array of values, or a string.

    ### Example

    ```

    // Create a reader with 512 bytes of EEPROM memory,
    // String field, an array of 20 uint8_t, and an array of 4 floats, and a single int
    EEPROMReader<512, EStr, EFs<uint8_t, 20>, EFs<float, 4>, EF<int>> reader;

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

    EEPROMReader<512, EStr, EFs<uint8_t, 20>, EFs<float, 4>, EF<int>> loader;
    loader.load(); // Load the data from EEPROM

    String str = loader.get_data<0>(); // Get the string field
    uint8_t* arr = loader.get_data<1>(); // Get the array of uint8_t
    float* farr = loader.get_data<2>(); // Get the array of floats
    int* i = loader.get_data<3>(); // Get the single int

    ```
     */
    template <size_t index>
    inline auto get_data()
    -> decltype(std::forward<decltype(std::get<index>(fields).data)>(std::get<index>(fields).data))
    {
        return std::forward<decltype(std::get<index>(fields).data)>(std::get<index>(fields).data);
    }

    /*
    Get the data point from the tuple, with a given index. 
    If the field is a string, or it should be interpreted as an array, call `get_data` instead of `get`.

    ### Example

    ```

    // Create a reader with 512 bytes of EEPROM memory,
    // with an integer field, a string field, and an array of 20 chars
    EEPROMReader<512, EF<int>, EFs<char, 20>, EStr> reader;

    reader.get<0>() = 10;  // Set the integer field to 10, same as reader.get<0>(0)

    // This is illegal, and will throw out_of_range exception, because
    // the integer field's size is 1 (all values are stored as arrays, in this case an array of 1 int)
    // reader.get<0>(1) = 5;

    auto cstr = reader.get_data<1>(); // Get the string field, memory is already allocated (up to 20 chars)
    strcpy(cstr, "Hello, World!"); // Copy the string to the allocated memory

    // This won't work, since we are working on pointers
    // reader.get_data<1>() = "Hello, World!";

    // Set the string field to the allocated memory
    reader.get<2>() = "Arduino String"; // This is valid, since the String class has `operator=` overloaded


    // Also this is possible:
    int* arr = reader.get_data<0>();
    arr[0] = 10;

    reader.save(); // Save the data to EEPROM

    // ...

    // Later in the code

    EEPROMReader<512, EF<int>, EFs<char, 20>, EStr> loader;
    loader.load(); // Load the data from EEPROM

    Serial.println(loader.get<0>()); // Get the integer field
    Serial.println(loader.get_data<2>()) // Get the string field
    // ... etc

    ```
     */
    template <size_t index>
    inline auto get(size_t value_index = 0) 
    -> decltype(std::get<index>(fields).data[value_index]) 
    { 
        static_assert(std::tuple_size<decltype(fields)>::value > index, "get(value_index = 0): Index out of bounds");
        auto& field = std::get<index>(fields);
        if (value_index >= field.size)
        {
            throw std::out_of_range(
                "get(value_index): `value_index` out of bounds: " 
                + std::to_string(value_index) + " >= " 
                + std::to_string(field.size)
            );
        }
        return field.data[value_index];
    }
};

template <size_t size, typename... Fields>
int EEPROMReader<size, Fields...>::instance_count = 0;