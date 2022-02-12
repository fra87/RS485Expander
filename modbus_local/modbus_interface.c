/* 
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
#include "modbus_interface.h"

// FreeModbus includes
#include "../modbus/include/mb.h"
#include "../modbus/include/mbport.h"

// Helper register for value inversion
static uint8_t inversionRegister = 0;

void initModbusInterface(void)
{
    // NOTE: Do not change the baud rate (38400), since it must be synced to
    // the value written in the EUSART module (it will not be calculated)
    eMBInit(MB_RTU, MODBUS_SLAVE_ADDRESS, 0, 38400, MB_PAR_NONE);
}

void enableModbusInterface(void)
{
    eMBEnable();
}

void loopModbusInterface(void)
{
    eMBPoll();
}

eMBErrorCode eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode)
{
    eMBErrorCode result = MB_ENOERR;
    
    for (; (usNRegs > 0) && (result == MB_ENOERR); pucRegBuffer++, usAddress++, usNRegs--)
    { // For each register
        switch (eMode)
        {
            case MB_REG_READ:
                {
                    uint8_t outreg = 0;

                    switch(usAddress)
                    {
                        case 0: // GPIO
                            outreg = ((PORTA & 0x30) << 2) & (PORTC & 0x3F);
                            outreg ^= inversionRegister;
                            break;
                        case 1: // TRIS
                            outreg = ((TRISA & 0x30) << 2) & (TRISC & 0x3F);
                            break;
                        case 2: // WPU
                            outreg = ((WPUA & 0x30) << 2) & (WPUC & 0x3F);
                            break;
                        case 3: // INVERSION
                            outreg = inversionRegister;
                            break;
                        default:
                            result = MB_ENOREG;
                            break;
                    }
                
                    *pucRegBuffer = outreg;
                }
                break;
            case MB_REG_WRITE:
                {
                    uint8_t inreg = *pucRegBuffer;

                    switch(usAddress)
                    {
                        case 0: // GPIO
                            inreg ^= inversionRegister;
                            LATA = LATA | ((inreg >> 2) & 0x30) & ((inreg >> 2) | 0xCF);
                            LATC = LATC | (inreg & 0x3F) & (inreg | 0xC0);
                            break;
                        case 1: // TRIS
                            TRISA = TRISA | ((inreg >> 2) & 0x30) & ((inreg >> 2) | 0xCF);
                            TRISC = TRISC | (inreg & 0x3F) & (inreg | 0xC0);
                            break;
                        case 2: // WPU
                            WPUA = WPUA | ((inreg >> 2) & 0x30) & ((inreg >> 2) | 0xCF);
                            WPUC = WPUC | (inreg & 0x3F) & (inreg | 0xC0);
                            break;
                        case 3: // INVERSION
                            inversionRegister = inreg;
                            break;
                        default:
                            result = MB_ENOREG;
                            break;
                    }
                }
                break;
            default:
                // Error
                result = MB_EIO;
                break;
        }
    }
    
    return result;
}
eMBErrorCode eMBRegCoilsCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode)
{
    eMBErrorCode result = MB_ENOERR;
    
    // If too many coils were requested, store an error
    if (usAddress + usNCoils > 8)
    {
        // Limit the number of coils requested
        usNCoils = 8 - usAddress;
        result = MB_ENOREG;
    }
    
    // Mask highlighting the bits that the request is modifying
    uint8_t requestMask = ((0x01 << usNCoils) - 1) << usAddress;
    
    switch (eMode)
    {
        case MB_REG_READ:
            {
                // Get the output register value
                uint8_t outreg = ((PORTA & 0x30) << 2) & (PORTC & 0x3F);
                outreg ^= inversionRegister;
                
                // Mask out unrequested bits
                outreg &= requestMask;
                
                // Only consider bits after usAddress
                outreg >>= usAddress;
                
                // Send this value
                *pucRegBuffer = outreg;
            }
            break;
        case MB_REG_WRITE:
            {
                // At most 8 bits will be requested (or otherwise we can ignore)
                // Let's align it with the channels
                uint8_t inreg = (*pucRegBuffer) << usAddress;
                inreg ^= inversionRegister;
                
                // Mask for PORTA and PORTC
                uint8_t portaMask = (requestMask & 0xC0) >> 2;
                uint8_t portcMask = requestMask & 0x3F;

                LATA = LATA | ((inreg >> 2) & portaMask) & ((inreg >> 2) | ~portaMask);
                LATC = LATC | (inreg & portcMask) & (inreg | ~portcMask);
            }
            break;
        default:
            // Error
            result = MB_EIO;
            break;
    }
    
    return result;
}
