/*
RPG demo - playground for developing an RPG I'm planning.
Note: clean up code as you figure out what you want/how things will be done
11/16/22
*/


#include <stdlib.h>
#include <stdbool.h>
#include <gb/gb.h>
#include "simplebackground.h"
#include "simplebackgroundmap.h"
#include "simplebackgroundmap2.h"


typedef struct GameCharacter {
    UBYTE spriteids[4];
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
    UBYTE sprite_size;
    UBYTE direction;
    bool is_shield;
    bool in_motion; // TODO - make separate struct for projectiles eventually
} GameCharacter_t;

// remember - adding many parameters adds unneccessary overhead; via manual, better to use global vars
// also note- DO NOT USE RECURSION.
static void delay_vbl(uint8_t n);
static void setup_sound();
static void setup_ball_sprite();
static void setup_background();
static void play_beep();
void move_game_character(GameCharacter_t* character, uint8_t x, uint8_t y);
bool can_move(uint8_t x, uint8_t y);
bool sprites_collide(GameCharacter_t* s1, GameCharacter_t* s2);
void setup_monk(GameCharacter_t* monk, uint8_t first_tile, uint8_t nb_tiles);
void move_magic(GameCharacter_t* magic, uint8_t x, uint8_t y);
void setup_magic(GameCharacter_t* magic, uint8_t first_tile);
void end_magic(GameCharacter_t* magic);

// TODO - do away with these coordinate variables as you shift to just
// identifying tiles for logic
static const uint8_t wall_top    =  25;
static const uint8_t wall_bottom = 136;
static const uint8_t wall_left   =  13;
static const uint8_t wall_right  = 148;
static const uint8_t door_top    =  64;
static const uint8_t door_bottom =  72;
static const uint8_t left_door_x1=   0;
static const uint8_t left_door_x2=   8;
static const uint8_t right_door_x = 152;
static const uint8_t door = 0x8;
static const uint8_t floor_map1   = 0;
static const uint8_t floor_map2   = 4;
static const UBYTE UP = 0;
static const UBYTE DOWN = 1;
static const UBYTE LEFT = 2;
static const UBYTE RIGHT = 3;
static const UBYTE MAGIC_SPEED = 4;





static const unsigned char Cat[] = 
{
  0x00,0x00,0x00,0x00,0x0A,0x0A,0x4E,0x44,
  0x8E,0x8E,0x7C,0x7C,0x24,0x24,0x24,0x24
};

static const unsigned char Magic[] =
{
  0x00,0x00,0x42,0x00,0x00,0x18,0x00,0x24,
  0x00,0x24,0x00,0x18,0x42,0x00,0x00,0x00
};

static const unsigned char MagicInverse[] =
{
  0x00,0x00,0x00,0x42,0x18,0x00,0x24,0x00,
  0x24,0x00,0x18,0x00,0x00,0x42,0x00,0x00
};

static const unsigned char DefaultMonk[] =
{
    0x01,0x01,0x03,0x03,0x07,0x07,0x06,0x07,
    0x04,0x07,0x06,0x07,0x03,0x03,0x03,0x03,
    0x07,0x07,0x0B,0x0B,0x0B,0x08,0x03,0x03,
    0x03,0x03,0x03,0x03,0x02,0x02,0x02,0x02,
    0x80,0x80,0xC0,0xC0,0xE0,0xE0,0x60,0xE0,
    0x20,0xE0,0x60,0xE0,0xC0,0xC0,0xC0,0xC0,
    0xE0,0xE0,0xD0,0xD0,0xF0,0x10,0xE0,0xC0,
    0xE0,0xC0,0xE0,0xC0,0x60,0x40,0x60,0x40,
};
static const unsigned char ShieldMonk[] =
{
    0x01,0x01,0x03,0x03,0x07,0x07,0x06,0x07,
    0x04,0x07,0x06,0x07,0x03,0x03,0x03,0x3B,
    0x03,0x7F,0x13,0x7F,0x3B,0x7C,0x13,0x7F,
    0x13,0x7F,0x03,0x3B,0x02,0x02,0x02,0x02,
    0x80,0x80,0xC0,0xC0,0xE0,0xE0,0x60,0xE0,
    0x20,0xE0,0x60,0xE0,0xC0,0xC0,0xC0,0xC0,
    0xE0,0xE0,0xD0,0xD0,0xF0,0x10,0xE0,0xC0,
    0xE0,0xC0,0xE0,0xC0,0x60,0x40,0x60,0x40
};
static const unsigned char StaffMonk[] =
{
    0x01,0x01,0x03,0x03,0x07,0x77,0x06,0x57,
    0x04,0x57,0x06,0x17,0x03,0x13,0x03,0x13,
    0x07,0x17,0x0B,0x1B,0x0B,0x18,0x03,0x13,
    0x03,0x13,0x03,0x13,0x02,0x12,0x02,0x12,
    0x80,0x80,0xC0,0xC0,0xE0,0xE0,0x60,0xE0,
    0x20,0xE0,0x60,0xE0,0xC0,0xC0,0xC0,0xC0,
    0xE0,0xE0,0xD0,0xD0,0xF0,0x10,0xE0,0xC0,
    0xE0,0xC0,0xE0,0xC0,0x60,0x40,0x60,0x40
};
static const unsigned char StaffMonkReverse[] =
{
  0x01,0x01,0x03,0x03,0x07,0x07,0x07,0x07,
  0x07,0x07,0x07,0x07,0x03,0x03,0x03,0x03,
  0x07,0x07,0x0B,0x0B,0x0F,0x08,0x07,0x03,
  0x07,0x03,0x07,0x03,0x06,0x02,0x06,0x02,
  0x80,0x80,0xC0,0xC0,0xE0,0xEE,0xE0,0xEA,
  0xE0,0xEA,0xE0,0xE8,0xC0,0xC8,0xC0,0xC8,
  0xE0,0xE8,0xD0,0xD8,0xD0,0x18,0xC0,0xC8,
  0xC0,0xC8,0xC0,0xC8,0x40,0x48,0x40,0x48
};
static const unsigned char StaffMonkLeft[] =
{
    0x01,0x01,0x03,0x03,0x07,0x77,0x06,0x57,
    0x04,0x57,0x06,0x17,0x03,0x13,0x03,0x13,
    0x07,0x17,0x0B,0x1B,0x0B,0x18,0x03,0x13,
    0x03,0x13,0x03,0x13,0x02,0x12,0x02,0x12,
    0x80,0x80,0xC0,0xC0,0xE0,0xE0,0x60,0xE0,
    0x20,0xE0,0x60,0xE0,0xC0,0xC0,0xC0,0xC0,
    0xE0,0xE0,0xD0,0xD0,0xF0,0x10,0xE0,0xC0,
    0xE0,0xC0,0xE0,0xC0,0x60,0x40,0x60,0x40
};
static const unsigned char StaffMonkRight[] =
{
  0x01,0x01,0x03,0x03,0x07,0x07,0x06,0x07,
  0x04,0x07,0x06,0x07,0x03,0x03,0x03,0x03,
  0x07,0x07,0x0B,0x0B,0x0B,0x08,0x03,0x03,
  0x03,0x03,0x03,0x03,0x02,0x02,0x02,0x02,
  0x80,0x80,0xC0,0xC0,0xE0,0xEE,0x60,0xEA,
  0x20,0xEA,0x60,0xE8,0xC0,0xC8,0xC0,0xC8,
  0xE0,0xE8,0xD0,0xD8,0xF0,0x18,0xE0,0xC8,
  0xE0,0xC8,0xE0,0xC8,0x60,0x48,0x60,0x48,
};





void main(){

    const INT8 x_speed = 4;
    const INT8 y_speed = 4;
    bool is_in_rm2 = false;
    bool monk2_up = false;

    // 0,1,2,3 - monk1
    // 4,5,6,7 - monk2
    // 8 - magic
    set_sprite_data(0, 4, StaffMonk);
    set_sprite_data(4, 4, ShieldMonk);
    set_sprite_data(8, 1, Magic);


    // note = may not allocate memory in the final version; could just have them as global
    GameCharacter_t* monk = (GameCharacter_t*) malloc(sizeof(GameCharacter_t));
    setup_monk(monk, 0, 4);

    GameCharacter_t* monk2 = (GameCharacter_t*) malloc(sizeof(GameCharacter_t));
    setup_monk(monk2, 4, 4); 

    GameCharacter_t* magic = (GameCharacter_t*) malloc(sizeof(GameCharacter_t));
    setup_magic(magic, 8);


    setup_background();
    setup_sound();
    move_game_character(monk2, 40, 40);

    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    bool pressed;
    while(1){
        pressed = false;
        if ((joypad() & J_DOWN)){
            pressed = true;
            if (can_move(monk->x, monk->y)) {
                monk->y = monk->y + y_speed;
            }
            if (monk->direction != DOWN) {
                if (!monk->is_shield) {
                    set_sprite_data(0, 4, StaffMonk);
                }
                monk->direction = DOWN;
            }
        }
        if (!pressed && (joypad() & J_UP)) {
            pressed = true;
            if (can_move(monk->x, monk->y - monk->height - monk->sprite_size)) {
                monk->y = monk->y - y_speed;
            }
            if (monk->direction != UP) {
                if (!monk->is_shield) {
                    set_sprite_data(0, 4, StaffMonkReverse);
                }
                monk->direction = UP;
            }
        }
        if (!pressed && (joypad() & J_LEFT)){
            pressed = true;
            if (can_move(monk->x - monk->sprite_size, monk->y - monk->sprite_size) ) {
                monk->x = monk->x - x_speed;
            }
            if (monk->direction != LEFT) {
                if (!monk->is_shield) {
                    set_sprite_data(0, 4, StaffMonkLeft);
                }
                monk->direction = LEFT;
            }
        }
        if (!pressed && (joypad() & J_RIGHT)){
            pressed = true;
            if (can_move(monk->x + monk->sprite_size, monk->y - monk->sprite_size)) {
                monk->x = monk->x + x_speed;
            }
            if (monk->direction != RIGHT) {
                if (!monk->is_shield) {
                    set_sprite_data(0, 4, StaffMonkRight);
                }
                monk->direction = RIGHT;
            }
        }
        if ((joypad() & J_A)) {
            if (monk->is_shield) {
                set_sprite_data(0, 4, StaffMonk);
            } else{
                set_sprite_data(0, 4, ShieldMonk);
            }
            monk->is_shield = !monk->is_shield;
            delay_vbl(5); // TODO - use sys_time instead for this, figure out from manual
        }
        if ((joypad() & J_B) && !monk->is_shield) {
            play_beep();
            magic->direction = monk->direction;
            if (magic->direction == UP || magic->direction == RIGHT) { // this bit is in order to make sure
                move_magic(magic, monk->x + monk->sprite_size, monk->y); // the magic comes from the staff
            } else {
                move_magic(magic, monk->x, monk->y);
            }
            magic->in_motion = true;

        }
        if (magic->in_motion) {
            if (magic->direction == UP) {
                if (can_move(magic->x, magic->y - MAGIC_SPEED)) {
                    magic-> y -= MAGIC_SPEED;
                } else {
                    end_magic(magic);
                }
            } else if (magic->direction == DOWN) {
                if (can_move(magic->x, magic->y - magic->sprite_size)) {
                    magic-> y += MAGIC_SPEED;
                } else {
                    end_magic(magic);
                }
            } else if (magic->direction == LEFT) {
                if (can_move(magic->x - magic->sprite_size, magic->y)) {
                    magic->x -= MAGIC_SPEED;
                } else {
                    end_magic(magic);
                }
            } else if (magic->direction == RIGHT) {
                if (can_move(magic->x, magic->y)) {
                    magic->x += MAGIC_SPEED;
                } else {
                    end_magic(magic);
                }
            }
            // create alternating / flashing sprite for magical effect
            if ((magic->x + magic->y) & 1) {
                set_sprite_data(8, 1, Magic);
            } else {
                set_sprite_data(8, 1, MagicInverse);
            }
            move_magic(magic, magic->x, magic->y);
        }

        

        bool entered_right = monk->x >= right_door_x;
        if (monk->y >= door_top && monk->y <= door_bottom && 
            (monk->x >= left_door_x1 && monk->x < left_door_x2 
            || entered_right))
        {
            if (entered_right) {
                monk->x = left_door_x2 + 4;
            } else {
                monk->x = right_door_x - 4; 
            }
            if (is_in_rm2) {
                set_bkg_tiles(0, 0, backgroundmapWidth, backgroundmapHeight, backgroundmap);
            } else {
                set_bkg_tiles(0, 0, backgroundmapWidth, backgroundmapHeight, backgroundmap2);
            }
            is_in_rm2 = !is_in_rm2;
        }
        if ((monk2->y - 1 <= wall_top) || (monk2->y + 1 >= wall_bottom)){
            monk2_up = !monk2_up;
        }

        // scroll_sprite(4, 0, monk2_up ? -1 : 1);
        monk2->y += monk2_up ? -1 : 1;

        // collision detection
        if (sprites_collide(magic, monk2)) {
            monk2->x = 80;
            monk2->y = 80;
            end_magic(magic);
            move_game_character(magic, magic->x, magic->y);
        }


        move_game_character(monk, monk->x, monk->y);            
        move_game_character(monk2, monk2->x, monk2->y);
        delay_vbl(5);
    }
}


// use this instead of delay() as it is less CPU intensive
void delay_vbl(uint8_t n){
    uint8_t i;
    for(i = 0; i < n; i++){
        wait_vbl_done();
    }     
}

// static inline unsigned char get_tile(uint8_t x, uint8_t y)
// {
//     // length of row * (y) + x offset
//     return backgroundmap[(x >> 3) + backgroundmapWidth * (y >> 3)];
// }

void move_game_character(GameCharacter_t* character, uint8_t x, uint8_t y)
{
    move_sprite(character->spriteids[0], x, y);
    move_sprite(character->spriteids[1], x, y + character->sprite_size);
    move_sprite(character->spriteids[2], x + character->sprite_size, y);
    move_sprite(character->spriteids[3], x + character->sprite_size, y + character->sprite_size);
    character->x = x;
    character->y = y;
}

bool can_move(uint8_t x, uint8_t y)
{
    uint8_t tile = get_bkg_tile_xy((x) / 8, (y) / 8);
    return tile == floor_map1 || tile == floor_map2 || tile == door;
}

bool sprites_collide(GameCharacter_t* s1, GameCharacter_t* s2)
{
    return (s1->x >= s2->x && s1->x <= s2->x + s2->width && s1->y >= s2->y && s1->y <= s2->y + s2->height)
        || (s2->x >= s1->x && s2->x <= s1->x + s1->width && s2->y >= s1->y && s2->y <= s1->y + s1->height);
}


void setup_magic(GameCharacter_t* magic, uint8_t first_tile)
{
    magic->x = 160;
    magic->y = 160;
    magic->width = 8;
    magic->height = 8;
    magic->sprite_size = 8;
    magic->direction = DOWN;
    magic->in_motion = false;
    set_sprite_tile(first_tile, first_tile);
    magic->spriteids[0] = first_tile;
}

void move_magic(GameCharacter_t* magic, uint8_t x, uint8_t y) {
    move_sprite(magic->spriteids[0], x, y);
    magic->x = x;
    magic->y = y;
}

void end_magic(GameCharacter_t* magic)
{
    magic->in_motion = false;
    magic->x = 160;
    magic->y = 160;
}

void setup_monk(GameCharacter_t* monk, uint8_t first_tile, uint8_t nb_tiles)
{
    monk->x = 64;
    monk->y = 64;
    monk->width = 16;
    monk->height = 16;
    monk->sprite_size = 8;
    monk->direction = DOWN;
    monk->is_shield = false;
    for (uint8_t i = first_tile; i < first_tile + nb_tiles; i++) {
        set_sprite_tile(i, i);
        monk->spriteids[i - first_tile] = i;
    }
    // set_sprite_tile(0, 0);
    // monk->spriteids[0] = 0;
    // set_sprite_tile(1, 1);
    // monk->spriteids[1] = 1;
    // set_sprite_tile(2, 2);
    // monk->spriteids[2] = 2;
    // set_sprite_tile(3, 3);
    // monk->spriteids[3] = 3;
    move_game_character(monk, monk->x, monk->y);
}

void setup_background()
{
    set_bkg_data(0, 9, backgroundtiles);
    set_bkg_tiles(0, 0, 20, 18, backgroundmap);
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
    NR12_REG = 0x72;  

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