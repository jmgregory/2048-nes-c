// let's define some things

#define PPU_CTRL		*((unsigned char*)0x2000)
#define PPU_MASK		*((unsigned char*)0x2001)
#define PPU_STATUS		*((unsigned char*)0x2002)
#define OAM_ADDRESS		*((unsigned char*)0x2003)
#define SCROLL			*((unsigned char*)0x2005)
#define PPU_ADDRESS		*((unsigned char*)0x2006)
#define PPU_DATA		*((unsigned char*)0x2007)
#define OAM_DMA			*((unsigned char*)0x4014)


#define RIGHT		0x01
#define LEFT		0x02
#define DOWN		0x04
#define UP			0x08
#define START		0x10
#define SELECT		0x20
#define B_BUTTON	0x40
#define A_BUTTON	0x80


// Globals
// our startup code initialized all values to zero
#pragma bss-name(push, "ZEROPAGE")
unsigned char NMI_flag;
unsigned char Frame_Count;
unsigned char i;
unsigned char tile_addr;
unsigned char pals[4];
unsigned char color;
unsigned char power;
unsigned char key_dir;
unsigned char board_x;
unsigned char board_y;
unsigned char board_changed;
unsigned char joypad1;
unsigned char joypad1old;
unsigned char joypad1test; 
unsigned char joypad2; 
unsigned char joypad2old;
unsigned char joypad2test;
unsigned char draw_row;
unsigned char board[16] = { 0 };

/*
unsigned char board[] = {
     1,  2,  3,  4,
     1,  2,  3,  4,
     5,  6,  7,  8,
     9,  0,  1,  2
};
*/

unsigned char availables[15];


// 5 colors: ABCDE
// Palette  Colors
// -------  ------
//    0      0ABC
//    1      0ADE
//    2      0BDE
//    3      0CDE

// power display color
// ----- ------- -----
//    1      2     A
//    2      4     A
//    3      8     B
//    4     16     B
//    5     32     C
//    6     64     C
//    7    128     D
//    8    256     D
//    9    512     E
//   10   1024     E

//         P   C1   C2
//  0   0x00
//  1   0x10 0x04 0x01
//  2   0x20 0x08 0x02
//  3   0x30 0x0C 0x03
unsigned char color_lookup[] = {
    0x00, // dummy (so we start at index 1)
    0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x16, 0x16, 0x17, 0x17,
          0x05, 0x06, 0x06, 0x07, 0x07, 0x16, 0x16, 0x17, 0x17,
                0x0A, 0x0A, 0x0B, 0x0B, 0x26, 0x26, 0x27, 0x27,
                      0x0A, 0x0B, 0x0B, 0x26, 0x26, 0x27, 0x27,
                            0x0F, 0x0F, 0x36, 0x36, 0x37, 0x37,
                                  0x0F, 0x36, 0x36, 0x37, 0x37,
                                        0x1A, 0x1A, 0x3B, 0x3B,
                                              0x1A, 0x3B, 0x3B,
                                                    0x1F, 0x1F,
                                                          0x1F
};
unsigned char color_row_starts[] = {
     0, // dummy
     0, 9, 17, 24, 30, 35, 39, 42, 44, 45
};
// x = power1
// y = power2
// assuming y >= x  (swap if needed)
// index(x, y) = x + color_row_starts[y]
// lookup = color_lookup[index(x, y)]
// palette = lookup >> 4
// color_x = (lookup & 0x0C) >> 2
// color_y = lookup & 0x03

unsigned short seed;
unsigned short draw_addr;


#pragma bss-name(push, "OAM")
unsigned char SPRITES[256];
// OAM equals ram addresses 200-2ff




const unsigned char PALETTE[] = {
    0x1F, 0x31, 0x21, 0x37,
    0x1F, 0x31, 0x27, 0x16,
    0x1F, 0x21, 0x27, 0x16,
    0x1F, 0x37, 0x27, 0x16,
};

// Prototypes
void All_Off (void);
void All_On (void);
void Reset_Scroll (void);
void Load_Palette (void);
void update_Sprites (void);
void move_logic (void);

