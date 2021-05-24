#pragma once
#include <Arduino.h>

#pragma pack(push,1)
typedef struct {		
	bool isPress_f: 1;
	bool inv_state: 1;
	bool type: 1;
} SensorsFlags;
#pragma pack(pop)

#define HIGH_PULL 0
#define LOW_PULL 1
#define NORM_OPEN 0
#define NORM_CLOSE 1

class Sensors {
  public:
    Sensors(uint8_t pin, bool type = HIGH_PULL, bool dir = NORM_OPEN);

    void setType(bool type);			// установка типа кнопки (HIGH_PULL - подтянута к питанию, LOW_PULL - к gnd)	
	void setDirection(bool dir);		// установка направления (разомкнута/замкнута по умолчанию - NORM_OPEN, NORM_CLOSE)	
    void tick();						// опрос сенсоров

	boolean isPress();		// возвращает true при нажатии на кнопку. Сбрасывается после вызова
	boolean isRelease();	// возвращает true при отпускании кнопки. Сбрасывается после вызова
	boolean isClick();		// возвращает true при клике. Сбрасывается после вызова
	boolean isHold();		// возвращает true при нажатой кнопке, не сбрасывается

  private:
	bool buttonRead();
	uint8_t _pin;
    SensorsFlags flags;
	bool btn_state = false;
	bool btn_flag = false;
};
