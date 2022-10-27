#pragma once
// здесь объ€вл€етс€ всЄ про отрисовку кроссворда на экране 
#include <Windows.h>
#include "glut.h"
#include "crossword.h"
#include "draw_text.h"

#pragma comment(lib, "glut32.lib")

HDC init_gl(HWND hwnd);
void WndResize(int x, int y);

void draw(HDC hdc, parameters params, int, int);

extern int max_cells_x, max_cells_y;