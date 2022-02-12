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

#include "port.h"
#include "../modbus/include/mb.h"
#include "../modbus/include/mbport.h"
#include "../mcc_generated_files/mcc.h"

// ISR helper functions
void Custom_EUSART_Transmit_ISR(void);
void Custom_EUSART_Receive_ISR(void);

// EUSART MCC global variables
extern volatile uint8_t eusartTxTail;
extern volatile uint8_t eusartTxBuffer[];
extern volatile uint8_t eusartTxBufferRemaining;
extern volatile uint8_t eusartRxHead;
extern volatile eusart_status_t eusartRxStatusBuffer[];

extern void (*EUSART_FramingErrorHandler)(void);
extern void (*EUSART_OverrunErrorHandler)(void);
extern void (*EUSART_ErrorHandler)(void);
void EUSART_RxDataHandler(void);

// WARNING: This must match the definition in eusart.c
#define EUSART_TX_BUFFER_SIZE 8

void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    // Enable receive
    RCSTAbits.CREN = xRxEnable ? 1 : 0;
    
    // Enable transmit
    TXSTAbits.TXEN = xTxEnable ? 1 : 0;
}

BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
    // Ignoring ucPORT since it is not useful in this platform
    // Ignoring ulBaudRate, since we are using the integrated calculator in MCC
    
    // Initialize also the CREN/TXEN bits
    vMBPortSerialEnable(FALSE, FALSE);
    
    // Set the callbacks
    PIE1bits.RCIE = 0;
    EUSART_SetRxInterruptHandler(Custom_EUSART_Receive_ISR);
    EUSART_SetTxInterruptHandler(Custom_EUSART_Transmit_ISR);
    PIE1bits.RCIE = 1;
    
    // For the moment, only 8 bit - no parity mode is supported
    return (ucDataBits == 8) && (eParity == MB_PAR_NONE);
}

BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    EUSART_Write(ucByte);
    return TRUE;
}

BOOL xMBPortSerialGetByte(CHAR * pucByte)
{
    *pucByte = EUSART_Read();
    return TRUE;
}

void Custom_EUSART_Transmit_ISR(void)
{
    // add your EUSART interrupt custom code
    if(EUSART_TX_BUFFER_SIZE > eusartTxBufferRemaining)
    {
        TXREG = eusartTxBuffer[eusartTxTail++];
        if(EUSART_TX_BUFFER_SIZE <= eusartTxTail)
        {
            eusartTxTail = 0;
        }
        eusartTxBufferRemaining++;
    }
    else
    {
        PIE1bits.TXIE = 0;
    }
    pxMBFrameCBTransmitterEmpty();
}

void Custom_EUSART_Receive_ISR(void)
{
    
    eusartRxStatusBuffer[eusartRxHead].status = 0;

    if(RCSTAbits.FERR){
        eusartRxStatusBuffer[eusartRxHead].ferr = 1;
        EUSART_FramingErrorHandler();
    }

    if(RCSTAbits.OERR){
        eusartRxStatusBuffer[eusartRxHead].oerr = 1;
        EUSART_OverrunErrorHandler();
    }
    
    if(eusartRxStatusBuffer[eusartRxHead].status){
        EUSART_ErrorHandler();
    } else {
        EUSART_RxDataHandler();
        pxMBFrameCBByteReceived();
    }
    
    // or set custom function using EUSART_SetRxInterruptHandler()
}