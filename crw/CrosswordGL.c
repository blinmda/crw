// CrosswordGUI.cpp : Определяет точку входа для приложения.
// попытка
//попытка моя
#define _CRT_SECURE_NO_WARNINGS
#include "resource.h"
#include "framework.h"
#include "crossword.h"
#include "draw.h"
#include <locale.h>
#include <stdio.h>
#include <time.h>

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр приложения
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Default(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Settings(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Dictionary(HWND, UINT, WPARAM, LPARAM);

parameters mainParams = {
                            DENSITY_HIGH,
                            15,
                            15
                        };
unsigned maxWordsCount = 1;
struct Dictionary ru_dict;
HDC hdc;
HWND hWnd;
FILE* fp = NULL;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    fp = fopen("time.txt", "w");
    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CROSSWORDGL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    setlocale(LC_ALL, "Russian");

    ru_dict = readDictionary(L"dict.txt");
    initCrossword(mainParams);
    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CROSSWORDGL));

    MSG msg;

    // Цикл обработки сообщений:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



// Регистрирует класс окна.
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CROSSWORDGL));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CROSSWORDGL);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//создает главное окно

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить идентификатор экземпляра в глобальной переменной

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);

   hdc = init_gl(hWnd);

   loadFontTexture("Verdana_B_alpha.png"); // загрузка текстуры, предназначенной для вывода символов на экран

   UpdateWindow(hWnd);

   return TRUE;
}

// Обрабатывает события в главном окне.

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, Default);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_SAVE:
                saveCrossword(hWnd);
                break;
            case IDM_LOAD:
            {
                parameters loaded = loadCrossword(hWnd);
                mainParams.width = loaded.width;
                mainParams.height = loaded.height;
                SendMessage(hWnd, WM_PRINT, 0, 0);
                break;
            }
            
            case IDM_HELP:
                MessageBox(hWnd, L"Для создания кроссворда выберите слова и их количество в пункте «Словарь», а также размер поля и плотность кроссворда в пункте «Настройки», затем нажмите кнопку «Сгенерировать».\n Сохранить или загрузить кроссворд можно в пункте «Файл».",
                    L"Помощь по использованию", MB_OK | MB_ICONINFORMATION);
                break;
            case IDM_SETTINGS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGSBOX), hWnd, Settings);
                break;
            case IDM_DICTIONARY:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_DICTBOX), hWnd, Dictionary);
                break;
            case IDM_GENERATE:
            {
                double time_spent = 0;
                clock_t begin = clock();
                initCrossword(mainParams);
                int cntSelected = 0;
                for (unsigned int i = 0; i < ru_dict.size; i++)
                    if (ru_dict.selected[i]) cntSelected++;

                struct Dictionary dict = {cntSelected, (wchar_t**)calloc(cntSelected, sizeof(wchar_t*)), NULL};
                for (unsigned int i = 0, j = 0; i < ru_dict.size; i++)
                    if (ru_dict.selected[i])
                        dict.words[j++] = ru_dict.words[i];

                fillCrossword(dict, mainParams.density, hWnd, hInst);
                SendMessage(hWnd, WM_PRINT, 0, 0);
                clock_t end = clock();
                time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
                fprintf(fp, "time: %f sec \n", time_spent);
                break;
            }
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PRINT:
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            hdc = BeginPaint(hWnd, &ps);
            RECT rect;
            GetClientRect(hWnd, &rect);
            draw(hdc, mainParams, rect.right - rect.left, rect.bottom - rect.top);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        WndResize(LOWORD(lParam), HIWORD(lParam));
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Дефолтный обработчик сообщений.
INT_PTR CALLBACK Default(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Обработчик сообщений для окна "Настройки".
INT_PTR CALLBACK Settings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    wchar_t buff[5];
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        
        
        /* 
            При открытии окна настроек заполняем поля текущими значениями
        */
        CheckDlgButton(hDlg, mainParams.density, 1);
        _itow_s(mainParams.width, buff, 5, 10);
        SetDlgItemTextW(hDlg, EDIT_WIDTH, buff);
        _itow_s(mainParams.height, buff, 5, 10);
        SetDlgItemTextW(hDlg, EDIT_HEIGHT, buff);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK: // нажатие кнопки ОК
        {
            // проверяем выбор плотности кроссворда
            if (IsDlgButtonChecked(hDlg, DENSITY_HIGH))
                mainParams.density = DENSITY_HIGH;
            else if (IsDlgButtonChecked(hDlg, DENSITY_MEDIUM))
                mainParams.density = DENSITY_MEDIUM;
            else if (IsDlgButtonChecked(hDlg, DENSITY_LOW))
                mainParams.density = DENSITY_LOW;

            wchar_t sz[16]; 
            GetDlgItemTextW(hDlg, EDIT_WIDTH, sz, 16); // считываем значение ширины 
            int inputWidth = _wtoi(sz); // конвертируем строку в число
            GetDlgItemTextW(hDlg, EDIT_HEIGHT, sz, 16); // то же для высоты 
            int inputHeight = _wtoi(sz);

            if (inputWidth <= 0 || inputWidth > max_cells_x ||
                inputHeight <= 0 || inputHeight > max_cells_y) // если введенные данные не корректны
            {
                char bf[200]; // выводим сообщение об ошибке
                sprintf(bf, "Допустимые параметры ширины: 0 < W <= %d\nДопустимые параметры высоты: 0 < H <= %d\n", max_cells_x, max_cells_y);
                MessageBoxA(NULL, bf, "Troubles with size", MB_ICONEXCLAMATION | MB_OK);
                
                break;
            }
            mainParams.height = inputHeight; // если всё ок, то сохраняем новые параметры 
            mainParams.width = inputWidth;
        }
        case IDCANCEL:
            initCrossword(mainParams);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        case DENSITY_HIGH:
        case DENSITY_MEDIUM:
        case DENSITY_LOW:
            CheckDlgButton(hDlg, DENSITY_HIGH, 0);
            CheckDlgButton(hDlg, DENSITY_MEDIUM, 0);
            CheckDlgButton(hDlg, DENSITY_LOW, 0);
            CheckDlgButton(hDlg, LOWORD(wParam), 1);
            break;
        }
        break;
    }
    SendMessage(hWnd, WM_PRINT, 0, 0);
    return (INT_PTR)FALSE;
}

// Обработчик сообщений для окна "Словарь".
INT_PTR CALLBACK Dictionary(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        {
        HWND hListBox = GetDlgItem(hDlg, IDM_DICTLISTBOX);
        
        for (unsigned int i = 0; i < ru_dict.size; i++) // заполняем список всеми словами из словаря
        {
            SendMessageW(hListBox, LB_ADDSTRING, 0, (LPARAM)ru_dict.words[i]);
        }
        wchar_t buff[10];
        _itow(maxWordsCount, buff, 10);
        SetDlgItemTextW(hDlg, EDIT_WORDSCNT, buff); // устанавливаем в поле "количество слов"
                                                                    // текущее значение параметра
        for (unsigned int i = 0; i < ru_dict.size; i++)
            if (ru_dict.selected[i] == 1)        // слова, которые были выбраны в прошлый раз,
                                                    // помечаем выбранными 
                SendMessage(hListBox, LB_SETSEL, TRUE, i);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        {
            
            HWND hListBox = GetDlgItem(hDlg, IDM_DICTLISTBOX);
            wchar_t buff[16];
            GetDlgItemTextW(hDlg, EDIT_WORDSCNT, buff, 16);
            unsigned int inputWordsCnt = _wtoi(buff);
            int selectedCount = SendMessageW(hListBox, LB_GETSELCOUNT, 0, 0);

            switch (LOWORD(wParam))
            {
            case DICTOK:

                if (inputWordsCnt > ru_dict.size)
                {
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_DICTPROBLEMS), hDlg, Default);
                    return (INT_PTR)FALSE;
                }
                else
                    maxWordsCount = inputWordsCnt;

                if (selectedCount != maxWordsCount)
                {
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_LACKOFWORDS), hDlg, Default);
                    return (INT_PTR)FALSE;
                }

                {
                    int* selected_ids = (int*)calloc(maxWordsCount, sizeof(int));

                    SendMessage(hListBox, LB_GETSELITEMS, maxWordsCount, (LPARAM)selected_ids);

                    memset(ru_dict.selected, 0, sizeof(char) * ru_dict.size);
                    for (unsigned int i = 0; i < maxWordsCount; i++)
                        ru_dict.selected[selected_ids[i]] = 1;

                    free(selected_ids);
                }

            case IDCANCEL:
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            case RANDCHOICE:
                

                if (inputWordsCnt > min(ru_dict.size, 477))
                {
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_DICTPROBLEMS), hDlg, Default);

                }
                else
                {
                    maxWordsCount = inputWordsCnt;
                    SendMessage(hListBox, LB_SETSEL, FALSE, -1);
                    for (unsigned int i = 0; i < maxWordsCount; i++)
                    {
                        int index;
                        do {
                            index = rand() % ru_dict.size;
                        } while (SendMessage(hListBox, LB_GETSEL, index, 0));
                        
                        SendMessage(hListBox, LB_SETSEL, TRUE, index);
                    }
                }
            }
        }

        break;
    }
    return (INT_PTR)FALSE;
}
