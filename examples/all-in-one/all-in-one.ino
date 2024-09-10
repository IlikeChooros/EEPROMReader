// Include the library
#include <EEPROMReader.h>

struct SomeData {
    // These must be standard types.
    int number;
    char string[20];
    float pi;

    // Can't put here, because the `String` can't be converted to
    // (uint8_t*) array, to be written to EEPROM.
    // String str;
};

void setup(){
    // Initialize the serial port
    // You may need to change the baud rate to match your device
    // For Arduino boards, the default baud rate is 9600
    // For ESP32, the default baud rate is 115200
    Serial.begin(115200);

    // Create a reader object with 128 bytes of EEPROM memory
    // With elements:
    // 1. int
    // 2. char[20]
    // 3. String
    // 4. int[2][3]
    //
    // EF<type>, is a single element with given type
    // EFs<type, size>, is an array of elements with given type and size
    // EStr is a single String element
    // EFArr<type, rows, cols> is a 2D array with given type, rows and cols
    // And that's all there is to it!
    EEPROMReader<128, EF<int>, EFs<char, 20>, EStr, EFArr<int, 2, 3>, EF<SomeData>> writer;

    // Set the integer value
    writer.get<0>() = 123;

    // We interpret this as an array of 20 characters, so that means we should
    // use `get_data` to modify the whole string. Calling `get(i)` will only return
    // character at index `i`
    strcpy(writer.get_data<1>(), "Hello, EEPROM!");

    // This is important, you can't use `get` here, 
    // if you want to modify the whole string
    writer.get_data<2>() = "Mystirng"; 
    
    // Set the first row of the 2x3 array
    writer.get<3>(0)[0] = 1;
    writer.get<3>(0)[1] = 2;
    writer.get<3>(0)[2] = 3;

    // This is also valid
    for(int i = 0; i < 3; i++){
        writer.get<3>(1)[i] = i + 4; // set 4, 5, 6
    }

    // Set the SomeData struct
    SomeData& data = writer.get<4>();
    
    data.number = 420;
    strcpy(data.string, "Hello, struct!");
    data.pi = 3.141592;

    // Commit the changes to EEPROM, starting from address 10
    writer.save(10);

    // Wait a bit, so that we can see the output message
    delay(500);
}

void loop(){
    // Load the data from EEPROM, the types must match the ones used in `setup`
    // In case of ESP32 Alocates 128 + sizeof(reader) bytes of memory
    // On Arduino boards uses only sizeof(reader) bytes of memory, since the EEPROM data isn't copied to an array.
    EEPROMReader<128, EF<int>, EFs<char, 20>, EStr, EFArr<int, 2, 3>, EF<SomeData>> reader;

    // Load the data from EEPROM, starting from address 10
    reader.load(10);

    // Print the data
    Serial.println(reader.get<0>());
    Serial.println(reader.get_data<1>());
    Serial.println(reader.get_data<2>());

    // Print the 2x3 array
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 3; j++){
            Serial.print(reader.get<3>(i)[j]);
            Serial.print(" ");
        }
        Serial.println();
    }

    // Print the data from the struct
    SomeData& data = reader.get<4>();
    Serial.println("SomeData:");
    Serial.println(data.number);
    Serial.println(data.string);
    Serial.println(data.pi, 6);

    while(1){
        yield();
    }
}

