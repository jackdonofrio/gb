/*
Simple bouncing ball Gameboy rom demo.
This is a debugging tool.
Some of the code is messy.
Testing currently:
- graphics - sprite and bkg layers
- moving sprites
- sound effects
Soon to be added:
- window layer graphics
*/


#include <gb/gb.h>
#include "simplebackground.h"
#include "simplebackgroundmap.h"
#include "ball.h"

static void delay_vbl(UINT8 n);
static void fall(INT8 spritelocation[], BYTE* falling, const INT8 floorY, 
    const INT8 gravity, INT8* y_velocity);
static void setup_sound();
static void setup_ball_sprite();
static void setup_background();
static void play_beep();


void main(){

    const INT8 startY = 20;
    const INT8 startX = 10;
    const INT8 floorY = 119;
    const INT8 gravity = 1;
    const INT8 x_velocity = 2;
    INT8 y_velocity = 1;
    BYTE falling = 0;
    INT8 spritelocation[2];
    
    setup_background();
    setup_ball_sprite();
    setup_sound();

    spritelocation[0] = startX;
    spritelocation[1] = startY;
    move_sprite(0, spritelocation[0], spritelocation[1]);

    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    while(1){
        if ((joypad() & J_DOWN) || falling){
            fall(spritelocation, &falling, floorY, gravity, &y_velocity);
        }
        if ((joypad() & J_UP)) {
            spritelocation[1] = startY;
            falling = 0;
        }
        if (joypad() & J_LEFT){
            spritelocation[0] = spritelocation[0] - x_velocity;
        }
        if (joypad() & J_RIGHT){
            spritelocation[0] = spritelocation[0] + x_velocity;
        }
        move_sprite(0, spritelocation[0], spritelocation[1]);
        delay_vbl(5);
    }
}


// use this instead of delay() as it is less CPU intensive
void delay_vbl(UINT8 n){
    UINT8 i;
    for(i = 0; i < n; i++){
        wait_vbl_done();
    }     
}

void fall(INT8 spritelocation[], BYTE* falling, const INT8 floorY, 
    const INT8 gravity, INT8* y_velocity)
{
    if (*falling == 0) {
        *falling = 1;
        (*y_velocity) = 2;
    }
    (*y_velocity) += gravity;

    spritelocation[1] += (*y_velocity);

    if (spritelocation[1] >= floorY) {
        *y_velocity = -((*y_velocity) * 9) / 10;
        spritelocation[1] = floorY;
        play_beep();
        if ((*y_velocity) == 0) {
            *falling = 0;
        }
    }
}

void setup_background()
{
    set_bkg_data(0, 9, backgroundtiles);
    set_bkg_tiles(0, 0, 40, 18, backgroundmap);
}

void setup_ball_sprite()
{
    set_sprite_data(0, 1, ball);
	set_sprite_tile(0,0); 
}

void setup_sound()
{
    NR52_REG = 0x80; // turns on sound
    NR50_REG = 0x77; // sets the volume for both left and right channel - this is max
    NR51_REG = 0xFF; // select which channels to use - left nibble, right nibble = one bit for the L one bit for the R of all four channels
}

void play_beep()
{
    // see https://github.com/bwhitman/pushpin/blob/master/src/gbsound.txt
    // channel 1 register 0, Frequency sweep settings
    // 7	Unused
    // 6-4	Sweep time(update rate) (if 0, sweeping is off)
    // 3	Sweep Direction (1: decrease, 0: increase)
    // 2-0	Sweep RtShift amount (if 0, sweeping is off)
    // 0001 0110 is 0x16, sweet time 1, sweep direction increase, shift ammount per step 110 (6 decimal)
    NR10_REG = 0x16; 

    // channel 1 register 1: Wave pattern duty and sound length
    // Channels 1 2 and 4
    // 7-6	Wave pattern duty cycle 0-3 (12.5%, 25%, 50%, 75%), duty cycle is how long a quadrangular  wave is "on" vs "of" so 50% (2) is both equal.
    // 5-0 sound length (higher the number shorter the sound)
    // 01000000 is 0x40, duty cycle 1 (25%), wave length 0 (long)
    NR11_REG = 0x40;

    // channel 1 register 2: Volume Envelope (Makes the volume get louder or quieter each "tick")
    // On Channels 1 2 and 4
    // 7-4	(Initial) Channel Volume
    // 3	Volume sweep direction (0: down; 1: up)
    // 2-0	Length of each step in sweep (if 0, sweeping is off)
    // NOTE: each step is n/64 seconds long, where n is 1-7	
    // 0111 0011 is 0x73, volume 7, sweep down, step length 3
    NR12_REG = 0x73;  

    // channel 1 register 3: Frequency LSbs (Least Significant bits) and noise options
    // for Channels 1 2 and 3
    // 7-0	8 Least Significant bits of frequency (3 Most Significant Bits are set in register 4)
    NR13_REG = 0x00;   

    // channel 1 register 4: Playback and frequency MSbs
    // Channels 1 2 3 and 4
    // 7	Initialize (trigger channel start, AKA channel INIT) (Write only)
    // 6	Consecutive select/length counter enable (Read/Write). When "0", regardless of the length of data on the NR11 register, sound can be produced consecutively.  When "1", sound is generated during the time period set by the length data contained in register NR11.  After the sound is ouput, the Sound 1 ON flag, at bit 0 of register NR52 is reset.
    // 5-3	Unused
    // 2-0	3 Most Significant bits of frequency
    // 1100 0011 is 0xC3, initialize, no consecutive, frequency = MSB + LSB = 011 0000 0000 = 0x300
    NR14_REG = 0xC3;

}