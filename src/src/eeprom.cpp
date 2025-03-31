#include "eeprom.hpp"


BEGIN_DETAIL_TEEPROM_NAMESPACE

#if defined(ARDUINO_EEPROM)

int EEPROMArduinoClass::writeString(int index, const char *value) noexcept
{
    int len = strlen(value);
    if (index + len + 1 > M_max_size) {
        // truncate the string if it doesn't fit
        len = M_max_size - index - 1;
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

String EEPROMArduinoClass::readString(int index) noexcept
{
    int len = 0, i = index;
    for (; i < M_max_size; i++, len++)
        if (read(i) == '\0') // Get the length of the string
            break;

    // If the terminator is not found or the string is empty
    if (len == 0 || read(i) != '\0') 
        return String();

    // Read the string
    char buf[len + 1];
    for (i = 0; i < len; i++)
        buf[i] = read(index + i);
    buf[len] = '\0';
    return String(buf);
}

EEPROMArduinoClass EEPROM_CLASS = EEPROMArduinoClass();

#endif

END_DETAIL_TEEPROM_NAMESPACE


#if defined(ESP8266) || defined(ESP32)
EEPROMESPClass EEPROM_CLASS = EEPROMESPClass();
#endif

