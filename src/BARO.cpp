/*
  This file is part of the Arduino_LPS22HB library.
  Copyright (c) 2019 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Wire.h>

#include "BARO.h"

#define LPS22HB_ADDRESS  0x5C

#define LPS22HB_WHO_AM_I_REG        0x0f
#define LPS22HB_CTRL2_REG           0x11
#define LPS22HB_STATUS_REG          0x27
#define LPS22HB_PRESS_OUT_XL_REG    0x28
#define LPS22HB_PRESS_OUT_L_REG     0x29
#define LPS22HB_PRESS_OUT_H_REG     0x2a

#define LPS22HB_TEMP_OUT_L_REG      0x2b
#define LPS22HB_TEMP_OUT_H_REG      0x2c

LPS22HBClass::LPS22HBClass(TwoWire& wire) :
  _wire(&wire)
{
}

int LPS22HBClass::begin()
{
  _wire->begin();

  if (i2cRead(LPS22HB_WHO_AM_I_REG) != 0xb1) {
    end();
    return 0;
  }

  return 1;
}

void LPS22HBClass::end()
{
  _wire->end();
}

float LPS22HBClass::readPressure(P_UNIT unit)
{
  // trigger one shot
  i2cWrite(LPS22HB_CTRL2_REG, 0x01);

  // wait for completion
  while ((i2cRead(LPS22HB_STATUS_REG) & 0x02) == 0) {
    yield();
  }

  float reading = (i2cRead(LPS22HB_PRESS_OUT_XL_REG) |
          (i2cRead(LPS22HB_PRESS_OUT_L_REG) << 8) | 
          (i2cRead(LPS22HB_PRESS_OUT_H_REG) << 16)) / 4096.0;

  if (unit == PSI) {  			// 1 hPa = 0.0145038 PSI
    return reading * 0.0145038;
  } else if (unit == ATM) {  	// 1 atm = 1013.25 hPa
    return reading / 1013.25;
  } else {
    return reading;
  }
}

float LPS22HBClass::readTemperature(T_UNIT unit)
{
  // trigger one shot
  i2cWrite(LPS22HB_CTRL2_REG, 0x01);

  // wait for completion
  while ((i2cRead(LPS22HB_STATUS_REG) & 0x02) == 0) {
    yield();
  }

  float reading = (i2cRead(LPS22HB_TEMP_OUT_L_REG) |
	          (i2cRead(LPS22HB_TEMP_OUT_H_REG) << 8)) / 100.0;

  if (unit == F) { 	// 1 Fahrenheit = (1 Celsius * 9 / 5) + 32
    return (reading * 9.0 / 5.0) + 32.0;
  } else {
    return reading;
  }
}

int LPS22HBClass::i2cRead(uint8_t reg)
{
  _wire->beginTransmission(LPS22HB_ADDRESS);
  _wire->write(reg);
  if (_wire->endTransmission(false) != 0) {
    return -1;
  }

  if (_wire->requestFrom(LPS22HB_ADDRESS, 1) != 1) {
    return -1;
  }
  
  return _wire->read();
}

int LPS22HBClass::i2cWrite(uint8_t reg, uint8_t val)
{
  _wire->beginTransmission(LPS22HB_ADDRESS);
  _wire->write(reg);
  _wire->write(val);
  if (_wire->endTransmission() != 0) {
    return 0;
  }

  return 1;
}

#ifdef ARDUINO_ARDUINO_NANO33BLE
LPS22HBClass BARO(Wire1);
#else
LPS22HBClass BARO(Wire);
#endif
