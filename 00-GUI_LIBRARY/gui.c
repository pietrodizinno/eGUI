/**	
 * |----------------------------------------------------------------------
 * | Copyright (c) 2016 Tilen Majerle
 * |  
 * | Permission is hereby granted, free of charge, to any person
 * | obtaining a copy of this software and associated documentation
 * | files (the "Software"), to deal in the Software without restriction,
 * | including without limitation the rights to use, copy, modify, merge,
 * | publish, distribute, sublicense, and/or sell copies of the Software, 
 * | and to permit persons to whom the Software is furnished to do so, 
 * | subject to the following conditions:
 * | 
 * | The above copyright notice and this permission notice shall be
 * | included in all copies or substantial portions of the Software.
 * | 
 * | THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * | EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * | OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * | AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * | HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * | WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * | FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * | OTHER DEALINGS IN THE SOFTWARE.
 * |----------------------------------------------------------------------
 */
#include "gui.h"

/******************************************************************************/
/******************************************************************************/
/***                           Private structures                            **/
/******************************************************************************/
/******************************************************************************/
GUI_t GUI;

/******************************************************************************/
/******************************************************************************/
/***                           Private definitions                           **/
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/******************************************************************************/
/***                            Private variables                            **/
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/******************************************************************************/
/***                            Private functions                            **/
/******************************************************************************/
/******************************************************************************/
//Gets number of widgets waiting for redraw
uint32_t __GetNumberOfPendingWidgets(GUI_HANDLE_t parent) {
    GUI_HANDLE_t h;
    uint32_t cnt = 0;
    
    if (parent && parent->Flags & GUI_FLAG_REDRAW) {    /* Check for specific widget */
        return 1;                                   /* We have object to redraw */
    }
    for (h = __GUI_LINKEDLIST_GetNextWidget((GUI_HANDLE_ROOT_t *)parent, 0); h; h = __GUI_LINKEDLIST_GetNextWidget((GUI_HANDLE_ROOT_t *)parent, h)) {
        if (h->Widget->MetaData.AllowChildren) {    /* If this widget has children elements */
            cnt += __GetNumberOfPendingWidgets(h);  /* Redraw this widget and all its children if required */
        } else if (h->Flags & GUI_FLAG_REDRAW) {    /* Check if we need redraw */
            cnt++;                                  /* Increase number of elements to redraw */
        }
    }
    return cnt;
}

//Draws widgets
uint32_t __RedrawWidgets(GUI_HANDLE_t parent) {
    GUI_HANDLE_t h;
    uint32_t cnt = 0;
    
    if (parent && (parent->Flags & GUI_FLAG_REDRAW)) {  /* Check if parent window should redraw operation */
        parent->Flags &= ~GUI_FLAG_REDRAW;          /* Clear flag */
        for (h = __GUI_LINKEDLIST_GetNextWidget((GUI_HANDLE_ROOT_t *)parent, 0); h; h = __GUI_LINKEDLIST_GetNextWidget(NULL, h)) {
            h->Flags |= GUI_FLAG_REDRAW;            /* Set redraw bit to all children elements */
        }
        if (parent->Widget->WidgetDraw && __GUI_WIDGET_IsInsideClippingRegion(parent)) {    /* If draw function is set and drawing is inside clipping region */
            parent->Widget->WidgetDraw(&GUI.Display, parent);   /* Call drawing function */
        }
    }

    /* Go through all elements of parent */
    for (h = __GUI_LINKEDLIST_GetNextWidget((GUI_HANDLE_ROOT_t *)parent, 0); h; h = __GUI_LINKEDLIST_GetNextWidget(NULL, h)) {
        if (h->Widget->MetaData.AllowChildren) {    /* If this widget has children elements */
            cnt += __RedrawWidgets(h);              /* Redraw this widget and all its children if required */
        } else {
            if (h->Flags & GUI_FLAG_REDRAW) {       /* Check if redraw required */
                h->Flags &= ~GUI_FLAG_REDRAW;       /* Clear flag */
                if (h->Widget && h->Widget->WidgetDraw && __GUI_WIDGET_IsInsideClippingRegion(h)) { /* If draw function is set and drawing is inside clipping region */
                    h->Widget->WidgetDraw(&GUI.Display, h); /* Redraw widget */
                }
                cnt++;
            }
        }
    }
    
    return cnt;                                     /* Return number of redrawn objects */
}

__GUI_TouchStatus_t __ProcessTouch(GUI_TouchData_t* touch, GUI_TouchData_t* touchLast, GUI_HANDLE_t parent) {
    GUI_HANDLE_t h;
    __GUI_TouchStatus_t tStat;
    
    /* Check touches if any matches, go reverse on linked list */
    for (h = __GUI_LINKEDLIST_GetPrevWidget((GUI_HANDLE_ROOT_t *)parent, 0); h; h = __GUI_LINKEDLIST_GetPrevWidget((GUI_HANDLE_ROOT_t *)parent, h)) {
        GUI_Dim_t x, y;
        
        /* Check children elements first */
        if (h->Widget->MetaData.AllowChildren) {
            tStat = __ProcessTouch(touch, touchLast, h);    /* Process touch on widget elements first */
            if (tStat != touchCONTINUE) {           /* If we should not continue */
                return tStat;
            }
        }
        
        x = __GUI_WIDGET_GetAbsoluteX(h);           /* Get actual widget X position */
        y = __GUI_WIDGET_GetAbsoluteY(h);           /* Get actual widget Y position */
        
        /* Check if widget is in drawing area */
        if (touch->X >= x && touch->X <= (x + h->Width) && touch->Y >= y && touch->Y <= (y + h->Height)) {
            if (touch->Status && !touchLast->Status) {    /* Check for touchdown event */
                if (h->Widget->TouchEvents.TouchDown) {
                    tStat = h->Widget->TouchEvents.TouchDown(h, touch, touchCONTINUE);  /* Check for touch */
                    if (tStat != touchCONTINUE) {   /* If check was handled */
                        if (tStat == touchHANDLED) {    /* Touch handled for widget completelly */
                            if (GUI.FocusedWidget) {
                                GUI.FocusedWidget->Flags &= ~GUI_FLAG_FOCUS;    /* Clear focus flag */
                                __GUI_WIDGET_Invalidate(GUI.FocusedWidget); /* Invalidate widget for redraw */
                            }
                            GUI.FocusedWidget = GUI.ActiveWidget;   /* Set new focused widget */
                            
                            GUI.ActiveWidget = h;       /* Save active touch element */
                            __GUI_LINKEDLIST_MoveDown_Widget(h);    /* Move widget to end of list to be redrawn on top of everything and touch move detected first and fastest */
                            h->Flags |= GUI_FLAG_ACTIVE;    /* Set touch active flag */
                            __GUI_WIDGET_Invalidate(h); /* Invalidate widget and its parent */
                        } else {                    /* Touch handled with no focus */
                            if (GUI.FocusedWidget) {
                                GUI.FocusedWidget->Flags &= ~GUI_FLAG_FOCUS;    /* Clear focus flag */
                                __GUI_WIDGET_Invalidate(GUI.FocusedWidget); /* Invalidate widget for redraw */
                            }
                            GUI.FocusedWidget = NULL;   /* No focus widget anymore */
                            if (GUI.ActiveWidget) {
                                GUI.FocusedWidget->Flags &= ~GUI_FLAG_ACTIVE;   /* Clear focus flag */
                                __GUI_WIDGET_Invalidate(GUI.FocusedWidget); /* Invalidate widget for redraw */
                            }
                            GUI.ActiveWidget = NULL;    /* No active widget anymore */
                        }
                        return tStat;
                    }
                }
            } else if (!touch->Status && touchLast->Status) { /* Check for touchup event */
                if (h->Widget->TouchEvents.TouchUp) {
                    h->Widget->TouchEvents.TouchUp(h, touch, touchCONTINUE);
                }
                if (h == GUI.ActiveWidget) {
                    h->Flags &= ~GUI_FLAG_ACTIVE;   /* Remove active touch flag */
                    __GUI_WIDGET_Invalidate(h);     /* Invalidate widget */
                    GUI.ActiveWidget = NULL;
                }
            }
        }
    }
    return touchCONTINUE;                           /* Try with another widget */
}

/******************************************************************************/
/******************************************************************************/
/***                              Protothreads                               **/
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/******************************************************************************/
/***                                Public API                               **/
/******************************************************************************/
/******************************************************************************/
GUI_Result_t GUI_Init(void) {
    memset((void *)&GUI, 0x00, sizeof(GUI_t));      /* Reset GUI structure */
    
    /* Call LCD low-level function */
    GUI_LL_Init(&GUI.LCD, &GUI.LL);                 /* Call low-level initialization */
    GUI.LL.Init(&GUI.LCD);                          /* Call user LCD driver function */
    
    /* Draw LCD with default color */
    
    /* Check situation with layers */
    if (GUI.LCD.LayersCount == 1) {
        GUI.LCD.ActiveLayer = 0;
        GUI.LCD.DrawingLayer = 0;
        GUI.LL.Fill(&GUI.LCD, GUI.LCD.DrawingLayer, (void *)GUI.LCD.Layers[GUI.LCD.DrawingLayer].StartAddress, GUI.LCD.Width, GUI.LCD.Height, 0, 0xFFFFFFFF);
    } else if (GUI.LCD.LayersCount > 1) {
        GUI.LCD.ActiveLayer = 0;
        GUI.LCD.DrawingLayer = 0;
        GUI.LL.Fill(&GUI.LCD, GUI.LCD.DrawingLayer, (void *)GUI.LCD.Layers[GUI.LCD.DrawingLayer].StartAddress, GUI.LCD.Width, GUI.LCD.Height, 0, 0xFFFFFFFF);
        GUI.LCD.DrawingLayer = 1;
    } else {
        return guiERROR;
    }
    
    /* Init input devices */
    __GUI_INPUT_Init();
    
    /* Init widgets */
    __GUI_WIDGET_Init();
    
    return guiOK;
}

int32_t GUI_Process(void) {
    static uint8_t first = 1;
    static GUI_TouchData_t touchLast;
    int32_t cnt = 0;
    GUI_TouchData_t touch;
     
    if (first) {                                    /* Process first call */
        first = 0;
        memset(&touchLast, 0x00, sizeof(touchLast));
        touchLast.Status = GUI_TouchState_RELEASED; /* Start with released touch */
    }
    
    while (__GUI_INPUT_ReadTouch(&touch)) {         /* Process all touch events possible */
        /* If there is already an active touch */
        if (GUI.ActiveWidget && touch.Status && touchLast.Status) {
            /* Try to process mouse move event on this widget */
            if (GUI.ActiveWidget->Widget && GUI.ActiveWidget->Widget->TouchEvents.TouchMove) {
                GUI.ActiveWidget->Widget->TouchEvents.TouchMove(GUI.ActiveWidget, &touch, touchCONTINUE);
            }
        } else {
            /* Process other touches */
            if (__ProcessTouch(&touch, &touchLast, NULL) == touchHANDLED) {
                
            }
        }
        
        /* In case odd things happen, make sure there is no active widget */
        if (!touch.Status) {                        /* If touch released, release active widget */
            if (GUI.ActiveWidget) {                 /* If touch is active, clear flag */
                GUI.ActiveWidget->Flags &= ~GUI_FLAG_ACTIVE;    /* Remove active flag, touch/mouse not active */
                __GUI_WIDGET_Invalidate(GUI.ActiveWidget);  /* Invalidate widget */
            }
            GUI.ActiveWidget = 0;                   /* No active widget anymore */
        }
        memcpy((void *)&touchLast, (void *)&touch, sizeof(GUI_TouchData_t));/* Copy current touch to last touch status */
    }

    
    /* Check if anything new to redraw */
    if (!(GUI.LCD.Flags & GUI_FLAG_LCD_WAIT_LAYER_CONFIRM) && __GetNumberOfPendingWidgets(NULL)) {  /* Check if anything to draw first */
        uint32_t time;
        GUI_Byte active = GUI.LCD.ActiveLayer;
        GUI_Byte drawing = GUI.LCD.DrawingLayer;
        
        /* Copy current status from one layer to another */
        GUI.LL.Copy(&GUI.LCD, drawing, (void *)GUI.LCD.Layers[active].StartAddress, (void *)GUI.LCD.Layers[drawing].StartAddress, GUI.LCD.Width, GUI.LCD.Height, 0, 0);
            
        /* Actually draw new screen based on setup */
        time = TM_GENERAL_DWTCounterGetValue();
        cnt = __RedrawWidgets(NULL);                /* Redraw all widgets now */
        __GUI_DEBUG("Time: %u\r\n", TM_GENERAL_DWTCounterGetValue() - time);
        
//        GUI_DRAW_Rectangle(&GUI.Display, GUI.Display.X1, GUI.Display.Y1, GUI.Display.X2 - GUI.Display.X1, GUI.Display.Y2 - GUI.Display.Y1, GUI_COLOR_CYAN);
        
        /* Invalid clipping region */
        GUI.Display.X1 = 0xFFFF;
        GUI.Display.Y1 = 0xFFFF;
        GUI.Display.X2 = 0;
        GUI.Display.Y2 = 0;
        
        /* Set drawing layer as pending */
        GUI.LCD.Layers[drawing].Pending = 1;
        
        /* Notify low-level about layer change */
        GUI.LCD.Flags |= GUI_FLAG_LCD_WAIT_LAYER_CONFIRM;
        GUI_LL_Control(&GUI.LCD, GUI_LL_Command_SetActiveLayer, &drawing); /* Set new active layer to low-level driver */
        
        /* Swap active and drawing layers */
        /* New drawings won't be affected until confirmation from low-level is not received */
        GUI.LCD.ActiveLayer = drawing;
        GUI.LCD.DrawingLayer = active;
    }
    
    return cnt;                                     /* Return number of elements updated on GUI */
}

void GUI_LCD_ConfirmActiveLayer(GUI_Byte layer_num) {
    if ((GUI.LCD.Flags & GUI_FLAG_LCD_WAIT_LAYER_CONFIRM)) {/* If we have anything pending */
        GUI.LCD.Layers[layer_num].Pending = 0;
        GUI.LCD.Flags &= ~GUI_FLAG_LCD_WAIT_LAYER_CONFIRM;  /* Clear flag */
    }
}
