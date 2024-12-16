#ifndef EEPROM_H
#define EEPROM_H

// Адреса для записи в EEPROM
// Контрольные значения датчиков влажности почвы
#define addr_control_moisture1_value 0
#define addr_control_moisture2_value 4
#define addr_moisture1_air 8
#define addr_moisture1_water 12
#define addr_moisture2_air 16
#define addr_moisture2_water 20

// Настройки полива и вентиляции
#define addr_waterind_settings 24
#define addr_window_settings 48

//Функция: сохранение настроек в EEPROM
//Принимает: -
//Возвращает: -
void saveToEEPROM();

//Функция: загрузка настроек из EEPROM
//Принимает: -
//Возвращает: -
void loadFromEEPROM();

#endif //EEPROM_H
