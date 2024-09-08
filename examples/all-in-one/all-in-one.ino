// Include the library
#include <EEPROMReader.h>


void setup(){
    // Initialize the serial port
    Serial.begin(115200);

    // Create a reader object with 512 bytes of EEPROM memory
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
    EEPROMReader<128, EF<int>, EFs<char, 20>, EStr, EFArr<int, 2, 3>> writer;
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

    // Commit the changes to EEPROM
    writer.save(10);
}

void loop(){
    // Load the data from EEPROM, the types must match the ones used in `setup`
    // Alocates 128 + 88 bytes of memory = 216 bytes, generally it uses `size` + 88 bytes of memory.
    EEPROMReader<128, EF<int>, EFs<char, 20>, EStr, EFArr<int, 2, 3>> reader;

    // Load the data from EEPROM
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

    while(1){
        yield();
    }
}

