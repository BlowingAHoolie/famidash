// Written by Doug Fraker
// version 1.3, 10/31/2022

// Why am I doing so much with a vram_buffer? This is an automated system, which
// works when the screen is on. You can write to the buffer at any time. 
// During NMI / v-blank, it automatically writes to the PPU. 
// You don't have to worry about timing those writes. And, more importantly, 
// you shouldn't have to turn the screen off...
// allowing us to make smooth scrolling games.


void set_vram_buffer(void);
// sets the vram update to point to the vram_buffer. VRAM_BUF defined in crt0.s
// this can be undone by set_vram_update(NULL)


void __fastcall__ _one_vram_buffer(unsigned long args);
#define one_vram_buffer(data, ppu_address) (storeByteToSreg(data), __AX__ = ppu_address, _one_vram_buffer(__EAX__))
// to push a single byte write to the vram_buffer

void __fastcall__ _multi_vram_buffer(unsigned long args);

#define multi_vram_buffer_horz(data, len, ppu_address) (xargs[0] = len, storeWordToSreg((unsigned int)data), __A__ = LSB(ppu_address), __AX__<<=8, __AX__ |= MSB(ppu_address)|NT_UPD_HORZ, _multi_vram_buffer(__EAX__))
// to push multiple writes as one sequential horizontal write to the vram_buffer
#define multi_vram_buffer_vert(data, len, ppu_address) (xargs[0] = len, storeWordToSreg((unsigned int)data), __A__ = LSB(ppu_address), __AX__<<=8, __AX__ |= MSB(ppu_address)|NT_UPD_VERT, _multi_vram_buffer(__EAX__))
// to push multiple writes as one sequential vertical write to the vram_buffer


void clear_vram_buffer(void);
// resets the vram buffer, if you need to undo something, like for a scene change


unsigned char __fastcall__ get_pad_new(unsigned char pad);
// pad 0 or 1, use AFTER pad_poll() to get the trigger / new button presses
// more efficient than pad_trigger, which runs the entire pad_poll code again


unsigned char __fastcall__ get_frame_count(void);
// use this internal value to time events, this ticks up every frame

unsigned char __fastcall__ _check_collision(unsigned long args);
#define check_collision(object1, object2) (storeWordToSreg((unsigned int)object1), __AX__ = (unsigned int)object2, _check_collision(__EAX__))
// expects an object (struct) where the first 4 bytes are X, Y, width, height
// you will probably have to pass the address of the object like &object
// the struct can be bigger than 4 bytes, as long as the first 4 bytes are X, Y, width, height


void __fastcall__ _pal_fade_to(unsigned short from_to);
#define pal_fade_to(from, to) _pal_fade_to((to<<8)|byte(from))
// adapted from Shiru's "Chase" game code
// values must be 0-8, 0 = all black, 8 = all white, 4 = normal


void __fastcall__ set_scroll_x(unsigned int x);
// x can be in the range 0-0x1ff, but any value would be fine, it discards higher bits


void __fastcall__ set_scroll_y(unsigned int y);
// y can be in the range 0-0x1ff, but any value would be fine, it discards higher bits
// NOTE - different system than neslib (which needs y in range 0-0x1df)
// the advantage here, is you can set Y scroll to 0xff (-1) to shift the screen down 1,
// which aligns it with sprites, which are shifted down 1 pixel


int __fastcall__ _add_scroll_y(unsigned long args);
#define add_scroll_y(add, scroll) (storeByteToSreg(add), __AX__ = scroll, _add_scroll_y(__EAX__))
// add a value to y scroll, keep the low byte in the 0-0xef range
// returns y scroll, which will have to be passed to set_scroll_y


int __fastcall__ _sub_scroll_y(unsigned long args);
#define sub_scroll_y(sub, scroll) (storeByteToSreg(sub), __AX__ = scroll, _sub_scroll_y(__EAX__))
// subtract a value from y scroll, keep the low byte in the 0-0xef range
// returns y scroll, which will have to be passed to set_scroll_y


int __fastcall__ _get_ppu_addr(unsigned long args);
#define get_ppu_addr(nt, x, y) (storeByteToSreg(nt), __AX__ = (x<<8)|y, _get_ppu_addr(__EAX__))
// gets a ppu address from x and y coordinates (in pixels)
// x is screen pixels 0-255, y is screen pixels 0-239, nt is nametable 0-3


int __fastcall__ _get_at_addr(unsigned long args);
#define get_at_addr(nt, x, y) (storeByteToSreg(nt), __AX__ = (x<<8)|y, _get_at_addr(__EAX__))
// gets a ppu address in the attribute table from x and y coordinates (in pixels)
// x is screen pixels 0-255, y is screen pixels 0-239, nt is nametable 0-3


// the next 4 functions are for my metatile system, as described in my tutorial
// nesdoug.com


void __fastcall__ set_data_pointer(const char * data);
// for the metatile system, pass it the addresses of the room data
// room data should be exactly 240 bytes (16x15)
// each byte represents a 16x16 px block of the screen


void __fastcall__ set_mt_pointer(const char * metatiles);
// for the metatile system, pass it the addresses of the metatile data
// a metatile is a 16x16 px block
// metatiles is variable length, 5 bytes per metatile...
// TopL, TopR, BottomL, BottomR, then 1 byte of palette 0-3
// max metatiles = 51 (because 51 x 5 = 255)


void __fastcall__ _buffer_1_mt(unsigned long args);
#define buffer_1_mt(ppu_address, index) (storeByteToSreg(index), __AX__ = ppu_address, _buffer_1_mt(__EAX__))
// will push 1 metatile and 0 attribute bytes to the vram_buffer
// make sure to set_vram_buffer(), and clear_vram_buffer(), 
// and set_mt_pointer() 
// "metatile" should be 0-50, like the metatile data


void __fastcall__ buffer_4_mt(short ppu_address, char index);
// will push 4 metatiles (2x2 box) and 1 attribute byte to the vram_buffer
// this affects a 32x32 px area of the screen, and pushes 17 bytes to the vram_buffer.
// make sure to set_vram_buffer(), and clear_vram_buffer(), 
// set_data_pointer(), and set_mt_pointer() 
// "index" is which starting byte in the room data, to convert to tiles.
// use index = (y & 0xf0) + (x >> 4); where x 0-255 and y 0-239;
// index should be 0-239, like the room data it represents


void flush_vram_update2(void);
// same as flush_vram_update, but assumes that a pointer to the vram has been set already
// this is for when the screen is OFF, but you still want to write to the PPU
// with the vram_buffer system


void __fastcall__ color_emphasis(char color);
// change the PPU's color emphasis bits

#define COL_EMP_BLUE 0x80
#define COL_EMP_GREEN 0x40
#define COL_EMP_RED 0x20
#define COL_EMP_NORMAL 0x00
#define COL_EMP_DARK 0xe0


void __fastcall__ _xy_split(unsigned long args);
#define xy_split(x, y) (storeWordToSreg(x), __AX__ = y, _xy_split(__EAX__))
// a version of split that actually changes the y scroll midscreen
// requires a sprite zero hit, or will crash


void gray_line(void);
// For debugging. Insert at the end of the game loop, to see how much frame is left.
// Will print a gray line on the screen. Distance to the bottom = how much is left.
// No line, possibly means that you are in v-blank.


#define high_byte(a) *((unsigned char*)&a+1)
#define low_byte(a) *((unsigned char*)&a)
// for getting or modifying just 1 byte of an int




#define POKE(addr,val)     (*(unsigned char*) (addr) = (val))
#define PEEK(addr)         (*(unsigned char*) (addr))
// examples
// POKE(0xD800, 0x12); // stores 0x12 at address 0xd800, useful for hardware registers
// B = PEEK(0xD800); // read a value from address 0xd800, into B, which should be a char


void seed_rng(void);
// get from the frame count. You can use a button (start on title screen) to trigger