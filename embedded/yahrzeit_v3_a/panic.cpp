// Panic.cpp
#include "panic.h"
// #include <Modulino.h>

[[noreturn]] void panic(const char *expr, const char *file, int line)
{
    Serial.println();
    Serial.println("PANIC");
    Serial.print("ASSERT: ");
    Serial.println(expr);
    Serial.print("FILE: ");
    Serial.println(file);
    Serial.print("LINE: ");
    Serial.println(line);

    // // Make all 8 Modulino pixels red.
    // statusPixels.clear();
    // for (int i = 0; i < 8; ++i) {
    //     statusPixels.set(i, 32, 0, 0);   // modest red, not retina-searing
    // }
    // statusPixels.show();

    pinMode(LED_BUILTIN, OUTPUT);

    // Stay here forever, blinking built-in LED as secondary indication.
    while (true) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(150);
        digitalWrite(LED_BUILTIN, LOW);
        delay(150);
    }
}