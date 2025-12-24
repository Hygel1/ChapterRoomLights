#include <stdio.h>
#include <stdlib.h>
#include "LightControl.h"
#include "driver/rmt.h"
#include "esp_err.h"
#define stripLength 300
#define numStrips 10 //10 strips of 300 leds, strips are meant to be placed parallel from each other
#define standardBrightness 75 //this will likely be a value that I need to test and calibrate based on what I see from the lights, hopefully a somewhat central value rather than max

struct LightStrip *strips[numStrips];
uint8_t outPins[numStrips];
//uint16_t stripLength=300;
//uint8_t numStrips=100;

void init(){
    for(uint16_t i=0;i<10;i++){
        strips[i]->length=stripLength;
        for(int q=0;q<stripLength;q++) strips[i]->brightness[q]=0xFFFF/2; //set all brightnesses to half
    }
}
void stripInit(struct LightStrip *ledStrip){
    ledStrip->length=stripLength;
    for(uint16_t i=0;i<stripLength;i++) ledStrip->brightness[i]=standardBrightness;
}
//takes a single color value and assigns to all leds
void stripFill(struct LightStrip *ledStrip, uint8_t colors[3]){
    for(uint16_t i=0;i<ledStrip->length;i++) for(int k=0;k<3;k++) ledStrip->color[i][k]=(uint8_t)colors[k];
}
//takes array of color values to be assigned to leds and outputs to strip
void stripAssign(struct LightStrip *ledStrip, uint8_t colors[stripLength][3]){
    for(uint16_t i=0;i<ledStrip->length;i++) for(int k=0;k<3;k++) (ledStrip->color[i][k])=(uint8_t)(colors[i][k]);
}
//takes a single brightness value and assigns to entire strip
void brightnessFill(struct LightStrip *ledStrip, uint32_t brightnessVal){
    for(uint16_t i=0;i<ledStrip->length;i++) (ledStrip->brightness[i])= (uint8_t)(brightnessVal);
}
//takes an array of brightness values and assigns to corresponding leds on strip
void brightnessAssign(struct LightStrip *ledStrip, uint32_t brightnessVals[stripLength]){
    for(uint16_t i=0;i<ledStrip->length;i++) (ledStrip->brightness[i])=(uint32_t)(brightnessVals[i]);
}
/**
 * @brief 'throws' light across room, aligned across all strips in timing, color, and brightness
 * 
 * 
 * @param ledStrip array of ledStrip structs to be set
 * @param color array of colors to be set to LEDs, should generally be uniform
 * @param brightness array of brightnesses to be set to each led row, should generally be uniform
 * @param bpm determines how fast the lights should move, used to define the delay time between loop iterations
 */
void acrossAll(struct LightStrip *ledStrip[numStrips], uint8_t colorSet[stripLength][3], uint32_t brightnessSet[stripLength], uint8_t bpm){
    blackout(ledStrip);
    uint32_t delay = (uint32_t)((1/((double)bpm)*60)/310*1000); 
    /*strip is 5 meters long with 300 LEDs
     bpm converted to seconds per beat with (1/bpm*60) -- this is the amount of time that a total sweep should take
     each blast is 10 lights long, so 310 changes (/310)
     correct from s to ms
    */
    for(uint16_t i=0;i<stripLength;i++){ //move through each led index
        blackout(ledStrip); //blackout entire board before iterating, trail will be set in next loop
        for(uint8_t q=0;q<numStrips;q++){ //set each strip's led at index i
            for(int w=0;w<9;w++){ //trail 10 leds
                if(i>0){
                for(int k=0;k<3;k++) (ledStrip[q]->color[i-w][k])=(uint8_t)(colorSet[i-w][k]);
                (ledStrip[q]->brightness[i-w])=(uint32_t)(brightnessSet[i-w]);
                }
            }
        }
        //place delay here
    }//after this, all lights have led the trail, need to do the last 10 iterations to move trail off
    for(uint8_t i=10;i>0;i--){ //10 iterations, final value of 1 ensures that the final value is the final index
        // | | | | | | | | | |
        blackout(ledStrip); //reset strip before iteration
        for(uint8_t q=0;q<numStrips;q++){
            for(uint8_t w=i;w>0;w--){ //i defines the inner index, w pushes out to the edge, final value of 1 ensures that the final value is the final index (no overflow)
            for(int k=0;k<3;k++) (ledStrip[q]->color[stripLength-w][k])=(uint8_t)(colorSet[stripLength-w][k]);
            (ledStrip[q]->brightness[stripLength-w])=(uint32_t)(brightnessSet[stripLength-w]);
            }
        }
        
    }
}
/**
 * @brief reset brightness to standard and color to black to init
 * 
 * @param ledStrip array of strips to black out
 */
void blackout(struct LightStrip *ledStrip[numStrips]){
    uint8_t black[3];
    for(int i=0;i<3;i++) black[i]=0;
    for(uint8_t i=0;i<numStrips;i++){
        for(uint16_t q=0;q<stripLength;q++){
            for(int k=0;k<3;k++) (ledStrip[i]->color[q][k])=(uint8_t)(black[k]);
            ledStrip[i]->brightness[q]=standardBrightness;
        }
    }
}

/**
 * @brief controls functions start here. sets gpio values and initializes everything a
 */
void rmtInit(uint8_t *gpio[numStrips]){
    rmt_config_t stripConfigs[numStrips];;
    for(int i=0;i<numStrips;i++){ //ESP32 has 8 rmt channels which likely means that I'll max out at 8 possible individual strips
        stripConfigs[i].rmt_mode=RMT_MODE_TX;
        stripConfigs[i].channel=RMT_CHANNEL_0; //this is likely going to be an issue (defining all in the same channel) -- might need to hard code number of strips
        stripConfigs[i].gpio_num=(uint8_t)gpio[i];
        stripConfigs[i].mem_block_num=1; //don't fully get this
        stripConfigs[i].clk_div=2; //clock divider, sets pulse resolution to half
        stripConfigs[i].tx_config.loop_en=0;
        stripConfigs[i].tx_config.carrier_en=0;
        stripConfigs[i].tx_config.idle_output_en=1;
        stripConfigs[i].tx_config.idle_level=RMT_IDLE_LEVEL_LOW;
    }
}
/**
 * @brief 
 * used chat for a lot of these numbers so might need some adjusting (1.25us total bit period, .8 high for 1, .4 high for 0)
 * rmt ticks every 25 ns, so put all values in multiples of 25 ns and then divide by 25 to get number of ticks for hold time
 * @param bit bit value being translated to rmt value
 * @return rmt_item32_t 
 */
rmt_item32_t rmtTranslate(uint8_t bit){ 
    rmt_item32_t itemOut;
    if(bit){ //in case of high
        itemOut.level0=1;
        itemOut.duration0=800/25; //.8us
        itemOut.level1=0;
        itemOut.duration1=450/25;
    }
    else{
        itemOut.level0=1;
        itemOut.duration0=400/25;
        itemOut.level1=0;
        itemOut.duration1=850/25;
    }
    return itemOut;
}/**
 * @brief builds a frams to program a single led array for a single frame. this will be a series of 24*stripLength items (each representing a single bit)
 * the LED array works by pushing a series of bytes down the line. each LED 'absorbs' 3 bytes (green, red, blue), then acts as a short, just passing down to the later LEDS
 * an 'end of frame' (push value) call is represented by a 50us low period
 * 
 * colors is the input for this function, defining the GRB code for each LED (each index represents an 8 bit code defining brightness of that led)
 * items is the output address, each value of the colors array will be translated to a bit and added to the items array as an rmt_item
 */
void buildFrame(uint8_t *colors[stripLength][3], rmt_item32_t *items[stripLength*24]){
    uint32_t index=0; //removes the need for a 4th for loop by embedding the count
    for(int i=0;i<stripLength;i++){
        for(int color=0;color<3;color++){
            for(uint8_t bitNum=0;bitNum<8;bitNum++){ //build backwards
                *items[index++]=rmtTranslate((*colors[i][color] >> (7-bitNum))&1); //shift down so desired bit is at the bottom, then bitwise and with 0x1 to isolate
            }
        }
    }
}
/**
 * outputs the timing array created in buildFrame to the desired RMT channel 
 * (this confirms that every pin on a channel has identical output, need to give every pin its own channel to get individual addressabililty on each strip)
 */
void push(rmt_item32_t *items[stripLength*24], uint8_t stripNum){
    rmt_write_items(RMT_CHANNEL_0,*items, stripLength*24, 1); //output channel, output array, length of output array, hold output until finished
    esp_rom_delay_us(50); //latching delay, pretty much just pauses the esp for 50us
}