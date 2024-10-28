#ifndef GLOBALS_H
#define GLOBALS_H
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EncButton.h>

//------------АДРЕСА I2C УСТРОЙСТВ------------
#define MY_LCD2004_ADDRESS 0x27

//----------- ОБЪЕКТЫ И ПЕРЕМЕННЫЕ -----------
extern LiquidCrystal_I2C lcd;
extern EncButton eb;

extern int pos;
extern bool in_menu;
extern int current_menu;
extern byte custom_wind[];
extern byte custom_sun[];
extern byte grad[];

extern long interval;
extern unsigned long previous_millis;

extern int control_moisture1_value;
extern int control_moisture2_value;
















#endif //GLOBALS_H
