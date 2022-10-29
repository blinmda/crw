#define _CRT_SECURE_NO_WARNINGS
#include "draw.h"

static HDC hDC;  //
static HGLRC hRC;// дескрипторы, по которым система определяет окно, в котором мы рисуем

float cellSize = 0.028f; // размер клетки кроссворда
float fontSize = 0.02f;  // размер шрифта кроссворда
float y_size = 0;        // y_size -- текущий размер по вертикали (при изменении размеров окна)
int max_cells_x, max_cells_y; // параметры, описывающие максимальную ширину и высоту кроссворда для данного размера окна
                             
void EnableOpenGL(HWND hwnd)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;
    /* get the device context (DC) */
    hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(hDC, &pfd);

    SetPixelFormat(hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    hRC = wglCreateContext(hDC);

    wglMakeCurrent(hDC, hRC);
}

void WndResize(int x, int y) 
{
    static int width = 0, height = 0; // статические переменные, сохраняют значения между вызовами функции

    if (width && height) // если уже были инициализированны размеры окна
    {
        cellSize *= ((float)width / x + (float)height / y) / 2;
        fontSize *= ((float)width / x + (float)height / y) / 2;
    }

    width = x, height = y;
    glViewport(0, 0, x, y);
    float k = x / (float)y;
    

    glLoadIdentity();
    glScalef(1, k, 1);
    y_size = 1 / k;

    max_cells_x = 1.95f / cellSize;
    max_cells_y = (2 * y_size - 0.05f) / cellSize;
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}


HDC init_gl(HWND hwnd)
{
	EnableOpenGL(hwnd);

    RECT rct;
    GetClientRect(hwnd, &rct);
    WndResize(rct.right, rct.bottom);

    glClear(GL_COLOR_BUFFER_BIT);

    SwapBuffers(hDC);

    return hDC;
}


void draw(HDC hdc, parameters params, int width, int height)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPushMatrix();

    glBegin(GL_LINES);

    glColor3f(1.0f, 1.0f, 1.0f);

    float widthMargin = (2.0f - params.width * cellSize) / 2 - 1.0f;
    float heightMargin = (y_size * 2 - params.height * cellSize) / 2 - y_size;
    float wth = 0, hht = 0; // снижение мощности
    for (int i = 0; i <= params.width; i++)
    {
        glVertex2f(widthMargin + wth, heightMargin);
        glVertex2f(widthMargin + wth, -heightMargin);
        wth += cellsize;
    }
    for (int i = 0; i <= params.height; i++)
    {
        glVertex2f(widthMargin, heightMargin + hht);
        glVertex2f(-widthMargin, heightMargin + hht);
        hht += cellsize;
    }

    glEnd();


    glPopMatrix();

    struct Crossword cw = getCrossword();
    float cornerX = widthMargin + (cellSize - fontSize) / 2;
    float cornerY = heightMargin + cw.height * cellSize -fontSize- (cellSize - fontSize) / 2;
    for (int i = 0; i < params.height; i++)
        for (int j = 0; j < params.width; j++)
        {
            char mb[4]; wctomb(mb, cw.map[i][j]);

            if ((i < params.height - 1 && cw.map[i + 1][j] != L' ' || i > 0 && cw.map[i - 1][j] != L' ')
                && (j > 0 && cw.map[i][j - 1] != L' ' || j < params.width - 1 && cw.map[i][j + 1] != L' '))

                glColor3f(1.0f, 0.f, 0.f);

            else
                glColor3f(1.f, 1.f, 1.f);

            drawChar(
                     mb[0],
                     fontSize, 
                     cornerX + j * cellSize,
                     cornerY - i * cellSize
                );
        }

    SwapBuffers(hdc);
}

