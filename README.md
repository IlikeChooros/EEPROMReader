# EEPROMReader

Arduino EEPROM reader, with type templating for easy data reading.

## Overview

The EEPROMReader library is designed to simplify reading and writing to EEPROM on Arduino devices. By using C++ type templating, you can store and retrieve various types of data with ease.

## Features

- **Type Templating**: Store and retrieve any data type using C++ templates.
- **Easy to Use**: Simple API for reading and writing data.
- **Memory Management**: Automatically handles memory allocation for strings and arrays.
- **Error Handling**: Provides error checking and handling for out-of-bounds access.

## Installation

1. Download the library from the [GitHub repository](https://github.com/IlikeChooros/EEPROMReader).
2. Extract the downloaded ZIP file.
3. Move the extracted folder to your Arduino libraries directory (usually `~/Documents/Arduino/libraries`).

## Overview

```cpp
#include <EEPROMReader.h>

// Create a reader with 512 bytes of EEPROM memory,
// with an integer field, a string field, and an array of 20 chars
EEPROMReader<512, EF<int>, EFs<char, 20>, EStr> reader;

void write(){
    reader.get<0>() = 123; // Set the integer field to 123
    
    // This is an char array, so we need to use strcpy to set the value
    strcpy(reader.get_data<1>(), "Hello, CWorld!");

    // This is a string, so we can set the value directly
    reader.get<2>() = "Hello, World!";

    // Save the data to EEPROM
    reader.save();
}

void load(){
    reader.load(); // Load the data from EEPROM

    // Get the values from the reader
    Serial.println(reader.get<0>()); // Should print 123
    Serial.println(reader.get_data<1>()); // Should print "Hello, CWorld!"
    Serial.println(reader.get<2>()); // Should print "Hello, World!"
}

```

## License

This library is licensed under the MIT License. See the LICENSE file for more details.

