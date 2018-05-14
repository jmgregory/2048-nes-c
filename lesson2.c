/*	simple Hello World, for cc65, for NES
 *	doug fraker 2015
 *	feel free to reuse any code here
 */	

#include "DEFINE.c"

void draw_board(void);
void update_board(void);
void get_key_dir(void);
void Get_Input(void);
	
void merge_and_pull(unsigned char i1, unsigned char i2, unsigned char i3, unsigned char i4);
void merge4(unsigned char i1, unsigned char i2, unsigned char i3, unsigned char i4);
void merge2(unsigned char dest, unsigned char src);
void pull(unsigned char i1, unsigned char i2, unsigned char i3, unsigned char i4);
void place_new_tile(void);

unsigned char rand(void);

int main (void) {
	//	turn off the screen
	All_Off();

    seed = 0x8C54;
    place_new_tile();
    draw_row = 0;

	Load_Palette();
	Reset_Scroll();

	//	turn on screen
	All_On();

	
	while (1){ //	infinite loop
		while (NMI_flag == 0);	//	wait till NMI
		NMI_flag = 0;
        Frame_Count = 0;
		
        draw_board();
        Reset_Scroll();

        Get_Input();
        get_key_dir();
        update_board();
	}
}
	
void get_key_dir(void)
{
    if ((joypad1 & RIGHT) != 0 &&
        (joypad1old & RIGHT) == 0) {
        key_dir = RIGHT;
        return;
    }
    if ((joypad1 & LEFT) != 0 &&
        (joypad1old & LEFT) == 0) {
        key_dir = LEFT;
        return;
    }
    if ((joypad1 & UP) != 0 &&
        (joypad1old & UP) == 0) {
        key_dir = UP;
        return;
    }
    if ((joypad1 & DOWN) != 0 &&
        (joypad1old & DOWN) == 0) {
        key_dir = DOWN;
        return;
    }
    key_dir = 0;
}

//	inside the startup code, the NMI routine will ++NMI_flag and ++Frame_Count at each V-blank

void draw_board(void) {
    //draw_addr = 0x2000 + (4 << 5) + 4;
    ++draw_row;
    if (draw_row == 4)
        draw_row = 0;
    for (i = draw_row * 4; i < (draw_row * 4) + 4; ++i) {
        draw_addr = 0x2000 + (4 << 5) + 4 + ((i & 0x03) << 1) + ((i & 0x0C) << 4);
        PPU_ADDRESS = draw_addr >> 8;
        PPU_ADDRESS = draw_addr & 0xFF;
        
        power = board[i] & 0x0F;
        color = power == 0 ? 0x05 : ((color_lookup[power] & 0x03));

        tile_addr = 0x04 * color;
        PPU_DATA = tile_addr;
        ++tile_addr;
        PPU_DATA = tile_addr;

        draw_addr += 32;
        PPU_ADDRESS = draw_addr >> 8;
        PPU_ADDRESS = draw_addr & 0xFF;

        ++tile_addr;
        PPU_DATA = tile_addr;
        ++tile_addr;
        PPU_DATA = tile_addr;
    }

    // palette starting at 0x23C9
    for (i = 0; i < 16; i += 8) {
        PPU_ADDRESS = 0x23;
        PPU_ADDRESS = 0xC9 + i;

        power = board[i+0] & 0x0F;
        color = power == 0 ? 0x05 : color_lookup[power];
        pals[0] = color & 0x30;

        power = board[i+1] & 0x0F;
        color = power == 0 ? 0x05 : color_lookup[power];
        pals[1] = color & 0x30;

        power = board[i+4] & 0x0F;
        color = power == 0 ? 0x05 : color_lookup[power];
        pals[2] = color & 0x30;

        power = board[i+5] & 0x0F;
        color = power == 0 ? 0x05 : color_lookup[power];
        pals[3] = color & 0x30;

        PPU_DATA = (pals[0] >> 4) | (pals[1] >> 2) | (pals[2]) | (pals[3] << 2);

        power = board[i+2] & 0x0F;
        color = power == 0 ? 0x05 : color_lookup[power];
        pals[0] = color & 0x30;

        power = board[i+3] & 0x0F;
        color = power == 0 ? 0x05 : color_lookup[power];
        pals[1] = color & 0x30;

        power = board[i+6] & 0x0F;
        color = power == 0 ? 0x05 : color_lookup[power];
        pals[2] = color & 0x30;

        power = board[i+7] & 0x0F;
        color = power == 0 ? 0x05 : color_lookup[power];
        pals[3] = color & 0x30;

        PPU_DATA = (pals[0] >> 4) | (pals[1] >> 2) | (pals[2]) | (pals[3] << 2);
    }
}

//  0  1  2  3
//  4  5  6  7
//  8  9 10 11
// 12 13 14 15
void update_board(void) {
    board_changed = 0;

    for (i = 0; i < 16; ++i) {
        board[i] = board[i] & 0xF;
    }

    switch (key_dir) {
        case RIGHT:
            merge_and_pull( 3,  2,  1,  0);
            merge_and_pull( 7,  6,  5,  4);
            merge_and_pull(11, 10,  9,  8);
            merge_and_pull(15, 14, 13, 12);
            break;
        case LEFT:
            merge_and_pull( 0,  1,  2,  3);
            merge_and_pull( 4,  5,  6,  7);
            merge_and_pull( 8,  9, 10, 11);
            merge_and_pull(12, 13, 14, 15);
            break;
        case UP:
            merge_and_pull( 0,  4,  8, 12);
            merge_and_pull( 1,  5,  9, 13);
            merge_and_pull( 2,  6, 10, 14);
            merge_and_pull( 3,  7, 11, 15);
            break;
        case DOWN:
            merge_and_pull(12,  8,  4,  0);
            merge_and_pull(13,  9,  5,  1);
            merge_and_pull(14, 10,  6,  2);
            merge_and_pull(15, 11,  7,  3);
            break;
    }
    if (board_changed != 0)
        place_new_tile();
}

void place_new_tile()
{
    unsigned char emptyCount = 0;
    for (i = 0; i < 16; ++i) {
        if (board[i] == 0) {
            availables[emptyCount] = i;
            ++emptyCount;
        }
    }

    if (emptyCount == 0)
        return; // Game Over
    
    board[availables[rand() % emptyCount]] = (rand() > 229) ? 2 : 1;
}

void merge_and_pull(unsigned char i1, unsigned char i2, unsigned char i3, unsigned char i4) {
    merge4(i1, i2, i3, i4);
    pull(i1, i2, i3, i4);
}

void merge4(unsigned char i1, unsigned char i2, unsigned char i3, unsigned char i4)
{
    if (board[i1] != 0) {
        if ((board[i1] & 0xF) == (board[i2] & 0xF)) {
            merge2(i1, i2);
        }
        else if (board[i2] == 0) {
            if ((board[i1] & 0xF) == (board[i3] & 0xF)) {
                merge2(i1, i3);
            }
            else if (board[i3] == 0 && (board[i1] & 0xF) == (board[i4] & 0xF)) {
                merge2(i1, i4);
            }
        }
    }
    if (board[i2] != 0) {
        if ((board[i2] & 0xF) == (board[i3 & 0xF])) {
            merge2(i2, i3);
        }
        else if (board[i3] == 0 && (board[i2] & 0xF) == (board[i4] & 0xF)) {
            merge2(i2, i4);
        }
    }
    if (board[i3] != 0 && (board[i3] & 0xF) == (board[i4] & 0xF)) {
        merge2(i3, i4);
    }
}

void merge2(unsigned char dest, unsigned char src) {
    board[dest]++;
    board[src] = 0;
    ++board_changed;
}

void pull(unsigned char i1, unsigned char i2, unsigned char i3, unsigned char i4) {
    if (board[i1] == 0) {
        if (board[i2] != 0) {
            board[i1] = board[i2];
            board[i1] += 0x10;
            board[i2] = 0;
            ++board_changed;
        }
        else if (board[i3] != 0) {
            board[i1] = board[i3];
            board[i1] += 0x10;
            board[i3] = 0;
            ++board_changed;
        }
        else if (board[i4] != 0) {
            board[i1] = board[i4];
            board[i1] += 0x10;
            board[i4] = 0;
            ++board_changed;
        }
    }
    if (board[i2] == 0) {
        if (board[i3] != 0) {
            board[i2] = board[i3];
            board[i2] += 0x10;
            board[i3] = 0;
            ++board_changed;
        }
        else if (board[i4] != 0) {
            board[i2] = board[i4];
            board[i2] += 0x10;
            board[i4] = 0;
            ++board_changed;
        }
    }
    if (board[i3] == 0 && board[i4] != 0) {
        board[i3] = board[i4];
        board[i3] += 0x10;
        board[i4] = 0;
        ++board_changed;
    }
}

void All_Off(void) {
	PPU_CTRL = 0;
	PPU_MASK = 0; 
}

void All_On(void) {
	PPU_CTRL = 0x90; //	screen is on, NMI on
	PPU_MASK = 0x1e; 
}

void Reset_Scroll (void) {
	PPU_ADDRESS = 0;
	PPU_ADDRESS = 0;
	SCROLL = 0;
	SCROLL = 0;
}

void Load_Palette(void) {
	PPU_ADDRESS = 0x3f;
	PPU_ADDRESS = 0x00;
	for( i = 0; i < sizeof(PALETTE); ++i ) {
		PPU_DATA = PALETTE[i];
	}
}

