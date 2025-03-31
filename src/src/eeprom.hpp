#pragma once

#include "settings.hpp"

// All EEPROM classes must have implemented the following methods:
// - begin(int size)
// - commit()
// - writeString(int index, const String &value)
// - writeString(int index, const char *value)
// - readString(int index)
// - get_eeprom_size()
// - template<typename T> T& get(int index)
// - template<typename T> void put(int index, const T& value)

// BEGIN_DETAIL_TEEPROM_NAMESPACE

#if defined(ARDUINO_EEPROM)

#include <EEPROM.h>

// EEPROM class wrapper for Arduino EEPROM library, since it doesn't support writing strings.
class EEPROMArduinoClass: public EEPROMClass {

    int M_max_size = 0;
public:

    // Get the maximum size of the EEPROM
    inline int get_eeprom_size() noexcept { return M_max_size; }

    // Set the maximum size of the EEPROM
    void begin(int size) noexcept
    {
        M_max_size = size;
        if (M_max_size > EEPROM.length())
            M_max_size = EEPROM.length();
    }

    // Commit the changes to the EEPROM, by default it will always return true
    bool commit() noexcept { return true; }

    // Get buffer pointer to the EEPROM memory (for compatibility with other EEPROM classes)
    uint8_t* getDataPtr() noexcept { return nullptr; }

    // Write a string to EEPROM, same as writeString(int, const char*)
    int writeString(int index, const String &value) noexcept 
    {
        return writeString(index, value.c_str());
    }

    // Write a string to EEPROM, up to the null terminator, or until max size is reached
    int writeString(int index, const char *value) noexcept;

    // Read a string from EEPROM at given index, may return empty string if no
    // string is found or the string is empty
    String readString(int index) noexcept;
};

extern EEPROMArduinoClass EEPROM_CLASS;
#endif

// EPS32 and ESP8266 EEPROM class
#if defined(ESP8266) || defined(ESP32)

#define NO_GLOBAL_EEPROM
#include <EEPROM.h>

// For ESP8266 and ESP32
class EEPROMESPClass: public EEPROMClass {
public:
    inline int get_eeprom_size() 
    {
        return this->length();
    }
};


// auto EEPROM_CLASS = EEPROMESPClass(); // For ESP32
extern EEPROMESPClass EEPROM_CLASS;
#endif

// END_DETAIL_TEEPROM_NAMESPACE