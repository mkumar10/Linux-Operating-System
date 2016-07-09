#include "terminal.h"

#define update_coords_back(X, Y)     \
do{                             \
    if (X == 0){                \
        X = NUM_COLS - 1;       \
        Y = (Y > 0)?(Y-1):0;    \
    }else{                      \
        X--;                    \
    }                           \
}while(0)

#define update_coords_forward(X, Y) \
do{                                 \
    X++;                            \
    if (X>=NUM_COLS){                \
        X %= NUM_COLS;              \
        Y++;                        \
    }                               \
}while(0)

static uint8_t * term_vid_mem = (uint8_t *)VIDEO;
static char input_buffer[NUM_TERMS][BUFFER_NUM][LINE_BUFFER_SIZE];
volatile static uint32_t curr_idx[NUM_TERMS];
volatile static uint32_t term_X[NUM_TERMS];
volatile static uint32_t term_Y[NUM_TERMS];

// Command History-----------------------------------------------------------
command_hist_t cmd_hist_arr[NUM_TERMS][MAX_CMD_HIST];
volatile command_hist_t* cmd_hist_head[NUM_TERMS];
volatile command_hist_t* cmd_hist_tail[NUM_TERMS];
volatile uint32_t cmd_hist_size[NUM_TERMS];
//---------------------------------------------------------------------------

// Stop move cursor or delete contents before or after it.
// Used only when TERM_READ_FLAG is on.
volatile static uint32_t term_X_start[NUM_TERMS];
volatile static uint32_t term_Y_start[NUM_TERMS];
volatile static uint32_t term_X_end[NUM_TERMS];
volatile static uint32_t term_Y_end[NUM_TERMS];

volatile static int32_t TERM_READ_FLAG[NUM_TERMS];
volatile static int32_t TERM_WRITE_FLAG[NUM_TERMS];
volatile static int32_t ENTER_FLAG[NUM_TERMS];

/*
 * terminal_open
 *   DESCRIPTION: Initialize the terminal.
 *   INPUTS: ignored
 *   OUTPUTS: none
 *   RETURN VALUE: 0.
 *   SIDE EFFECTS: Change contents in video_mem and display.
 */
int32_t terminal_open(const uint8_t* filename){
    uint32_t i;
    for(i = 0; i<NUM_TERMS; i++){
        curr_idx[i] = 0;
        term_X[i] = 0;
        term_Y[i] = 0;
        term_X_start[i] = 0;
        term_Y_start[i] = 0;
        term_X_end[i] = 0;
        term_Y_end[i] = 0;
        TERM_READ_FLAG[i] = 0;
        TERM_WRITE_FLAG[i] = 0;
        ENTER_FLAG[i] = 0;

        // Command History-----------------------------------------------
        // Initializing the doubly linked list for command history
        cmd_hist_size[i] = 0;
        int j;
        for(j = 0; j<MAX_CMD_HIST; j++){
            cmd_hist_arr[i][j].next = (j>=(MAX_CMD_HIST-1)) ? NULL : &cmd_hist_arr[i][j+1];
            cmd_hist_arr[i][j].prev = (j<=0) ? NULL : &cmd_hist_arr[i][j-1];
        }
        cmd_hist_head[i] = &cmd_hist_arr[i][0];
        cmd_hist_tail[i] = &cmd_hist_arr[i][0];
        //------------------------------------------------------------------
    }

    char_from_KB = 0;
    terminal_clear();

    // Uncomment the following to debug.------------------------------------
    // TERM_WRITE_FLAG[curr_term] = 1;
    //----------------------------------------------------------------------
    return 0;
}

/*
 * terminal_read
 *   DESCRIPTION: Read the inputs from the terminal.
 *   INPUTS: buf, nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: number of characters processed.
 *   SIDE EFFECTS: none.
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    uint32_t chars_read;
    // Condition check.
    if (buf == NULL || nbytes<0){
        return -1;
    }
    if (nbytes == 0)
        return 0;

    // uint32_t TERM_R_F_save = TERM_READ_FLAG[curr_term];
    //          TEMM_W_F_save = TERM_WRITE_FLAG[curr_term];

    TERM_READ_FLAG[curr_term] = 1;
    TERM_WRITE_FLAG[curr_term] = 0;
    ENTER_FLAG[curr_term] = 0;

    term_X_start[curr_term] = term_X[curr_term];
    term_Y_start[curr_term] = term_Y[curr_term];
    term_X_end[curr_term] = term_X[curr_term];
    term_Y_end[curr_term] = term_Y[curr_term];
    terminal_reset_buffer();

    for(;!((ENTER_FLAG[curr_term]) && (disp_term==curr_term));){
    }

    uint32_t flags;
    cli_and_save(flags);

    // for (chars_read=0; (chars_read < nbytes) && !(input_buffer[curr_term][ENTERED_LINE][chars_read]=='\0' && input_buffer[curr_term][CURR_LINE][chars_read]=='\0') && chars_read < LINE_BUFFER_SIZE; chars_read++){
    //     *((uint8_t*)buf+chars_read) = (curr_idx[curr_term]==0) ? input_buffer[curr_term][ENTERED_LINE][chars_read] : input_buffer[curr_term][CURR_LINE][chars_read];
    // }
     for (chars_read=0; (chars_read < nbytes) && !(input_buffer[disp_term][ENTERED_LINE][chars_read]=='\0' && input_buffer[disp_term][CURR_LINE][chars_read]=='\0') && chars_read < LINE_BUFFER_SIZE; chars_read++){
        *((uint8_t*)buf+chars_read) = input_buffer[disp_term][ENTERED_LINE][chars_read];
    }
    // chars_read = (curr_idx[curr_term]==0) ? (chars_read-1) : chars_read;
    terminal_reset_buffer();

    ENTER_FLAG[disp_term] = 0;
    // TERM_READ_FLAG[curr_term] = TERM_R_F_save;
    TERM_READ_FLAG[curr_term] = 0;
    sti();
    restore_flags(flags);
    return (chars_read >= LINE_BUFFER_SIZE) ? (LINE_BUFFER_SIZE) : (chars_read);
}

/*
 * terminal_write
 *   DESCRIPTION: Output the buffer contents into display.
 *   INPUTS: buf, nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: number of characters processed.
 *   SIDE EFFECTS: Change contents in video_mem and display.
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    uint32_t chars_written;
    if (buf == NULL || nbytes<0){
        return -1;
    }

    uint32_t flags;
    cli_and_save(flags);

    // uint32_t    TM_W_F_save = TERM_WRITE_FLAG[curr_term],
    //             TM_R_F_save = TERM_READ_FLAG[curr_term];

    TERM_WRITE_FLAG[curr_term] = 1;
    TERM_READ_FLAG[curr_term] = 0;

    // for (chars_written=0; chars_written<nbytes && (*((char*)buf + chars_written)  != '\0'); chars_written++){
    for (chars_written=0; chars_written<nbytes; chars_written++){    // Verbose mode
        if ((curr_idx[curr_term] % LINE_BUFFER_SIZE) == (LINE_BUFFER_SIZE-1))
            terminal_reset_buffer();
        // if ((*((char*)buf + chars_written)  != '\0'))            // Compact mode.
            terminal_putc(*((char*)buf + chars_written));
    }
    terminal_reset_buffer();
    // ENTER_FLAG[curr_term] = 0;
    // TERM_WRITE_FLAG[curr_term] = TM_W_F_save;
    // TERM_READ_FLAG[curr_term] = TM_R_F_save;
    TERM_WRITE_FLAG[curr_term] = 0;
    sti();
    restore_flags(flags);

    return chars_written;
}


/*
 * terminal_close
 *   DESCRIPTION: Always return 0.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: clear contents in video_mem and display.
 */
int32_t terminal_close(int32_t fd){
    terminal_clear();
    return 0;
}





/*
 * all_char
 *   DESCRIPTION: put any other characters onto display.
 *   INPUTS: write_flag
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Change contents in video_mem and display.
 */
void all_char(char c, uint32_t write_flag){
    if (write_flag==1){
                // Show the character in display
        *(uint8_t *)(term_vid_mem + ((NUM_COLS*term_Y[disp_term] + term_X[disp_term]) << 1)) = c;
        *(uint8_t *)(term_vid_mem + ((NUM_COLS*term_Y[disp_term] + term_X[disp_term]) << 1) + 1) = ATTRIB;

        *(uint8_t *)(video_buffer[disp_term] + ((NUM_COLS*term_Y[disp_term] + term_X[disp_term]) << 1)) = c;
        *(uint8_t *)(video_buffer[disp_term] + ((NUM_COLS*term_Y[disp_term] + term_X[disp_term]) << 1) + 1) = ATTRIB;

        //Update the coordinates in terminal
        update_coords_forward(term_X[disp_term], term_Y[disp_term]);
        if (term_Y[disp_term] >= NUM_ROWS){
            terminal_scroll_down_disp();
            term_Y[disp_term] = NUM_ROWS - 1;
        }

        // Update curr_idx[disp_term] and cursor
        terminal_update_cursor();
        input_buffer[disp_term][CURR_LINE][curr_idx[disp_term]] = c;
        curr_idx[disp_term]++;
    }
    else if (write_flag==2){

        *(uint8_t *)(video_buffer[curr_term] + ((NUM_COLS*term_Y[curr_term] + term_X[curr_term]) << 1)) = c;
        *(uint8_t *)(video_buffer[curr_term] + ((NUM_COLS*term_Y[curr_term] + term_X[curr_term]) << 1) + 1) = ATTRIB;

        //Update the coordinates in terminal
        update_coords_forward(term_X[curr_term], term_Y[curr_term]);
        if (term_Y[curr_term] >= NUM_ROWS){
            terminal_scroll_down_curr();
            term_Y[curr_term] = NUM_ROWS - 1;
        }

        // Update curr_idx[curr_term] and cursor
        input_buffer[curr_term][CURR_LINE][curr_idx[curr_term]] = c;
        curr_idx[curr_term]++;

    }
}


/*
 * new_line_char
 *   DESCRIPTION: enter next line and update curosr.
 *   INPUTS: write_flag
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Change contents in video_mem and display.
 */
void new_line_char(uint32_t write_flag)
{

    switch(write_flag)
    {
        case 1:
        {
            uint32_t i;
            // Append the line feed at the end. Unix style.
            input_buffer[disp_term][CURR_LINE][curr_idx[disp_term]] = '\n';

            // Copy the contents into ENTERED_LINE buffer to return.
            for (i=0; i < LINE_BUFFER_SIZE; i++){
                input_buffer[disp_term][ENTERED_LINE][i] = (i <= curr_idx[disp_term]) ? input_buffer[disp_term][CURR_LINE][i] : '\0';
                input_buffer[disp_term][CURR_LINE][i] = '\0';
            }

            // Update X & Y.
            term_X[disp_term] = 0;
            term_Y[disp_term]++;
            if (term_Y[disp_term] >= NUM_ROWS){        // Update Y and scroll the screen down
                terminal_scroll_down_disp();
                term_Y[disp_term] = NUM_ROWS - 1;
            }
            curr_idx[disp_term] = 0;

            // Update flags and cursor
            terminal_update_cursor();

            if (TERM_READ_FLAG[disp_term]) ENTER_FLAG[disp_term] = 1;

            break;
        }

        case 2:
        {
            uint32_t i;
            // Append the line feed at the end. Unix style.
            input_buffer[curr_term][CURR_LINE][curr_idx[curr_term]] = '\n';

            // Copy the contents into ENTERED_LINE buffer to return.
            for (i=0; i < LINE_BUFFER_SIZE; i++){
                input_buffer[curr_term][ENTERED_LINE][i] = (i <= curr_idx[curr_term]) ? input_buffer[curr_term][CURR_LINE][i] : '\0';
                input_buffer[curr_term][CURR_LINE][i] = '\0';
            }

            // Update X & Y.
            term_X[curr_term] = 0;
            term_Y[curr_term]++;
            if (term_Y[curr_term] >= NUM_ROWS){        // Update Y and scroll the screen down
                terminal_scroll_down_curr();
                term_Y[curr_term] = NUM_ROWS - 1;
            }
            curr_idx[curr_term] = 0;
            break;
        }
    }

}

/*
 * backspace_char
 *   DESCRIPTION: Put a backspace_char.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Change contents in video_mem and display.
 */
void backspace_char()
{
    if (TERM_READ_FLAG[disp_term])
    {
        if (curr_idx[disp_term] > 0)
        {
            // Todo: multiline input. Need to wrap to next line but keep the buffer.
            // Update X & Y
            update_coords_back(term_X[disp_term], term_Y[disp_term]);

            // Erase the previous character.
            *(uint8_t *)(term_vid_mem + ((NUM_COLS*term_Y[disp_term] + term_X[disp_term]) << 1)) = ' ';
            *(uint8_t *)(term_vid_mem + ((NUM_COLS*term_Y[disp_term] + term_X[disp_term]) << 1) + 1) = ATTRIB;

            *(uint8_t *)(video_buffer[disp_term] + ((NUM_COLS*term_Y[disp_term] + term_X[disp_term]) << 1)) = ' ';
            *(uint8_t *)(video_buffer[disp_term] + ((NUM_COLS*term_Y[disp_term] + term_X[disp_term]) << 1) + 1) = ATTRIB;

            // Update the buffer and cursor
            curr_idx[disp_term]--;
            input_buffer[disp_term][CURR_LINE][curr_idx[disp_term]] = '\0';
            terminal_update_cursor();
        }
    }
}

/*
 * terminal_putc
 *   DESCRIPTION: Put a character onto display in terminal.
 *   INPUTS: char c - character
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Change contents in video_mem and display.
 */
void terminal_putc(char c)
{
    uint32_t flags;
    cli_and_save(flags);

    uint32_t write_flag = 0;
    uint32_t bmap_val = ((TERM_READ_FLAG[disp_term]<<4) | (TERM_WRITE_FLAG[disp_term]<<3) | (TERM_READ_FLAG[curr_term]<<2) | (TERM_WRITE_FLAG[curr_term]<<1) | char_from_KB);
    char bitmap_temp[STRING_SIZE];
    itoa(bmap_val , bitmap_temp, 2);
    // printf("BMAP VALUE - %d\n", bmap_val);
    char bitmap[STRING_SIZE];
    uint32_t l = strlen((int8_t*)bitmap_temp);
    // printf("%d\n", l);
    uint32_t temp_idx;
    for (temp_idx=0; temp_idx<STRING_LENGTH; temp_idx++){
        if (temp_idx<(STRING_LENGTH-l))
            bitmap[temp_idx] = '0';
        else
            bitmap[temp_idx] = bitmap_temp[temp_idx+l-STRING_LENGTH];
    }
    bitmap[STRING_SIZE-1] = '\0';

    // printf("%s\n", bitmap);

    // if ( c=='+' && curr_term!=disp_term){
    //     printf(" %s%d\n", bitmap, (curr_term==disp_term));
    //     // printf("Condition met!\n");
    // }

    if ((strncmp(bitmap, (int8_t*)"10000", STRING_LENGTH)==0) ||
        (strncmp(bitmap, (int8_t*)"01001", STRING_LENGTH)==0) ||
        (strncmp(bitmap, (int8_t*)"01000", STRING_LENGTH)==0) ||
        (strncmp(bitmap, (int8_t*)"00101", STRING_LENGTH)==0) ||
        (strncmp(bitmap, (int8_t*)"00100", STRING_LENGTH)==0) ||
        (strncmp(bitmap, (int8_t*)"00011", STRING_LENGTH)==0) ||
        (strncmp(bitmap, (int8_t*)"10100", STRING_LENGTH)==0) ||
        (strncmp(bitmap, (int8_t*)"01101", STRING_LENGTH)==0) ||
        (strncmp(bitmap, (int8_t*)"01100", STRING_LENGTH)==0) ||
        (strncmp(bitmap, (int8_t*)"01011", STRING_LENGTH)==0) ){
        write_flag = 0;
    }

    else if (strncmp(bitmap, (int8_t*)"10001", STRING_LENGTH)==0)
    {
        write_flag = 1;
    }
    else if (strncmp(bitmap, (int8_t*)"10101", STRING_LENGTH)==0)
    {
        write_flag = 1;
    }
    else if (strncmp(bitmap, (int8_t*)"10011", STRING_LENGTH)==0)
    {
        write_flag = 1;
    }
    else if (strncmp(bitmap, (int8_t*)"10010", STRING_LENGTH)==0)
    {
        write_flag = 2;
    }
    else if (strncmp(bitmap, (int8_t*)"01010", STRING_LENGTH)==0 || strncmp(bitmap, (int8_t*)"1010", 4)==0)
    {
        write_flag = 2;
    }
    else if (strncmp(bitmap, (int8_t*)"00010", STRING_LENGTH)==0 || strncmp(bitmap, (int8_t*)"10", 2)==0)
    {
        write_flag = 2;
    }

    if((disp_term!=curr_term)&&TERM_WRITE_FLAG[disp_term]&&TERM_WRITE_FLAG[curr_term]&&(!char_from_KB))
    {
        write_flag = 2;
    }

    if(curr_term == disp_term)
    {
        if (TERM_READ_FLAG[curr_term] || TERM_WRITE_FLAG[curr_term]){
            if (c == '\n' || c == '\r') {
                new_line_char(1);
            }
            else if(c==Backspace_ASCII && (char_from_KB==1) && TERM_READ_FLAG[disp_term]){
                backspace_char();
            }
            else{
                all_char(c, 1);
            }
        }
    }
    else
    {
        if (write_flag==1 || write_flag==2){

            if (c == '\n' || c == '\r') {
                new_line_char(write_flag);
            }
            else if(c==Backspace_ASCII && (char_from_KB==1) && TERM_READ_FLAG[disp_term])
            {
                backspace_char();
            }
            else
            {
                all_char(c, write_flag);
            }
        }
    }

    char_from_KB = 0;
    sti();
    restore_flags(flags);
}

/*
 * terminal_scroll_down_curr
 *   DESCRIPTION: Scroll down the screen by one line.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Change contents in video_mem and display.
 */
void terminal_scroll_down_curr(){
    // Video memory consists of 2 bytes.
    uint32_t new_size = ((NUM_ROWS-1)*NUM_COLS) << 1;

    uint16_t empty_char = ((int)' ' | (ATTRIB << 8));

    // Copy from the second line.
    uint8_t* new_start = video_buffer[curr_term] + (NUM_COLS << 1);
    memcpy(video_buffer[curr_term], new_start, new_size);

    new_start = video_buffer[curr_term] + new_size;
    memset_word(new_start, empty_char, NUM_COLS);
}

void terminal_scroll_down_disp(){
    // Video memory consists of 2 bytes.
    uint32_t new_size = ((NUM_ROWS-1)*NUM_COLS) << 1;

    uint16_t empty_char = ((int)' ' | (ATTRIB << 8));

    // Copy from the second line. - Buffer
    uint8_t* new_start = video_buffer[disp_term] + (NUM_COLS << 1);
    memcpy(video_buffer[disp_term], new_start, new_size);

    new_start = video_buffer[disp_term] + new_size;
    memset_word(new_start, empty_char, NUM_COLS);
    // Copy from the second line. - the actual physical memory
    new_start = term_vid_mem + (NUM_COLS << 1);
    memcpy(term_vid_mem, new_start, new_size);

    new_start = term_vid_mem + new_size;
    memset_word(new_start, empty_char, NUM_COLS);
}

/*
 * terminal_clear
 *   DESCRIPTION: Clear the screen and customize.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Change contents in video_mem and display.
 */
void terminal_clear() {
    uint32_t flags;
    cli_and_save(flags);

    // For now we just use clear() from lib.c
    terminal_reset_buffer();
    // Clear Video Memory
    clear();
    uint32_t i;
    // Clear buffer
    for(i=0; i<NUM_ROWS*NUM_COLS; i++) {
        *(video_buffer[disp_term] + (i << 1)) = ' ';
        *(video_buffer[disp_term]+ (i << 1) + 1) = ATTRIB;
    }

    term_X[disp_term] = 0;
    term_Y[disp_term] = 0;
    term_X_start[disp_term] = 0;
    term_Y_start[disp_term] = 0;
    term_X_end[disp_term] = 0;
    term_Y_end[disp_term] = 0;
    terminal_update_cursor();
    ENTER_FLAG[disp_term] = 0;    // Make sure user can type.

    sti();
    restore_flags(flags);
}

/*
 * terminal_update_cursor
 *   DESCRIPTION: Update the cursor on the screen.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none.
 */
void terminal_update_cursor(){
    uint16_t position=(term_Y[disp_term]*NUM_COLS) + term_X[disp_term];

    // cursor LOW port to vga INDEX register
    outb(CURSOR_INDEX_1, CURSOR_PORT_1);
    outb((uint8_t)(position&MASK), CURSOR_PORT_2);
    // cursor HIGH port to vga INDEX register
    outb(CURSOR_INDEX_2, CURSOR_PORT_1);
    outb((uint8_t)((position>>8)&MASK), CURSOR_PORT_2);
}

/*
 * terminal_reset_buffer
 *   DESCRIPTION: clear the buffer of terminal.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none.
 */
void terminal_reset_buffer(){
    uint32_t flags;
    cli_and_save(flags);

    for(curr_idx[curr_term] = 0; curr_idx[curr_term] < LINE_BUFFER_SIZE; curr_idx[curr_term]++){
        input_buffer[curr_term][CURR_LINE][curr_idx[curr_term]] = '\0';
        input_buffer[curr_term][ENTERED_LINE][curr_idx[curr_term]] = '\0';
    }
    curr_idx[curr_term] = 0;

    sti();
    restore_flags(flags);
}


/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output.
 * */
int32_t terminal_printf(int8_t *format, ...){

    uint32_t flags;
    cli_and_save(flags);

    /* Pointer to the format string */
    int8_t* buf = format;

    uint32_t    TM_W_F_save = TERM_WRITE_FLAG[curr_term],
                TM_R_F_save = TERM_READ_FLAG[curr_term];
    TERM_WRITE_FLAG[curr_term] = 1;
    TERM_READ_FLAG[curr_term] = 0;
    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while(*buf != '\0') {
    if ((buf-format)%LINE_BUFFER_SIZE == (LINE_BUFFER_SIZE-1))
        terminal_reset_buffer();
        switch(*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch(*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            terminal_putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if(alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    terminal_puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    terminal_puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                terminal_puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                terminal_puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            terminal_putc( (uint8_t) *((int32_t *)esp) );
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            terminal_puts( *((int8_t **)esp) );
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                terminal_putc(*buf);
                break;
        }
        buf++;
    }
    terminal_reset_buffer();
    TERM_WRITE_FLAG[curr_term] = TM_W_F_save;
    TERM_READ_FLAG[curr_term] = TM_R_F_save;

    sti();
    restore_flags(flags);

    return (buf - format);
}


/*
* int32_t puts(int8_t* s);
*   Inputs: int_8* s = pointer to a string of characters
*   Return Value: Number of bytes written
*   Function: Output a string to the console
*/
int32_t
terminal_puts(int8_t* s)
{
    register int32_t index = 0;
    uint32_t    TM_W_F_save = TERM_WRITE_FLAG[curr_term],
                TM_R_F_save = TERM_READ_FLAG[curr_term];
    TERM_WRITE_FLAG[curr_term] = 1;
    TERM_READ_FLAG[curr_term] = 0;
    while(s[index] != '\0') {
        if (index % LINE_BUFFER_SIZE == (LINE_BUFFER_SIZE -1))
            terminal_reset_buffer();
        terminal_putc(s[index]);
        index++;
    }
    terminal_reset_buffer();
    TERM_WRITE_FLAG[curr_term] = TM_W_F_save;
    TERM_READ_FLAG[curr_term] = TM_R_F_save;
    return index;
}

//---------------------------------------
// Extra Function for Extra point. Not in use for now.
void terminal_move_cursor_left(){
    // if (TERM_READ_FLAG[disp_term]){
    //     if (term_Y[disp_term] > term_Y_start[disp_term]) {
    //         // term_X[disp_term] = (term_X[disp_term] - 1 + NUM_COLS) % NUM_COLS;
    //         // term_Y[disp_term] = term_X[disp_term] == 0 ? (term_Y[disp_term] - 1) : term_Y[disp_term];
    //         if (term_X[disp_term] == 0){
    //             term_X[disp_term] = NUM_COLS - 1;
    //             term_Y[disp_term]--;
    //         }else{
    //             term_X[disp_term]--;
    //         }
    //     }else if (term_Y[disp_term] == term_Y_start[disp_term]) {
    //         term_X[disp_term] = term_X[disp_term] > term_X_start[disp_term] ? (term_X[disp_term] -1) : term_X_start[disp_term];
    //     }
    //     terminal_update_cursor();
    // }
}

void terminal_move_cursor_right(){
    // if (TERM_READ_FLAG[disp_term]){
    //     if (term_Y[disp_term] < term_Y_end[disp_term]){
    //         term_X[disp_term] = (term_X[disp_term]+1) % NUM_COLS;
    //         term_Y[disp_term] = (term_X[disp_term] == (NUM_COLS-1)) ? (term_Y[disp_term]+1) : term_Y[disp_term];
    //     }else if (term_Y[disp_term] == term_Y_end[disp_term]) {
    //         term_X[disp_term] = (term_X[disp_term] < term_X_end[disp_term]) ? (term_X[disp_term] + 1) : term_X_end[disp_term];
    //     }
    //     terminal_update_cursor();
    // }
}
