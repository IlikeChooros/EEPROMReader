#pragma once

#include "settings.hpp"
#include <EEPROM.h>

// Base class for EEPROM, to be used as a wrapper for different EEPROM libraries
class BaseEEPROMClass {
protected:
    int max_size = 0;
public:
    virtual void begin(int size) = 0;
    virtual bool commit() = 0;
    virtual int writeString(int index, const String &value) = 0;
    virtual int writeString(int index, const char *value) = 0;
    virtual String readString(int index) = 0;
};

#if defined(ARDUINO_EEPROM)

// For Arduino boards

// EEPROM class wrapper for Arduino EEPROM library, since it doesn't support writing strings.
class EEPROMArduinoClass: public EEPROMClass, public BaseEEPROMClass {
public:
    // Set the maximum size of the EEPROM
    void begin(int size) 
    {
        max_size = size;
        if (max_size > EEPROM.length())
            max_size = EEPROM.length();
    }

    // Commit the changes to the EEPROM, by default it will always return true
    bool commit() 
    {
        return true;
    }

    // Write a string to EEPROM, same as writeString(int, const char*)
    int writeString(int index, const String &value) 
    {
        return writeString(index, value.c_str());
    }

    // Write a string to EEPROM, up to the null terminator, or until max_size
    int writeString(int index, const char *value) 
    {
        int len = strlen(value);
        if (index + len + 1 > max_size) {
            // truncate the string if it doesn't fit
            len = max_size - index - 1;
            // Won't fit anyway
            if (len <= 0) 
                return 0;
        }

        for (int i = 0; i < len; i++) {
            update(index + i, value[i]);
        }
        update(index + len, '\0');
        return len;
    }

    // Read a string from EEPROM at given index
    String readString(int index)
    {
        int len = 0;
        for (int i = index; i < max_size; i++, len++)
            if (read(i) == '\0') // Get the length of the string
                break;
        
        // If the terminator is not found or the string is empty
        if (len == 0 || read(len) != '\0') 
            return String();

        // Read the string
        char buf[len + 1];
        for (int i = 0; i < len; i++)
            buf[i] = read(index + i);
        buf[len] = '\0';
        return String(buf);
    }
};

static EEPROMArduinoClass EEPROM_CLASS = EEPROMArduinoClass();
#endif

#if defined(ESP8266) || defined(ESP32)
static auto EEPROM_CLASS = EEPROM;
#endif