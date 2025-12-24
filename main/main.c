/**
 * @brief 
 * Chapter Room Ceiling LED controller program
 * This Program is intended to be placed on an ESP32 board and run as a standalone system
 * 
 * The Program will intake a BPM and provide light effects in response
 *      to begin with I plan on having to manually input the BPM but in the future I'd like to integrate some automatic BPM reader
 *          this could be either through an input audio or some (spotify) API access, both have their downsides but assuming there exists a spotify api with that info, both can be done
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include "LightControl.h"

#define stripLength 300
#define numStrips 10
#define standardBrightness 75

void app_main() { //apparently this is the standard main function for the esp-idf for some reason
    //uint32_t t1,t2=timer_read();
    uint8_t outPins[numStrips]={10,11,12,13,14,15,16,17,18,19}; //idk if these pins even actually exist, update when I figure that out
    for(int i=0;i<10;i++){ //sets values for strips array defined in LightControl.c
        strips[i]->length=stripLength;
        strips[i]->outPin=outPins[i];
    }
    init(); //inits strip structs
    rmtInit(outPins); //comm init
    rmt_item32_t frames[numStrips][stripLength*24]; //layout values to be redefined


    while(1){ //forever loop
        
        
        for(int i=0;i<numStrips;i++){ //by this time, strip structs should be prepared, they are now being translated to output
            buildFrame((strips[i]->colors),&frames[i]);
        }
        for(int i=0;i<numStrips;i++){ //making this its own for loop to avoid any processing delays making the strips seeming more out of sync
            push(frames[i],i);
        }
    }
}
