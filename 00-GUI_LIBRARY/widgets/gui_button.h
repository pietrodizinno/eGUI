/**
 * \author  Tilen Majerle
 * \email   tilen@majerle.eu
 * \brief   GUI button widget
 *	
\verbatim
   ----------------------------------------------------------------------
    Copyright (c) 2016 Tilen Majerle

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software, 
    and to permit persons to whom the Software is furnished to do so, 
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
    AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------
\endverbatim
 */
#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif
    
/**
 * \addtogroup      GUI_WIDGETS
 * \{
 */
    
#include "gui_widget.h"

/**
 * \defgroup        GUI_BUTTON
 * \{
 */

/**
 * \defgroup        GUI_BUTTON_Macros
 * \{
 */

#define GUI_BUTTON_COLOR_FG             0   /*!< Foreground color index in array */
#define GUI_BUTTON_COLOR_BG             1   /*!< Background color index in array */
#define GUI_BUTTON_COLOR_BORDER         2   /*!< Border color index in array */

/**
 * \} GUI_BUTTON_Macros
 */
    
/**
 * \defgroup        GUI_BUTTON_Typedefs
 * \{
 */

/**
 * \brief           GUI button structure
 */
typedef struct GUI_BUTTON_t {
    GUI_HANDLE C;                           /*!< GUI handle object, must always be first on list */
    
    GUI_Color_t Color[3];                   /*!< List of colors */
    GUI_Gradient_t Gradient[2];             /*!< List of gradient colors */
    
    GUI_Dim_t BorderWidth;                  /*!< Border width */
    GUI_Dim_t BorderRadius;                 /*!< Border radius */
    
    void (*Callback)(struct GUI_BUTTON_t *, uint16_t);  /* Widget callback function */
} GUI_BUTTON_t;

/**
 * \} GUI_BUTTON_Typedefs
 */
    
/**
 * \defgroup        GUI_BUTTON_Functions
 * \brief           Library Functions
 * \{
 */

/**
 * \brief           Initializes button widget
 * \note            This function should not be called by user
 */
GUI_HANDLE_t GUI_BUTTON_Create(GUI_ID_t id, GUI_iDim_t x, GUI_iDim_t y, GUI_Dim_t width, GUI_Dim_t height);
void GUI_BUTTON_Remove(GUI_HANDLE_t* ptr);
GUI_HANDLE_t GUI_BUTTON_Invalidate(GUI_HANDLE_t ptr);
GUI_HANDLE_t GUI_BUTTON_SetText(GUI_HANDLE_t ptr, const char* text);
GUI_HANDLE_t GUI_BUTTON_SetSize(GUI_HANDLE_t ptr, GUI_Dim_t width, GUI_Dim_t height);
GUI_HANDLE_t GUI_BUTTON_SetXY(GUI_HANDLE_t ptr, GUI_iDim_t x, GUI_iDim_t y);
GUI_HANDLE_t GUI_BUTTON_SetColor(GUI_HANDLE_t ptr, uint8_t index, GUI_Color_t color);
GUI_HANDLE_t GUI_BUTTON_SetBorderWidth(GUI_HANDLE_t ptr, GUI_Dim_t width);
GUI_HANDLE_t GUI_BUTTON_SetBorderRadius(GUI_HANDLE_t ptr, GUI_Dim_t size);
GUI_HANDLE_t GUI_BUTTON_SetFont(GUI_HANDLE_t ptr, GUI_Const GUI_FONT_t* font);

//Returns number of bytes allocated
uint32_t GUI_BUTTON_AllocTextMemory(GUI_HANDLE_t ptr, uint8_t size);
GUI_HANDLE_t GUI_BUTTON_FreeTextMemory(GUI_HANDLE_t ptr);
    
/**
 * \} GUI_BUTTON_Functions
 */
    
/**
 * \} GUI_BUTTON
 */
 
/**
 * \} GUI_WIDGETS
 */

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif
