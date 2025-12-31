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

enum class EncoderStates
{
	INIT = 0,
	F1 = 11,
	F2 = 12,
	F3 = 13,
	B1 = 21,
	B2 = 22,
	B3 = 23
};

typedef struct
{
	EncoderStates state = EncoderStates::INIT;
	uint8_t pin1, pin2;
	int32_t position = 0;
	int32_t interruptCount = 0;
	int32_t stateBackCount = 0;
	int32_t stateStayCount = 0;
	unsigned long lastCall = 0;

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

	//   F ->                     I  F1  F2  F3 (+) I
	//                           _______         _______
	//               Pin1 ______|       |_______|       |______ Pin1
	// negative <---         _______         _______         __      --> positive
	//               Pin2 __|       |_______|       |_______|   Pin2
	//   B ->                   I (-) B3  B2  B1  I

	// Starting Encoding is 1 1 (at least for JoyIt Encoder, )
	static void update(Encoder_internal_state_t *arg)
	{
		uint8_t p1val = digitalRead(arg->pin1);
		uint8_t p2val = digitalRead(arg->pin2);

		auto setInitState = [arg]()
		{
			arg->state = EncoderStates::INIT;
		};

		switch (arg->state)
		{
		case EncoderStates::INIT:
			if (p1val == 1 && p2val == 0)
			{
				arg->state = EncoderStates::F1;
			}
			else if (p1val == 0 && p2val == 1)
			{
				arg->state = EncoderStates::B1;
			}
			// keep init state
			break;

		case EncoderStates::F1:
			if (p1val == 1 && p2val == 0)
			{
				arg->stateStayCount++;
			}
			else if (p1val == 0 && p2val == 0)
			{
				arg->state = EncoderStates::F2;
			}
			else
			{
				arg->stateBackCount++;
				setInitState();
			}
			break;

		case EncoderStates::F2:
			if (p1val == 0 && p2val == 0)
			{
				arg->stateStayCount++;
			}
			else if (p1val == 1 && p2val == 0)
			{
				arg->stateBackCount++;
				arg->state = EncoderStates::F1;
			}
			else if (p1val == 0 && p2val == 1)
			{
				arg->state = EncoderStates::F3;
			}
			else
			{
				setInitState();
			}
			break;

		case EncoderStates::F3:
			if (p1val == 0 && p2val == 1)
			{
				arg->stateStayCount++;
			}
			else if (p1val == 0 && p2val == 0)
			{
				arg->stateBackCount++;
				arg->state = EncoderStates::F2;
			}
			else if (p1val == 1 && p2val == 1)
			{
				arg->position++;
				setInitState();
			}
			else
			{
				setInitState();
			}
			break;

		case EncoderStates::B1:;
			if (p1val == 0 && p2val == 1)
			{
				arg->stateStayCount++;
			}
			else if (p1val == 0 && p2val == 0)
			{
				arg->state = EncoderStates::B2;
			}
			else
			{
				setInitState();
			}
			break;

		case EncoderStates::B2:;
			if (p1val == 0 && p2val == 0)
			{
				arg->stateStayCount++;
			}
			else if (p1val == 1 && p2val == 0)
			{
				arg->state = EncoderStates::B3;
			}
			else
			{
				setInitState();
			}
			break;
		case EncoderStates::B3:
			if (p1val == 1 && p2val == 0)
			{
				arg->stateStayCount++;
			}
			else if (p1val == 1 && p2val == 1)
			{
				arg->position--;
			}
			setInitState();
			break;

		// other transitions are glitches (staying at Init State)
		// fall trhough here
		default:
			setInitState();
			break;
		}
		arg->lastCall = millis();
	}
};
