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

#include "../modbus/include/mb.h"
#include "../modbus/include/mbport.h"

// Variable to hold the event
static eMBEventType _queuedEvent;

// TRUE if an event is present in _queuedEvent
static BOOL _eventPresent;

BOOL xMBPortEventInit(void)
{
    // Init the queue to be empty
    _eventPresent = FALSE;
    
    // init was successful
    return TRUE;
}

BOOL xMBPortEventPost(eMBEventType eEvent)
{
    // Save the event
    _queuedEvent = eEvent;
    // and mark it as present
    _eventPresent = TRUE;

    // post was succesful
    return TRUE;
}

BOOL xMBPortEventGet(eMBEventType *eEvent)
{
    // Save the status
    BOOL eventWasPresent = _eventPresent;

    // If there was an event
    if(_eventPresent)
    {
        // return it
        *eEvent = _queuedEvent;
        // and reset the queue
        _eventPresent = FALSE;
    }
    
    // return whether the event was present
    return eventWasPresent;
}
