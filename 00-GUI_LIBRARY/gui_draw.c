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
#include "gui_draw.h"

/******************************************************************************/
/******************************************************************************/
/***                           Private structures                            **/
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/******************************************************************************/
/***                           Private definitions                           **/
/******************************************************************************/
/******************************************************************************/
#define GUI_USE_CLIPPING        1

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
//Get string width
GUI_Dim_t __StringWidth(GUI_Const GUI_FONT_t* font, const char* str) {
    GUI_Dim_t out = 0;
    while (*str) {
        out += font->Data[(*str) - font->StartChar].xSize;
        out += font->Data[(*str) - font->StartChar].xMargin;
        str++;
    }
    
    return out;
}

/* Draw character to screen */
/* X and Y coordinates are TOP LEFT coordinates for character */
void __DRAW_Char(GUI_Display_t* disp, GUI_Const GUI_FONT_t* font, GUI_DRAW_FONT_t* draw, GUI_Dim_t x, GUI_Dim_t y, const GUI_FONT_CharInfo_t* c) {
    GUI_Byte i, b;
    GUI_Dim_t x1;
    GUI_iByte k;
    GUI_Byte columns;
    
    y += c->yPos;                                   /* Set Y position */
    
    if (font->Flags & GUI_FLAG_FONT_AA) {           /* Font has anti alliasing enabled */
        GUI_Color_t color;                          /* Temporary color for AA */
        GUI_Byte tmp, r1, g1, b1;
        
        columns = c->xSize / 4;                     /* Calculate number of bytes used for single character line */
        if (c->xSize % 4) {                         /* If only 1 column used */
            columns++;
        }   
        
        for (i = 0; i < columns * c->ySize; i++) {  /* Go through all data bytes */
            if (y < disp->Y1 || y > disp->Y2) {     /* Do not draw when we are outside clipping are */
                goto draw_char_handle_y_aa;
            }
            b = c->Data[i];                         /* Get character byte */
            for (k = 0; k < 4; k++) {               /* Scan each bit in byte */
                GUI_Color_t baseColor;
                x1 = x + (i % columns) * 4 + k;     /* Get new X value for pixel draw */
                if (x1 < disp->X1 || x1 > disp->X2) {
                    continue;
                }
                if (x1 < (draw->X + draw->Color1Width)) {
                    baseColor = draw->Color1;
                } else {
                    baseColor = draw->Color2;
                }
                tmp = (b >> (6 - 2 * k)) & 0x03;    /* Get temporary bits on bottom */
                if (tmp == 0x03) {                  /* Draw solid color if both bits are enabled */
                    GUI_DRAW_SetPixel(disp, x1, y, baseColor);
                } else if (tmp) {                   /* Calculate new color */
                    float t = (float)tmp / 3.0f;
                    color = GUI_DRAW_GetPixel(disp, x1, y); /* Read current color */
                    
                    /* Calculate new values for pixel */
                    r1 = (float)t * (float)((color >> 16) & 0xFF) + (float)(1.0f - (float)t) * (float)((baseColor >> 16) & 0xFF);
                    g1 = (float)t * (float)((color >>  8) & 0xFF) + (float)(1.0f - (float)t) * (float)((baseColor >>  8) & 0xFF);
                    b1 = (float)t * (float)((color >>  0) & 0xFF) + (float)(1.0f - (float)t) * (float)((baseColor >>  0) & 0xFF);
                    
                    /* Draw actual pixel to screen */
                    GUI_DRAW_SetPixel(disp, x1, y, r1 << 16 | g1 << 8 | b1);
                }
            }
draw_char_handle_y_aa:
            if ((i % columns) == (columns - 1)) {   /* Check for next line */
                y++;
            }
        }
    } else {
        columns = c->xSize / 8;
        if (c->xSize % 8) {
            columns++;
        }
        for (i = 0; i < columns * c->ySize; i++) {  /* Go through all data bytes */
            if (y < disp->Y1 || y > disp->Y2) {     /* Do not draw when we are outside clipping are */
                goto draw_char_handle_y;
            }
            b = c->Data[i];                         /* Get character byte */
            for (k = 0; k < 8; k++) {               /* Scan each bit in byte */
                if (b & (1 << (7 - k))) {           /* If bit is set, draw pixel */
                    x1 = x + (i % columns) * 8 + k; /* Get new X value for pixel draw */
                    if (x1 < disp->X1 || x1 > disp->X2) {
                        continue;
                    }
                    if (x1 <= (draw->X + draw->Color1Width)) {
                        GUI_DRAW_SetPixel(disp, x1, y, draw->Color1);
                    } else {
                        GUI_DRAW_SetPixel(disp, x1, y, draw->Color2);
                    }
                }
            }
draw_char_handle_y:
            if ((i % columns) == (columns - 1)) {   /* Check for next line */
                y++;
            }
        }
    }
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

/******************************************************************************/
/******************************************************************************/
/***                    Functions with low-level communication               **/
/******************************************************************************/
/******************************************************************************/
void GUI_DRAW_FillScreen(GUI_Display_t* disp, GUI_Color_t color) {
    GUI.LL.Fill(&GUI.LCD, GUI.LCD.DrawingLayer, 0, GUI.LCD.Width, GUI.LCD.Height, 0, color);
}

void GUI_DRAW_Fill(GUI_Display_t* disp, GUI_Dim_t x, GUI_Dim_t y, GUI_Dim_t width, GUI_Dim_t height, GUI_Color_t color) {
#if GUI_USE_CLIPPING
    if (                                            /* Check if redraw is inside area */
        (x + width) < disp->X1 ||                   /* Too left */
        (y + height) < disp->Y1 ||                  /* Too top */
        x > disp->X2 ||                             /* Too right */
        y > disp->Y2                                /* Too bottom */
        ) {
        return;
    }
    
    /* We are in region */
    if (x < disp->X1) {
        width -= (disp->X1 - x);
        x = disp->X1;
    }
    if (y < disp->Y1) {
        height -= (disp->Y1 - y);
        y = disp->Y1;
    }
    
    /* Check out of regions */
    if ((x + width) > disp->X2) {
        width = disp->X2 - x;
    }
    if ((y + height) > disp->Y2) {
        height = disp->Y2 - y;
    }
#endif
    GUI.LL.FillRect(&GUI.LCD, GUI.LCD.DrawingLayer, x, y, width, height, color);
}

void GUI_DRAW_SetPixel(GUI_Display_t* disp, GUI_Dim_t x, GUI_Dim_t y, GUI_Color_t color) {
#if GUI_USE_CLIPPING
    if (y < disp->Y1 || y > disp->Y2 || x < disp->X1 || x > disp->X2) {
        return;
    }
#endif
    GUI.LL.SetPixel(&GUI.LCD, GUI.LCD.DrawingLayer, x, y, color);
}

GUI_Color_t GUI_DRAW_GetPixel(GUI_Display_t* disp, GUI_Dim_t x, GUI_Dim_t y) {
    return GUI.LL.GetPixel(&GUI.LCD, GUI.LCD.DrawingLayer, x, y);
}

void GUI_DRAW_VLine(GUI_Display_t* disp, GUI_Dim_t x, GUI_Dim_t y, GUI_Dim_t length, GUI_Color_t color) {
#if GUI_USE_CLIPPING
    if (x > disp->X2 || x < disp->X1 || y > disp->Y2 || (y + length) < disp->Y1) {
        return;
    }
    if (y < disp->Y1) {
        length -= disp->Y1 - y;
        y = disp->Y1;
    }
    if ((y + length) > disp->Y2) {
        length = disp->Y2 - y;
    }
#endif
    GUI.LL.DrawVLine(&GUI.LCD, GUI.LCD.DrawingLayer, x, y, length, color);
}

void GUI_DRAW_HLine(GUI_Display_t* disp, GUI_Dim_t x, GUI_Dim_t y, GUI_Dim_t length, GUI_Color_t color) {
#if GUI_USE_CLIPPING
    if (y > disp->Y2 || y < disp->Y1 || x > disp->X2 || (x + length) < disp->X1) {
        return;
    }
    if (x < disp->X1) {
        length -= disp->X1 - x;
        x = disp->X1;
    }
    if ((x + length) > disp->X2) {
        length = disp->X2 - x;
    }
#endif
    GUI.LL.DrawHLine(&GUI.LCD, GUI.LCD.DrawingLayer, x, y, length, color);
}

/******************************************************************************/
/******************************************************************************/
/***                          Functions for primitives                       **/
/******************************************************************************/
/******************************************************************************/
void GUI_DRAW_Line(GUI_Display_t* disp, GUI_Dim_t x1, GUI_Dim_t y1, GUI_Dim_t x2, GUI_Dim_t y2, GUI_Color_t color) {
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
	curpixel = 0;

	deltax = __GUI_ABS(x2 - x1);
	deltay = __GUI_ABS(y2 - y1);
	x = x1;
	y = y1;
	
	if (x2 >= x1) {
		xinc1 = 1;
		xinc2 = 1;
	} else {
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) {
		yinc1 = 1;
		yinc2 = 1;
	} else {
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay) {
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	} else {
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++) {
		GUI_DRAW_SetPixel(disp, x, y, color);
		num += numadd;
		if (num >= den) {
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}

void GUI_DRAW_Rectangle(GUI_Display_t* disp, GUI_Dim_t x, GUI_Dim_t y, GUI_Dim_t width, GUI_Dim_t height, GUI_Color_t color) {
    if (width == 0 || height == 0) {
        return;
    }
    GUI_DRAW_HLine(disp, x,             y,              width,  color);
    GUI_DRAW_VLine(disp, x,             y,              height, color);
    
    GUI_DRAW_HLine(disp, x,             y + height - 1, width,  color);
    GUI_DRAW_VLine(disp, x + width - 1, y,              height, color);
}

void GUI_DRAW_FilledRectangle(GUI_Display_t* disp, GUI_Dim_t x, GUI_Dim_t y, GUI_Dim_t width, GUI_Dim_t height, GUI_Color_t color) {
    GUI_DRAW_Fill(disp, x, y, width, height, color);
}

void GUI_DRAW_Rectangle3D(GUI_Display_t* disp, GUI_Dim_t x, GUI_Dim_t y, GUI_Dim_t width, GUI_Dim_t height, GUI_DRAW_3D_State_t state) {
    GUI_Color_t c1, c2, c3;
    
    c1 = GUI_COLOR_BLACK;
    if (state == GUI_DRAW_3D_State_Raised) {
        c2 = 0xFFAAAAAA;
        c3 = 0xFF555555;
    } else {
        c2 = 0xFF555555;
        c3 = 0xFFAAAAAA;
    }
    
    GUI_DRAW_Rectangle(disp, x, y, width, height, c1);
    
    GUI_DRAW_HLine(disp, x + 1, y + 1, width - 2, c2);
    GUI_DRAW_VLine(disp, x + 1, y + 1, height - 2, c2);
    
    GUI_DRAW_HLine(disp, x + 1, y + height - 2, width - 2,  c3);
    GUI_DRAW_VLine(disp, x + width - 2, y + 1, height - 2, c3);
}

void GUI_DRAW_RoundedRectangle(GUI_Display_t* disp, GUI_Dim_t x, GUI_Dim_t y, GUI_Dim_t width, GUI_Dim_t height, GUI_Dim_t r, GUI_Color_t color) {
    if (r >= (height / 2)) {
        r = height / 2;
    }
    if (r) {
        GUI_DRAW_HLine(disp, x + r,         y,              width - 2 * r,  color);
        GUI_DRAW_VLine(disp, x + width - 1, y + r,          height - 2 * r, color);
        GUI_DRAW_HLine(disp, x + r,         y + height - 1, width - 2 * r,  color);
        GUI_DRAW_VLine(disp, x,             y + r,          height - 2 * r, color);
        
        GUI_DRAW_CircleCorner(disp, x + r,             y + r,              r, GUI_DRAW_CIRCLE_TL, color);
        GUI_DRAW_CircleCorner(disp, x + width - r - 1, y + r,              r, GUI_DRAW_CIRCLE_TR, color);
        GUI_DRAW_CircleCorner(disp, x + r,             y + height - r - 1, r, GUI_DRAW_CIRCLE_BL, color);
        GUI_DRAW_CircleCorner(disp, x + width - r - 1, y + height - r - 1, r, GUI_DRAW_CIRCLE_BR, color);
    } else {
        GUI_DRAW_Rectangle(disp, x, y, width, height, color);
    }
}

void GUI_DRAW_FilledRoundedRectangle(GUI_Display_t* disp, GUI_Dim_t x, GUI_Dim_t y, GUI_Dim_t width, GUI_Dim_t height, GUI_Dim_t r, GUI_Color_t color) {
    if (r >= (height / 2)) {
        r = height / 2 - 1;
    }
    if (r) {
        GUI_DRAW_FilledRectangle(disp, x + r,         y,     width - 2 * r, height,         color);
        GUI_DRAW_FilledRectangle(disp, x,             y + r, r,             height - 2 * r, color);
        GUI_DRAW_FilledRectangle(disp, x + width - r, y + r, r,             height - 2 * r, color);
        
        GUI_DRAW_FilledCircleCorner(disp, x + r,             y + r,              r, GUI_DRAW_CIRCLE_TL, color);
        GUI_DRAW_FilledCircleCorner(disp, x + width - r - 1, y + r,              r, GUI_DRAW_CIRCLE_TR, color);
        GUI_DRAW_FilledCircleCorner(disp, x + r,             y + height - r - 1, r, GUI_DRAW_CIRCLE_BL, color);
        GUI_DRAW_FilledCircleCorner(disp, x + width - r - 1, y + height - r - 1, r, GUI_DRAW_CIRCLE_BR, color);
    } else {
        GUI_DRAW_FilledRectangle(disp, x, y, width, height, color);
    }
}

void GUI_DRAW_Circle(GUI_Display_t* disp, GUI_Dim_t x, GUI_Dim_t y, GUI_Dim_t r, GUI_Color_t color) {
    GUI_DRAW_CircleCorner(disp, x, y, r, GUI_DRAW_CIRCLE_TL, color);
    GUI_DRAW_CircleCorner(disp, x - 1, y, r, GUI_DRAW_CIRCLE_TR, color);
    GUI_DRAW_CircleCorner(disp, x, y - 1, r, GUI_DRAW_CIRCLE_BL, color);
    GUI_DRAW_CircleCorner(disp, x - 1, y - 1, r, GUI_DRAW_CIRCLE_BR, color);
}

void GUI_DRAW_FilledCircle(GUI_Display_t* disp, GUI_Dim_t x, GUI_Dim_t y, GUI_Dim_t r, GUI_Color_t color) {
    GUI_DRAW_FilledCircleCorner(disp, x, y, r, GUI_DRAW_CIRCLE_TL, color);
    GUI_DRAW_FilledCircleCorner(disp, x - 1, y, r, GUI_DRAW_CIRCLE_TR, color);
    GUI_DRAW_FilledCircleCorner(disp, x, y - 1, r, GUI_DRAW_CIRCLE_BL, color);
    GUI_DRAW_FilledCircleCorner(disp, x - 1, y - 1, r, GUI_DRAW_CIRCLE_BR, color);
}

void GUI_DRAW_Triangle(GUI_Display_t* disp, GUI_Dim_t x1, GUI_Dim_t y1,  GUI_Dim_t x2, GUI_Dim_t y2, GUI_Dim_t x3, GUI_Dim_t y3, GUI_Color_t color) {

}

void GUI_DRAW_FilledTriangle(GUI_Display_t* disp, GUI_Dim_t x1, GUI_Dim_t y1, GUI_Dim_t x2, GUI_Dim_t y2, GUI_Dim_t x3, GUI_Dim_t y3, GUI_Color_t color) {

}

void GUI_DRAW_CircleCorner(GUI_Display_t* disp, GUI_iDim_t x0, GUI_iDim_t y0, GUI_iDim_t r, GUI_Byte_t c, GUI_Color_t color) {
	GUI_iDim_t f = 1 - r;
	GUI_iDim_t ddF_x = 1;
	GUI_iDim_t ddF_y = -2 * r;
	GUI_iDim_t x = 0;
	GUI_iDim_t y = r;
    
#if GUI_USE_CLIPPING
    if (!__GUI_RECT_MATCH(
        disp->X1, disp->Y1, disp->X2 - disp->X1, disp->Y2 - disp->Y1,
        x0 - r, y0 - r, 2 * r, 2 * r
    )) {
        return;
    }
#endif

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (c & GUI_DRAW_CIRCLE_TL) {               /* Top left corner */
			GUI_DRAW_SetPixel(disp, x0 - y, y0 - x, color);
			GUI_DRAW_SetPixel(disp, x0 - x, y0 - y, color);
		}
		
        if (c & GUI_DRAW_CIRCLE_TR) {               /* Top right corner */
			GUI_DRAW_SetPixel(disp, x0 + x, y0 - y, color);
			GUI_DRAW_SetPixel(disp, x0 + y, y0 - x, color);
		}
		
		if (c & GUI_DRAW_CIRCLE_BR) {               /* Bottom right corner */
			GUI_DRAW_SetPixel(disp, x0 + x, y0 + y, color);
			GUI_DRAW_SetPixel(disp, x0 + y, y0 + x, color);
		}
		
        if (c & GUI_DRAW_CIRCLE_BL) {               /* Bottom left corner */
			GUI_DRAW_SetPixel(disp, x0 - x, y0 + y, color);
			GUI_DRAW_SetPixel(disp, x0 - y, y0 + x, color);
		}
    }
}

void GUI_DRAW_FilledCircleCorner(GUI_Display_t* disp, GUI_iDim_t x0, GUI_iDim_t y0, GUI_iDim_t r, GUI_Byte_t c, GUI_Color_t color) {
    GUI_iDim_t f = 1 - r;
    GUI_iDim_t ddF_x = 1;
    GUI_iDim_t ddF_y = -2 * r;
    GUI_iDim_t x = 0;
    GUI_iDim_t y = r;
    
#if GUI_USE_CLIPPING
    if (!__GUI_RECT_MATCH(
        disp->X1, disp->Y1, disp->X2 - disp->X1, disp->Y2 - disp->Y1,
        x0 - r, y0 - r, 2 * r, 2 * r
    )) {
        return;
    }
#endif

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (c & GUI_DRAW_CIRCLE_TL) {               /* Top left */            
            GUI_DRAW_HLine(disp, x0 - x, y0 - y, x, color);
            GUI_DRAW_HLine(disp, x0 - y, y0 - x, y, color);
        }
        if (c & GUI_DRAW_CIRCLE_TR) {               /* Top right */
            GUI_DRAW_HLine(disp, x0, y0 - y, x, color);
            GUI_DRAW_HLine(disp, x0, y0 - x, y, color);
        }
        if (c & GUI_DRAW_CIRCLE_BL) {               /* Bottom left */
            GUI_DRAW_HLine(disp, x0 - y, y0 + x, y, color);
            GUI_DRAW_HLine(disp, x0 - x, y0 + y, x, color);
        }
        if (c & GUI_DRAW_CIRCLE_BR) {               /* Bottom right */
            GUI_DRAW_HLine(disp, x0, y0 + x, y, color);
            GUI_DRAW_HLine(disp, x0, y0 + y, x, color);
        }
    }
}

typedef struct GUI_DRAW_Poly_t {
    GUI_iDim_t X;
    GUI_iDim_t Y;
} GUI_DRAW_Poly_t;

void GUI_DRAW_Poly(GUI_Display_t* disp, GUI_DRAW_Poly_t* points, GUI_Byte len, GUI_Color_t color) {
    GUI_iDim_t x = 0, y = 0;

    if (len < 2) {
        return;
    }

    GUI_DRAW_Line(disp, points->X, points->Y, (points + len - 1)->X, (points + len - 1)->Y, color);

    while (--len) {
        x = points->X;
        y = points->Y;
        points++;
        GUI_DRAW_Line(disp, x, y, points->X, points->Y, color);
    }
}

void GUI_DRAW_WriteText(GUI_Display_t* disp, GUI_Const GUI_FONT_t* font, const char* str, GUI_DRAW_FONT_t* draw) {
    GUI_Dim_t w, x, y;
    const GUI_FONT_CharInfo_t* c;
    
    w = __StringWidth(font, str);                   /* Get string width for this box */
    
    if (w > draw->Width) {                          /* If string is wider than available */
        w = draw->Width;                            /* Strip text width to available */
    }
    
    x = draw->X;                                    /* Get start X position */
    y = draw->Y;                                    /* Get start Y position */
    
    if (draw->Align & GUI_VALIGN_CENTER) {          /* Check for vertical align center */
        y += (draw->Height - font->Size) / 2;       /* Align center of drawing area */
    } else if (draw->Align & GUI_VALIGN_BOTTOM) {   /* Check for vertical align bottom */
        y += draw->Height - font->Size;             /* Align bottom of drawing area */
    }
    
    if (draw->Align & GUI_HALIGN_CENTER) {          /* Check for horizontal align center */
        x += (draw->Width - w) / 2;                 /* Align center of drawing area */
    } else if (draw->Align & GUI_HALIGN_RIGHT) {    /* Check for horizontal align right */
        x += draw->Width - w;                       /* Align right of drawing area */
    }
    
    while (*str) {                                  /* Go through entire string */
        c = &font->Data[(*str) - font->StartChar];  /* Get char informations */
        
        if (w < (c->xSize + c->xMargin)) {          /* Check available width */
            break;                                  /* Stop execution right now */
        }
        
        __DRAW_Char(disp, font, draw, x, y, c);     /* Draw actual char */
        
        x += c->xSize + c->xMargin;                 /* Increase X position */
        w -= c->xSize + c->xMargin;                 /* Decrease available width for char */
        str++;
    } 
}
