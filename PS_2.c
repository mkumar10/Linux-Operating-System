#include "PS_2.h"

#define SCANCODE_SIZE   0x3A
#define TWO_CASES       2
#define UPPER_CASE      1
#define LOWER_CASE      0
#define Ctrl            0x1D
#define Ctrl_Release    (0x1D | 0x80)
#define LShift          0x2A
#define LShift_Release  (0x2A | 0x80)
#define RShift          0x36
#define RShift_Release  (0x36 | 0x80)
#define CAPSLOCK        0x3A

#define ALT             0x38
#define ALT_Release     (0x38 | 0x80)
#define F1              0x3B
#define F2              0x3C
#define F3              0x3D
#define RELEASE         0x80

#define UP              0x48
#define DOWN            0x50
#define LEFT            0x4B
#define RIGHT           0x4D
#define PAGE_UP         0x49
#define PAGE_DOWN       0x51


// Local flags for PS/2 handler
volatile static int32_t Ctrl_FLAG = 0;
volatile static int32_t LShift_FLAG = 0;
volatile static int32_t RShift_FLAG = 0;    // We might want to have different funcitonality for LShift and RShift.
volatile static int32_t CAPS_FLAG = 0;
volatile static int32_t ALT_FLAG = 0;
volatile static int32_t FN_FLAG = 0;       // 1,2,3 stands for F1/2/3
uint8_t c = 0;


// Array to convert scan code to ASCII value.
static const uint8_t scan_to_ascii[TWO_CASES][SCANCODE_SIZE] = {{ 0,
  0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0x08,
  0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
  '\n',
  0,
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'',
  '`',
  0, '\\',
  'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
	0,
	0,' '
}, { 0,
  0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0x08,
  0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
  '\n',
  0,
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"',
  '~',
  0, '|',
  'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
	0,
	0,' '
}};


/*
 * init_keyboard
 *   DESCRIPTION: Enable the keyboard interrupts.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enable IRQ for keyboard on PIC.
 */
void init_keyboard()
{
	// // Enable PS2 Controller
	// init_PS2();
	// // Test PS2 Port 1
	// test_PS2();
	// Remove KB IRQ 1 Mask from PIC
	clear();
	enable_irq(KB_IRQ_NUM);
}

/*
 * get_c_keyboard
 *   DESCRIPTION: interrupt handler for keyboard.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Put a valid character onto screen.
 */
void get_c_keyboard()
{
	//disable_irq(KB_IRQ_NUM);
	// __asm__ volatile("pushal");

	// save_registers();

    // NO need to wait for polling!
	// do {
	// 	// Read from the first PS2 Port
	// 	if(inb(PS2_CONTROLLER_PORT_1) != c)
	// 	{
	// 		c = inb(PS2_CONTROLLER_PORT_1);
	// 	}
	// 	if(c > 0)
	// 		break;
	// } while(0);
    char_from_KB = 1;

    c = inb(PS2_CONTROLLER_PORT_1);
    // printf("Current scancode: %x\n", c);

	send_eoi(KB_IRQ_NUM);
    // Check the scancode with Control keys.
    // terminal_printf("Scancode: %x\n", c);
    if (c == Ctrl) {
        Ctrl_FLAG = 1;
    }else if(c == Ctrl_Release){
        Ctrl_FLAG = 0;
    }else if (c == LShift) {
        LShift_FLAG = 1;
    }else if (c == LShift_Release){
        LShift_FLAG = 0;
    }else if (c == RShift) {
        RShift_FLAG = 1;
    }else if (c == RShift_Release){
        RShift_FLAG = 0 ;
    }else if (c ==  CAPSLOCK){
        CAPS_FLAG = !CAPS_FLAG;
    }else if (c == LEFT){
        terminal_move_cursor_left();
    }else if (c == RIGHT){
        terminal_move_cursor_right();
    }else if (c == ALT){
        ALT_FLAG = 1;
    }else if (c == ALT_Release){
        ALT_FLAG = 0;
    }else if (c ==  F1){
        FN_FLAG = 1;
    }else if (c ==  F2){
        FN_FLAG = 2;
    }else if (c ==  F3){
        FN_FLAG = 3;
    }else if (c == (F1|RELEASE) || c == (F2|RELEASE) || c == (F3|RELEASE)){
        FN_FLAG = 0;
    }

    // Response with the scancode (i.e. display, clear, control)
    if ( c > 0 && c < SCANCODE_SIZE && (c!=CAPSLOCK && c!=Ctrl && c!=LShift && c!=RShift && c!=LEFT && c!=RIGHT && c!=ALT && c!=F1 && c!=F2 && c!=F3)){
        if (Ctrl_FLAG) {
            if (scan_to_ascii[LOWER_CASE][(uint8_t)c] == 'l'){
                terminal_clear();
            }
        }else if (LShift_FLAG || RShift_FLAG) {
            char display_char;
            if (CAPS_FLAG){
                display_char = scan_to_ascii[LOWER_CASE][(uint8_t)c];
            }else{
                display_char = scan_to_ascii[UPPER_CASE][(uint8_t)c];
            }
            // Sanity Check!
            if (display_char != 0) {
                terminal_putc(display_char);
            }
        } else if ((!LShift_FLAG) && (!RShift_FLAG)) {
            char display_char;
            if (CAPS_FLAG){
                display_char = scan_to_ascii[UPPER_CASE][(uint8_t)c];
            }else{
                display_char = scan_to_ascii[LOWER_CASE][(uint8_t)c];
            }
            if (display_char != 0) {
                terminal_putc(display_char);
            }
        }
    }

    char_from_KB = 0;

    if(FN_FLAG != 0 && ALT_FLAG != 0)
    {
      switch_terminals(FN_FLAG - 1);
    }
	// if (c >= 1 && c < SCANCODE_SIZE)
	// {
	// 	c = scan_to_ascii[LOWER_CASE][(uint8_t)c];
	// 	terminal_putc(c);
	// 	//printf("scancode value %c\n", c);
	// }
	//c = scancode[c+1];
	//putc(c);
	// Restore old Registers
	//enable_irq(KB_IRQ_NUM);
    // sti();
	// __asm__ volatile("popal; leave; iret"); // Use IRET in interrupt.
}
