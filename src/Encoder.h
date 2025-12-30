/* Encoder Library, for measuring quadrature encoded signals
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 * Copyright (c) 2011,2013 PJRC.COM, LLC - Paul Stoffregen <paul@pjrc.com>
 *
 * Version 1.2 - fix -2 bug in C-only code
 * Version 1.1 - expand to support boards with up to 60 interrupts
 * Version 1.0 - initial release
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef Encoder_h_
#define Encoder_h_

#include "Arduino.h"

#include "pinDefinitions.h"

// All the data needed by interrupts is consolidated into this ugly struct
// to facilitate assembly language optimizing of the speed critical update.
// The assembly code uses auto-incrementing addressing modes, so the struct
// must remain in exactly this order.
typedef struct
{
	uint8_t state;
	uint8_t pin1, pin2;
	int32_t position;
} Encoder_internal_state_t;




class Encoder
{
public:
	Encoder(pin_size_t _pin1, pin_size_t _pin2)
	{
		encoderState.pin1 = _pin1;
		encoderState.pin2 = _pin2;
		pinMode(_pin1, INPUT);
		digitalWrite(_pin1, HIGH);
		pinMode(_pin2, INPUT);
		digitalWrite(_pin2, HIGH);
#endif
		encoderState.position = 0;
		// allow time for a passive R-C filter to charge
		// through the pullup resistors, before reading
		// the initial state
		delayMicroseconds(2000);
		uint8_t s = 0;
		if (digitalRead(_pin1))
			s |= 1;
		if (digitalRead(_pin2))
			s |= 2;
		encoderState.state = s;
		attachInterruptParam(_pin1, isrUpdate, CHANGE, &encoderState);
		attachInterruptParam(_pin2, isrUpdate, CHANGE, &encoderState);
		// update_finishup();  // to force linker to include the code (does not work)
	}


	inline int32_t read()
	{
		noInterrupts();
		int32_t ret = encoderState.position;
		interrupts();
		return ret;
	}
	inline int32_t readAndReset()
	{
		noInterrupts();
		int32_t ret = encoderState.position;
		encoderState.position = 0;
		interrupts();
		return ret;
	}
	inline void write(int32_t p)
	{
		noInterrupts();
		encoderState.position = p;
		interrupts();
	}

private:
	Encoder_internal_state_t encoderState;

public:
	// update() is not meant to be called from outside Encoder,
	// but it is public to allow static interrupt routines.
	// DO NOT call update() directly from sketches.

	static void isrUpdate(void* arg) {
		update(static_cast<Encoder_internal_state_t*>(arg));
	}

	static void update(Encoder_internal_state_t *arg)
	{
		uint8_t p1val = digitalRead(arg->pin1);
		uint8_t p2val = digitalRead(arg->pin2);
		uint8_t state = arg->state & 3;
		if (p1val)
			state |= 4;
		if (p2val)
			state |= 8;
		arg->state = (state >> 2);
		switch (state)
		{
		case 1:
		case 7:
		case 8:
		case 14:
			arg->position++;
			return;
		case 2:
		case 4:
		case 11:
		case 13:
			arg->position--;
			return;
		case 3:
		case 12:
			arg->position += 2;
			return;
		case 6:
		case 9:
			arg->position -= 2;
			return;
		}
	}
};
