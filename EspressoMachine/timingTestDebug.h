#ifndef TIMING_TEST_DEBUG_H
#define TIMING_TEST_DEBUG_H

volatile static unsigned long TIME_START = 0;
volatile static unsigned long TIME_END = 0;
volatile static unsigned long TIME_LAST_PRINT = 0;
volatile static bool TIME_PRINT_ENABLED = true;

#define PRINT_TIME \
if ((millis() - TIME_LAST_PRINT >= 1000) && TIME_PRINT_ENABLED) { \
	TIME_LAST_PRINT = millis(); \
	Serial.print("last time:"); \
	Serial.println(TIME_END - TIME_START);}


/* usage:
TIME_START = millis();
TIME_END = millis();
TIME_PRINT_ENABLED = true;
PRINT_TIME
*/

#endif
