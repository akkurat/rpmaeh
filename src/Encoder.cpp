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

//   F ->                     I  F1  F2  F3 (+) I
//                           _______         _______
//               Pin1 ______|       |_______|       |______ Pin1
// negative <---         _______         _______         __      --> positive
//               Pin2 __|       |_______|       |_______|   Pin2
//   B ->                   I (-) B3  B2  B1  I

typedef struct
{
	uint8_t pin1, pin2;

	uint8_t v1 = 1, v2 = 1;
	int8_t direction = 0;
	// direction and transitions could be one variable
	// substep counting but with initial threshold would be ideal
	uint8_t successfulTransitions = 0;

	int32_t position = 0;

	int32_t interruptCount = 0;

} Encoder_internal_state_t;

class Encoder
{
public:
	Encoder(pin_size_t _pin1, pin_size_t _pin2)
	{
		encoderState->pin1 = _pin1;
		encoderState->pin2 = _pin2;
		pinMode(_pin1, INPUT);
		digitalWrite(_pin1, HIGH);
		pinMode(_pin2, INPUT);
		digitalWrite(_pin2, HIGH);
#endif
		// allow time for a passive R-C filter to charge
		// through the pullup resistors, before reading
		// the initial state
		delayMicroseconds(2000);
		attachInterruptParam(_pin1, isrUpdate, CHANGE, encoderState);
		attachInterruptParam(_pin2, isrUpdate, CHANGE, encoderState);
	}

	inline int32_t read()
	{
		update(encoderState);
		int32_t ret = encoderState->position;
		return ret;
	}
	inline int32_t readAndReset()
	{
		noInterrupts();
		update(encoderState);
		int32_t ret = encoderState->position;
		encoderState->position = 0;
		interrupts();
		return ret;
	}
	inline void write(int32_t p)
	{
		update(encoderState);
		encoderState->position = p;
	}

	Encoder_internal_state_t getState()
	{
		return *encoderState;
	}

private:
	Encoder_internal_state_t *encoderState = new Encoder_internal_state_t();

public:
	// update() is not meant to be called from outside Encoder,
	// but it is public to allow static interrupt routines.
	// DO NOT call update() directly from sketches.

	static void isrUpdate(void *arg)
	{
		auto carg = static_cast<Encoder_internal_state_t *>(arg);
		carg->interruptCount++;
		update(carg);
	}

	// Starting Encoding is 1 1 (at least for JoyIt Encoder, )
	static void update(Encoder_internal_state_t *arg)
	{
		uint8_t w1 = digitalRead(arg->pin1);
		uint8_t w2 = digitalRead(arg->pin2);

		if(w1 == arg->v1 && w2 == arg->v2) {
			return;
		}

		if (arg->direction == 0)
		{
			arg->direction = getDirectionFromTransition(arg->v1, arg->v2, w1, w2);
		}
		else
		{
			auto _direction = getDirectionFromTransition(arg->v1, arg->v2, w1, w2);
			if (_direction == arg->direction)
			{
				arg->successfulTransitions++;
				if (arg->successfulTransitions >= 6)
				{
					arg->position += _direction;
					arg->successfulTransitions = 2;
					// keep direction
				}
			} else if( _direction == -arg->direction) {
				if(arg->successfulTransitions > 0) {
					arg->successfulTransitions-- ;
				} else {
					arg->direction = _direction;
				}
			}
		}

		arg->v1 = w1;
		arg->v2 = w2;
	}

private:
	//   F ->                     I  F1  F2  F3 (+) I
	//                           _______         _______
	//               Pin1 ______|       |_______|       |______ Pin1
	// negative <---         _______         _______         __      --> positive
	//               Pin2 __|       |_______|       |_______|   Pin2
	//   B ->                   I (-) B3  B2  B1  I

	static int8_t getDirectionFromTransition(uint8_t v1, uint8_t v2, uint8_t w1, uint8_t w2)
	{
		if (v1 == 0 && v2 == 0)
		{
			if (w1 == 0 && w2 == 1)
				return 1;
			if (w1 == 1 && w2 == 0)
				return -1;
			return 0;
		}
		else if (v1 == 1 && v2 == 1)
		{
			if (w1 == 1 && w2 == 0)
				return 1;
			if (w1 == 0 && w2 == 1)
				return -1;
			return 0;
		}
		else if (v1 == 1 && v2 == 0)
		{
			if (w1 == 0 && w2 == 0)
				return 1;
			if (w1 == 1 && w2 == 1)
				return -1;
			return 0;
		}
		else if (v1 == 0 && v2 == 1)
		{
			if (w1 == 1 && w2 == 1)
				return 1;
			if (w1 == 0 && w2 == 0)
				return -1;
			return 0;
		}
	}
};
