# EEPROMReader

Arduino EEPROM reader / writer, with type templating for easy data management.

## Features

- **Type Templating**: Store and retrieve any data type using C++ templates.
- **Easy to Use**: Simple API for reading and writing data.
- **Board support**: Right now library supports: ESP32, ESP8266 and all Arduino boards.

## Overview

The EEPROMReader library is designed to simplify reading and writing to EEPROM on Arduino devices.  
By using C++ type templating, you can store and retrieve various types of data with ease.   
Tested on ESP32 and Arduino UNO.

```cpp
#include <EEPROMReader.h>

struct MyData{
    int number;
    char string[20];
};

void write(){
    // Create a 'writer' with 128 bytes of EEPROM memory,
    // with an integer field, a string field, an array of 20 chars and 'MyData' struct
    EEPROMReader<128, EF<int>, EFs<char, 20>, EStr, EF<MyData>> writer;
    writer.get<0>() = 123; // Set the integer field to 123
    
    // This is an char array, so we need to use strcpy to set the value
    strcpy(writer.get_data<1>(), "Hello, CWorld!");

    // This is a string, so we can set the value directly, 
    // note that `get_data` is needed to set the value to the whole string.
    writer.get_data<2>() = "Hello, EEPROM!";

    // Set `MyData` struct
    writer.get<3>().number = 69;
    strcpy(writer.get<3>().string, "MyData string!");

    // Save the data to EEPROM
    writer.save();
}

void load(){
    // Create a reader with the same fields as the `writer`
    EEPROMReader<128, EF<int>, EFs<char, 20>, EStr, EF<MyData>> reader;
    reader.load(); // Load the data from EEPROM

    // Get the values from the reader
    Serial.println(reader.get<0>()); // 123
    Serial.println(reader.get_data<1>()); // "Hello, CWorld!"
    Serial.println(reader.get_data<2>()); // "Hello, EEPROM!"

    MyData& data = reader.get<3>();
    Serial.println("MyData:");
    Serial.println(data.number); // 69
    Serial.println(data.string); // "MyData string!"
}

void setup(){
    Serial.begin(9600);
    write();
    load();
}

void loop(){
    // Do nothing
}

```

## License

This library is licensed under the MIT License. See the LICENSE file for more details.

