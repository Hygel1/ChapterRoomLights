#include <stdio.h>
#include <stdlib.h>
#include "driver/rmt.h"
#include "esp_err.h"
#include "soc/rmt_struct.h" //gives rmt item datatype

#define stripLength 300
#define numStrips 10 //10 strips of 300 leds, strips are meant to be placed parallel from each other
#define standardBrightness 75 //this will likely be a value that I need to test and calibrate based on what I see from the lights, hopefully a somewhat central value rather than max

extern struct LightStrip *strips[numStrips]; //extern indicates that this exists somewhere so that including the .h file in main allows use

struct LightStrip{
        uint16_t length;
        uint8_t color[300][3]; //[LED_Num][R,G,B]
        uint32_t brightness[300];
        uint8_t outPin;
};

void init();
void stripFill(struct LightStrip *ledStrip, uint8_t *colors);
void stripAssign(struct LightStrip *ledStrip, uint8_t (*colors)[3]);
void brightnessFill(struct LightStrip *ledStrip, uint32_t brightnessVal);
void brightnessAssign(struct LightStrip *ledStrip, uint32_t *brightnessVals);
void acrossAll(struct LightStrip *ledStrip, uint8_t (*colorSet)[3], uint32_t *brightnessSet, uint8_t bpm);
void blackout(struct LightStrip *ledStrip);
//start lighting control functions
void rmtInit(uint8_t *gpio);
rmt_item32_t rmtTranslate(uint8_t bit);
void buildFrame(uint8_t (*colors)[3], rmt_item32_t *items);
void push(rmt_item32_t *items, uint8_t stripNum);