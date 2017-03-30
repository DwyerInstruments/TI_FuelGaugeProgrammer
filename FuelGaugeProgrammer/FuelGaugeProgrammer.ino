/******************************************************************************
 *
 *  Project:      FuelGaugeProgrammer.ino
 *
 *  Author:       Adam Johnson
 *
 *  Description:  Programs firmware into a TI Lithium-ion fuel gauge.
 *                Status information is sent over serial port.
 *                The user must send 'f' to start programming.
 *
 *  Warning:      Be careful!  PROGRAMMING FUEL GAUGES CAN BRICK THEM
 *                PERMANENTLY.  During development, always have multiple targets
 *                on hand in case one is rendered non-functional.
 *
 *                If "bad firmware" is programmed into a gauge, it's likely gone
 *                for good.  But if the programmer stops at the first sign of
 *                trouble, then the gauge will likely be stuck in a programming
 *                mode, and there is a good chance you'll be able to recover the
 *                gauge by using TI's bqStudio and TI's EV2400 or similar
 *                programmer to reflash the firmware.  (This program stops at
 *                the first failed command to increase your chances of
 *                recovery.)
 *
 *  License:      This software is released under the MIT license (see below).
 *
 *****************************************************************************/
 
/******************************************************************************
 Copyright (c) 2017 Dwyer Instruments, Inc.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*******************************************************************************/

#include <Wire.h>            // I2C driver
#include "data.h"            // firmware to upload

#define MAX_LENGTH    110    // maximum length of a single command
#define BAUD_RATE     115200 // baud rate for serial output

// Put your setup code here, to run once:
void setup()
{
    // Set the LED pin as an output.
//  pinMode(RED_LED, OUTPUT);
    
    // Set up I2C library as master.
    Wire.begin();
    
    // Start serial library for output.
    Serial.begin(BAUD_RATE);
}

// Put your main code here, to run repeatedly:
void loop()
{
    // Inform the user that we'll wait before programming.
    Serial.println("Press 'f' to program fuel gauge...");
    
    // Wait until 'f' is received.
    char inByte = 0;
    while (inByte != 'f')
    {
        if (Serial.available())
            inByte = Serial.read();
    }
    
    // Program firmware.
    Program();
}

// Send firmware to fuel gauge over I2C.
void Program(void)
{
    uint32_t i = 0;         // counter
    uint8_t addr;           // I2C address
    uint16_t len;           // length of message
    bool isError = false;   // was there an error during programming?
    
    // Print a test message.
    Serial.println("Begin programming...");
    
    // Turn on the red LED when we start.
//  digitalWrite(RED_LED, HIGH);
    
    // Process all the programming instructions.
    // Stop if we encounter an error.
    while ((i < sizeof(data)) && (isError == false))
    {
        // Read the command.
        uint8_t cmd = data[i++];
        
        // Follow the command.
        switch(cmd)
        {
        // Delay for a certain number of milliseconds.
        case 'X':
        {
            // Read 2 bytes as a 16-bit delay [ms].
            uint16_t ms = ((uint16_t)data[i++] << 8) + data[i++];
            
            // Report what we're doing.
            String text = "Delay:   ";
            String message = text + ms + " ms ";
            Serial.println(message);
            
            // Wait for the specified length of time.
            delay(ms);
            
            break;
        }
        
        // Write to the fuel gauge.
        case 'W':
        {
            // Read I2C address (7-bit format).
            addr = data[i++] >> 1;
            
            // Read length.
            len = data[i++];
            
            // Report what we're doing.
            Serial.print("Writing:");
            int j;    // Don't mess with i.
            for (j = i; j < (i + len); j++)
            {
                Serial.print(" ");
                if (data[j] < 0x10)
                    Serial.print("0");
                Serial.print(String(data[j], HEX));
            }
            Serial.print("\r\n");
            
            // Send data via I2C.
            Wire.beginTransmission(addr);
            Wire.write(&(data[i]), len);
            Wire.endTransmission();
            
            // Increment to start of next message.
            i += len;
            break;
        }
        
        // Read from the fuel gauge and compare to stored data.
        case 'C':
        {
            uint8_t j = 0;
            uint8_t readData[MAX_LENGTH];
            
            // Read I2C address.
            addr = data[i++] >> 1;
            
            // Read length.
            len = data[i++];
            
            // Read the register address.
            uint8_t regAddr = data[i++];
            
            // Report what we're doing.
            Serial.print("Reading:");
            
            // Write the register address to the device.
            Wire.beginTransmission(addr);
            Wire.write(regAddr);
            // Send a I2C restart.
            Wire.endTransmission(false);
            // We've now used up one of our data bytes.
            len--;
            
            // Read data via I2C.
            Wire.requestFrom(addr, len);
            while (Wire.available())
            {
                // Read a byte.
                readData[j] = Wire.read();
                
                // Report what we've read.
                Serial.print(" ");
                if (readData[j] < 0x10)
                    Serial.print("0");
                Serial.print(String(readData[j], HEX));
                
                // Compare the byte.
                if (readData[j] != data[i + j])
                {
                    isError = true;
                    Serial.print("...fail.");
                    break;
                }
                
                // Increment to next byte.
                j++;
            }
            
            // Increment to start of next message.
            i += len;
            
            // Finish the serial line.
            Serial.print("\r\n");
            
            break;
        }
        
        default:
            break;
        }
    }
    
    // Turn off the red LED when we finish.
//  digitalWrite(RED_LED, LOW);
    
    if (isError)
        Serial.println("Programming FAILED.");
    else
        Serial.println("Programming PASSED.");
}
