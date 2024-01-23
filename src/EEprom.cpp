#include <Arduino.h>
#include <EEprom.h>
#include <Wire.h>
#include <I2C_eeprom.h>


I2C_eeprom ee(0x50, I2C_DEVICESIZE_24LC256);

void eeprom_setup()
{
    Wire.begin();
    ee.begin();
}

long EEPROMReadlong(long address) 
{
    long four = ee.readByte(address);
    long three = ee.readByte(address + 1);
    long two = ee.readByte(address + 2);
    long one = ee.readByte(address + 3);
    
    return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void EEPROMWritelong(int address, long value) 
{
    byte four = (value & 0xFF);
    byte three = ((value >> 8) & 0xFF);
    byte two = ((value >> 16) & 0xFF);
    byte one = ((value >> 24) & 0xFF);
    
    ee.writeByte(address, four);
    ee.writeByte(address + 1, three);
    ee.writeByte(address + 2, two);
    ee.writeByte(address + 3, one);
}

void save_data(long baht_1, long baht_2, long baht_5, long baht_10, long total)
{
    EEPROMWritelong(1, baht_1);
    EEPROMWritelong(10, baht_2);
    EEPROMWritelong(20, baht_5);
    EEPROMWritelong(30, baht_10);
    EEPROMWritelong(40, total);
}

void reset_data()
{
    for (int i = 0; i < 254; i++)
    {
        ee.writeByte(i, 0);
        delay(10);
    }

}