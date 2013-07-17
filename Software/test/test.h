#ifndef __TEST_H__
#define __TEST_H__

#ifdef ARDUINO

void test(void)
{
	Serial.println("from test.h);
}

#else
#error This example is only for use on Arduino.
#endif // ARDUINO

#endif // __TEST_H__