#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "lib.h"
#include "types.h"
#include "structs.h"
#include "globals.h"

#define LINE_BUFFER_SIZE    128
#define VIDEO               0xB8000
#define NUM_COLS            80
#define NUM_ROWS            25
#define ATTRIB              0x7
#define BUFFER_NUM          2
#define CURR_LINE           0
#define ENTERED_LINE        1
#define Backspace_ASCII     0x08
#define CURSOR_PORT_1       0x3D4
#define CURSOR_PORT_2       0x3D5
#define MASK                0xFF
#define CURSOR_INDEX_1      0xF
#define CURSOR_INDEX_2      0xE
#define MAX_CMD_HIST        10
#define STRING_SIZE         6
#define STRING_LENGTH       5

volatile int32_t char_from_KB;

int32_t terminal_open(const uint8_t* filename);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_close(int32_t fd);

void terminal_scroll_down_curr();
void terminal_scroll_down_disp();

void terminal_clear();
void terminal_update_cursor();
void terminal_reset_buffer();
void terminal_putc(char c);
int32_t terminal_puts(int8_t* s);
int32_t terminal_printf(int8_t* format, ...);

void terminal_move_cursor_left();
void terminal_move_cursor_right();

#endif
