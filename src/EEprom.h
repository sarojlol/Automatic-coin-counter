#include <Arduino.h>

void eeprom_setup();
long EEPROMReadlong(long address);
void EEPROMWritelong(int address, long value);
void save_data(long baht_1, long baht_2, long baht_5, long baht_10, long total);
void reset_data();