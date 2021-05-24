#include "Sensors.h"
#include <Arduino.h>

// ==================== CONSTRUCTOR ====================
Sensors::Sensors(uint8_t pin, bool type, bool dir) {
	setType(type);
	_pin = pin;
	flags.type = type;
	flags.inv_state = dir;
}

// ==================== SET ====================
void Sensors::setType(bool type) {
	flags.type = type;
	for (uint8_t i = 7; i < 15; i++) {
		pinMode(i, INPUT_PULLUP);
	}
}
void Sensors::setDirection(bool dir) {
	flags.inv_state = dir;
}

// ==================== TICK ====================
bool Sensors::buttonRead() {
	uint8_t B = PINB & 0b00111111;
	return B > 0b00000000; //0b00111111;
}

void Sensors::tick() {
	// читаем состояние
	btn_state = !buttonRead() ^ (flags.inv_state ^ flags.type);
	// нажатие
	if (btn_state && !btn_flag) {
		btn_flag = true;			
		flags.isPress_f = true;
	}
	// отпускание
	if (!btn_state && btn_flag) {
		btn_flag = false;
	}
}

// ==================== IS ====================
boolean Sensors::isPress() {
	//if (flags.tickMode) Sensors::tick();
	if (flags.isPress_f) {
		flags.isPress_f = false;
		return true;
	} else return false;
}
