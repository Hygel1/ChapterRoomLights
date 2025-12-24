#include <stdio.h>
#include <stdlib.h>
#define stripLength 300
#define numStrips 10 //10 strips of 300 leds, strips are meant to be placed parallel from each other
#define standardBrightness 75 //this will likely be a value that I need to test and calibrate based on what I see from the lights, hopefully a somewhat central value rather than max

struct LightStrip{
        uint16_t length;
        uint8_t color[300][3]; //[LED_Num][R,G,B]
        uint32_t brightness[300];
        uint8_t outPin;
};

void init();
void stripFill(struct LightStrip *ledStrip, uint8_t colors[3]);
void stripAssign(struct LightStrip *ledStrip, uint8_t colors[stripLength][3]);
void brightnessFill(struct LightStrip *ledStrip, uint32_t brightnessVal);
void brightnessAssign(struct LightStrip *ledStrip, uint32_t brightnessVals[stripLength]);
void acrossAll(struct LightStrip *ledStrip[numStrips], uint8_t colorSet[stripLength][3], uint32_t brightnessSet[stripLength], uint8_t bpm);
void blackout(struct LightStrip *ledStrip[numStrips]);
//start lighting control functions
void rmtInit(uint8_t *gpio[numStrips]);
rmt_item32_t rmtTranslate(uint8_t bit);
void buildFrame(uint8_t *colors[stripLength][3], rmt_item32_t *items[stripLength*24]);
void push(rmt_item32_t *items[stripLength*24]);