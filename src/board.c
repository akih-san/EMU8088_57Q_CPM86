/*
 * Copyright (c) 2023 @hanyazou
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "../src/emu88.h"

void (*board_sys_init_hook)(void) = NULL;
void (*bus_master_hook)(int) = NULL;
void (*board_start_i88_hook)(void) = NULL;

uint8_t (*board_addr_l_pins_hook)(void) = NULL;
void (*board_set_addr_l_pins_hook)(uint8_t) = NULL;
uint8_t (*board_data_pins_hook)(void) = NULL;
void (*board_set_data_pins_hook)(uint8_t) = NULL;
void (*board_set_data_dir_hook)(uint8_t) = NULL;
__bit (*board_rd_pin_hook)(void) = NULL;
__bit (*board_wr_pin_hook)(void) = NULL;

void (*board_nmi_sig_off_hook)(void) = NULL;
void (*board_nmi_sig_on_hook)(void) = NULL;

