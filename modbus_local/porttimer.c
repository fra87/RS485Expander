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

// Postscaler value for the timer; timer period will be fixed to 20ms when using
// postscaler
static uint8_t postScalerReloadValue = 0;
// Variable counting the postscaler hits
static volatile uint8_t postScalerCount;

// Forward declaration
void timer1ISR(void);

BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    // Set postscaler if period is greater than maximum
    if (usTim1Timerout50us > 2048)
    {
        // Use postscaler to increase the timer range; resolution of the timer
        // will be 20ms instead of 50us
        
        // usTim1Timerout50us will be rounded by excess
        postScalerReloadValue = (usTim1Timerout50us-1) / 400;
        usTim1Timerout50us = 400;
    }
    else
    {
        postScalerReloadValue = 0;
    }
    
    postScalerCount = postScalerReloadValue;
    
    // Set the interrupt handler
    TMR1_SetInterruptHandler(timer1ISR);
    
    // Overwrite old value of timer1ReloadVal, to change the timer time
    extern volatile uint16_t timer1ReloadVal;
    timer1ReloadVal = (uint16_t)(0x10000 - ((uint32_t)usTim1Timerout50us) * 25);
    // Apply the reload value
    TMR1_Reload();
    
    return TRUE;
}

void vMBPortTimersEnable(void)
{
    TMR1_Reload();
    TMR1_StartTimer();
}

void vMBPortTimersDisable(void)
{
    TMR1_StopTimer();
}

void vMBPortTimersDelay(USHORT usTimeOutMS)
{
    // Function is never called (since MB_ASCII_TIMEOUT_WAIT_BEFORE_SEND_MS = 0)
    (void)usTimeOutMS;
}

void timer1ISR(void)
{
    if (postScalerCount)
    { // Reduce postScalerCount
        postScalerCount--;
    }
    else
    { // Post scaler elapsed; let's reload it and fire the event
        postScalerCount = postScalerReloadValue;
        pxMBPortCBTimerExpired();
    }
}
