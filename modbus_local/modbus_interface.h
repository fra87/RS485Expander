/* 
 * File:   modbus_interface.h
 * Author: Fra
 *
 * Created on February 10, 2022, 12:45 AM
 * 
 * Copyright (c) 2022 fra87
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef MODBUS_INTERFACE_H
#define	MODBUS_INTERFACE_H

#define MODBUS_SLAVE_ADDRESS 0x0A

/**
 * \brief Initialize the MODBUS interface
 * 
 * The MODBUS interface has the following specification:
 * - Address: MODBUS_SLAVE_ADDRESS
 * - Mode: RTU
 * - Baudrate: 38400
 * - Parity: None
 */
void initModbusInterface(void);

/**
 * \brief Enable the MODBUS interface
 * 
 * This function shall be called after interrupts are enabled
 */
void enableModbusInterface(void);

void loopModbusInterface(void);

#endif	/* MODBUS_INTERFACE_H */

