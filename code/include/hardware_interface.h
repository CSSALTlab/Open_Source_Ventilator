#pragma once
#include <stdint.h>
#include <stdbool.h>

//prototype the hardware interface functions here

void initialize_hardware();

void uart_print(char* str);

void lcd_print(uint8_t row, uint8_t col, const char* string);

void sound_alarm();

void silence_alarm();

void delay_ms(uint32_t ms);
