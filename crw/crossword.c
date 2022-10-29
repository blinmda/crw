#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <wchar.h>
#include "Resource.h"
#include "crossword.h"
#include <CommCtrl.h>
#pragma comment (lib, "comctl32")

static struct Crossword cw = { 0, 0, 0 };

struct Dictionary readDictionary(const wchar_t* file_name)
{
	struct Dictionary ru_dict = { 0,0,0 };
	FILE* file = NULL;
	_wfopen_s(&file, file_name, L"r");

	if (!file)
		exit(NO_SUCH_FILE_OR_DIRECTORY);

	fwscanf_s(file, L"%u", &ru_dict.size);
	ru_dict.words = (wchar_t**)calloc(ru_dict.size, sizeof(wchar_t*));
	ru_dict.selected = (char*)calloc(ru_dict.size, sizeof(char));

	if (!ru_dict.words || !ru_dict.selected)
		exit(MEMORY_ALLOCATION_ERROR);

	for (unsigned int i = 0; i < ru_dict.size; i++)
	{
		ru_dict.words[i] = (wchar_t*)calloc(MAX_WORD_LENGTH + 1, sizeof(wchar_t));
		if (!ru_dict.words[i])
			exit(MEMORY_ALLOCATION_ERROR);
		fwscanf(file, L"%31ls", ru_dict.words[i]);
	}

	fclose(file);
	return ru_dict;
}

void freeCrossword()
{
	if (!cw.map) return;
	for (unsigned int i = 0; i < cw.height; i++)
	{
		free(cw.map[i]);
	}
	free(cw.map);
}

void initCrossword(parameters ps)
{
	freeCrossword();
	cw.height = ps.height;
	cw.width = ps.width;

	cw.map = (wchar_t**)calloc(ps.height, sizeof(wchar_t*));

	if (!cw.map)
		exit(MEMORY_ALLOCATION_ERROR);

	for (int i = 0; i < ps.height; i++)
	{
		cw.map[i] = (wchar_t*)calloc(ps.width + 1, sizeof(wchar_t));

		if (!cw.map[i])
			exit(MEMORY_ALLOCATION_ERROR);

		wmemset(cw.map[i], L' ', ps.width);
	}
}

int isValidPosition(unsigned int x, unsigned int y)
{
	// буквы слов не должны вылезать за границы кроссворда
	int valid = 0;
	if (y < cw.width && x < cw.height) valid = 1;
	return valid;
}

int isEmptyCell(int x, int y)
{
	// используем '*' и ' ' для обозначения пустых ячеек
	int empty = 0;
	if (cw.map[x][y] == ' ' || cw.map[x][y] == '*') empty = 1;
	return empty;
}

/*
* beauty == number of intersections with other words
* beauty < 0 if word can't be placed
* beauty == 0 if word has no intersections
* max(beauty) == the best place to put word
*/
int wordPlaceBeauty(const wchar_t* word, int x, int y, enum Orientation orientation)
{
	int intersectionsNumber = 0;

	int step_x = 0, step_y = 0;
	if (orientation == horizontal) step_x = 1;
	else step_y = 1;

	if ((isValidPosition(x - step_x, y - step_y)) && (!isEmptyCell(x - step_x, y - step_y)))
			return -1;

	for (; *word != '\0'; x += step_x, y += step_y, word++)
	{
		if (!isValidPosition(x, y))
			return -1;
		if (!(cw.map[x][y] == ' ' || *word == cw.map[x][y]))
			return -1;
		if ((isValidPosition(x + step_y, y + step_x)) && (!isEmptyCell(x + step_y, y + step_x) && *word != cw.map[x][y]))
				return -1;

		if ((isValidPosition(x - step_y, y - step_x)) && (!isEmptyCell(x - step_y, y - step_x) && *word != cw.map[x][y]))
				return -1;

		if (cw.map[x][y] == *word)
			intersectionsNumber++;
	}

	if ((isValidPosition(x, y)) && (!isEmptyCell(x, y)))
			return -1;
	return intersectionsNumber;
}

void putWord(const wchar_t* word, int x, int y, enum Orientation dir)
{
	int step_x = 0, step_y = 0;
	if (dir == horizontal) step_x = 1;
	else step_y = 1;

	if (isValidPosition(x - step_x, y - step_y))
		cw.map[x - step_x][y - step_y] = '*';
	for (; *word != '\0'; y += step_y, x += step_x, word++)
	{
		cw.map[x][y] = *word;
	}

	if (isValidPosition(x, y))
		cw.map[x][y] = '*';
}


// лучшей позицией считаем ту, которая дает больше пересечений с другими словами 
int selectBestPosition(const wchar_t* word, int* x, int* y, enum Orientation* dir, int nb)
{
	int max_beauty = -1;
	// алгоритмическая оптимизация
	int hht = cw.height, wth = cw.width;
	if (nb == 1) {
		hht >>= 1;
		wth >>= 1;
	}
	for (unsigned int i = 0; i < hht; i++)
		for (unsigned int j = 0; j < wth; j++)
		{
			int k = rand() & 1; // снижение мощности
			int beauty = wordPlaceBeauty(word, i, j, (enum Orientation)k);
			if (beauty > max_beauty)
			{
				max_beauty = beauty;
				*x = i;
				*y = j;
				*dir = (enum Orientation)k;
			}

			k = (k + 1) & 1; // снижение мощности
			beauty = wordPlaceBeauty(word, i, j, (enum Orientation)k);
			if (beauty > max_beauty)
			{
				max_beauty = beauty;
				*x = i;
				*y = j;
				*dir = (enum Orientation)k;
			}
		}
	return max_beauty;
}

struct Position // позиция слова в кроссворде
{
	int beauty; // насколько хороша эта позиция
	int x;      // координата по X
	int y;		// по Y
	enum Orientation k; // ориентация
};

int cmpPositions(struct Position* a, struct Position* b)
{
	return a->beauty - b->beauty; // функция, сравнивающая две позиции для одного слова
}

int selectNotWorstPosition(const wchar_t* word, int* x, int* y, enum Orientation* dir, int nb)
{
	// Не худшая позиция -- из всех выбираем не лучшую и не худшую
	struct Position* positions = (struct Position*)calloc(cw.height * cw.width * 2, sizeof(struct Position));
	if (positions == NULL) exit(MEMORY_ALLOCATION_ERROR);
	int idx = 0;
	// алгоритмическая оптимизация
	int hht = cw.height, wth = cw.width;
	if (nb == 1) {
		hht >>= 1;
		wth >>= 1;
	}
	for (unsigned int i = 0; i < hht; i++)
		for (unsigned int j = 0; j < wth; j++)
			for (int k = 0; k < 2; k++)
			{
				positions[idx].beauty = wordPlaceBeauty(word, i, j, (enum Orientation)k);
				positions[idx].x = i;
				positions[idx].y = j;
				positions[idx++].k = (enum Orientation)k;
			}
	qsort(positions, idx, sizeof(struct Position), (_CoreCrtNonSecureSearchSortCompareFunction)cmpPositions);

	int l = 0;
	int r = idx;

	while (l < idx - 1 && positions[l].beauty != 1) l++;
	l += (r - l) >> 1; // снижение мощности

	*x = positions[l].x;
	*y = positions[l].y;
	*dir = (enum Orientation)positions[l].k;
	return positions[l].beauty;
}

int selectRandPosition(const wchar_t* word, int* x, int* y, enum Orientation* dir, int ndBeauty)
{
	//Алгоритмическая оптимизация
	int beauty = -1, counter = 0, size = cw.height * cw.width;
	int i = 0, j = 0, k = 0;
	while (beauty < ndBeauty) {
		i = rand() % cw.height, j = rand() % cw.width;
		k = rand() & 1;
		beauty = wordPlaceBeauty(word, i, j, (enum Orientation)k);
		counter++;
		if (counter == size) break;
	}
	*x = i;
	*y = j;
	*dir = (enum Orientation)k;
	return beauty;
}

void fillCrossword(struct Dictionary dict, int density, HWND parent, HINSTANCE hInst)
{
	initProgressBar(dict.size, parent, hInst); // инициализируем полосу загрузки для долгих операций

	for (unsigned int i = 0; i < dict.size; i++) // перемешиваем слова в кроссворде
										// нужно для того, чтобы при каждой новой генерации с одним набором слов
										// получался разный результат
	{
		int first = rand() % dict.size, second = rand() % dict.size;
		wchar_t* tmp = dict.words[first]; // просто меняем местами случайные слова
		dict.words[first] = dict.words[second];
		dict.words[second] = tmp;
	}

	int* skippedWords = (int*)calloc(dict.size, sizeof(int)); // не всегда получается установить слово сразу
																		 // в этом массиве хранятся номера тех слов, которые установить не удалось
	int skippedCnt = 0;

	int (*selector)(const wchar_t*, int*, int*, enum Orientation*, int) = NULL; // функция, выбирающая место для какого-то слова
	if (density == DENSITY_LOW) // если нам нужен кроссворд с низкой плотностью
		selector = selectRandPosition; // слова будем ставить в рандомные места
	else if (density == DENSITY_MEDIUM) // для средней плотности
		selector = selectNotWorstPosition; // выбираем не худшие места
	else if (density == DENSITY_HIGH) // для высокой
		selector = selectBestPosition; // выбираем лучшие места
	else
		exit(INVALID_PARAMETER_ERROR); // что-то пошло не так
	int ndBeauty = 0;//переменная, хранящая минимальное количество пересечений

	for (unsigned int i = 0; i < dict.size; i++) // для всех слов в словаре 
	{
		wchar_t* word = dict.words[i];
		int x, y;
		enum Orientation dir;

		if (((density == DENSITY_LOW) && (dict.size > 20 && i >= (dict.size >> 1))) || ((density == DENSITY_HIGH || density == DENSITY_MEDIUM) && (i< (dict.size >> 2)))) ndBeauty = 1;
		int beauty = selector(word, &x, &y, &dir, ndBeauty); // выбираем позицию

		if (density != DENSITY_LOW || ndBeauty !=0) {
			if (beauty == 0 && i != 0) // если хорошесть слова -- 0, то мы не смогли установить слово
			{
				skippedWords[skippedCnt++] = i;
				continue;
			}
		}
		if (beauty < 0) continue;

		progressBarNextStep(); // слово установилось, двигаем полосу загрузки
		putWord(word, x, y, dir); // устанавливаем слово в кроссворд
	}
	ndBeauty = 0;

	for (int i = 0; i < skippedCnt; i++) // а теперь перебираем все пропущенные слова и устанавливаем их 
	{
		ndBeauty = 0;
		wchar_t* word = dict.words[skippedWords[i]];
		int x, y;
		enum Orientation dir;

		int beauty = selector(word, &x, &y, &dir, ndBeauty);

		if (beauty < 0) continue;

		progressBarNextStep();
		putWord(word, x, y, dir);
	}

	for (unsigned int i = 0; i < cw.height; i++)
		for (unsigned int j = 0; j < cw.width; j++)
			if (cw.map[i][j] == L'*') // убираем все зввездочки из кроссворда 
				cw.map[i][j] = L' ';
	destroyProgressBar(); // убираем полосу загрузки 
}

struct Crossword getCrossword()
{
	return cw;
}

//работа с полосой загрузки
static HWND hPB;


void initProgressBar(int wordsCount, HWND parent, HINSTANCE inst)
{
	InitCommonControls();
	RECT rcClient;  // Client area of parent window.
	int cyVScroll; // Height of scroll bar arrow.

	GetClientRect(parent, &rcClient); //извлекает координаты рабочей области окна

	cyVScroll = GetSystemMetrics(SM_CYVSCROLL); // Возвращает ширину вертикальной полосы прокрутки

	// Create the list-view window in report view with label editing enabled.
	hPB = CreateWindowEx(0, PROGRESS_CLASS, L"Generation progress", WS_CHILD | WS_VISIBLE,
		rcClient.left,
		rcClient.bottom - cyVScroll,
		rcClient.right, cyVScroll, parent, (HMENU)0, inst, NULL);


	SendMessage(hPB, PBM_SETRANGE, 0, MAKELPARAM(0, wordsCount));

	SendMessage(hPB, PBM_SETSTEP, (WPARAM)1, 0);
}

void progressBarNextStep()
{
	SendMessage(hPB, PBM_STEPIT, 0, 0);
}

void destroyProgressBar()
{
	DestroyWindow(hPB);
}

//работа с файлами

PWSTR loadFile(HWND hWnd) {
	OPENFILENAME ofn;

	wchar_t formats[] = L"Crossword(*.cw)\0*.cw\0\0";

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = formats;
	ofn.lpstrCustomFilter = formats;
	ofn.nMaxCustFilter = 256;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
	ofn.lpstrFile = (LPWSTR)calloc(512, sizeof(wchar_t));
	ofn.nMaxFile = 512;
	ofn.nFilterIndex = 1;
	if (!GetOpenFileName(&ofn)) printf("\nERROR %d\n", CommDlgExtendedError());
	return ofn.lpstrFile;
}

PWSTR uploadFile(HWND hWnd) {
	OPENFILENAME ofn;

	wchar_t formats[] = L"Crossword(*.cw)\0*.cw\0\0";
	wchar_t* file = (wchar_t*)calloc(512, sizeof(wchar_t));
	if (!file) return NULL;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;//Дескриптор окна, которое владеет блоком диалога
	ofn.lpstrFilter = formats; //Указатель на буфер, содержащий пары строк фильтров с нулевым символом в конце
	ofn.nMaxCustFilter = 256; //Определяет размер
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
	//Обозначает, что любые настройки, сделанные в диалоговом окне Открыть(Open) или Сохранить как(Save As) используют новые методы настройки в стиле Проводника
	//Определяет, что пользователь может вводить с клавиатуры имена только существующих файлов в поле ввода Имя файла (File Name
	ofn.lpstrFile = file; //Указатель на буфер, который содержит имя файла, используемое, чтобы инициализировать поле редактирования
	ofn.nMaxFile = 512;//ofn.nMaxFile
	ofn.nFilterIndex = 1; //Определяет индекс текущего выбранного фильтра в органе управления
	if (!GetSaveFileName(&ofn)) printf("\nERROR %d\n", CommDlgExtendedError());

	if (!wcsstr(file, L".cw")) {
		wcscat(file, L".cw");
	}
	for (int i = 0; i < 512; i++)
		printf("%wc", file[i]);
	return ofn.lpstrFile;
}

void saveCrossword(HWND hwnd)
{
	wchar_t* fileName = uploadFile(hwnd);

	FILE* file = _wfopen(fileName, L"w");
	free(fileName);
	if (file == NULL)
		exit(NO_SUCH_FILE_OR_DIRECTORY);

	fwprintf(file, L"%u %u\n", cw.height, cw.width);

	for (unsigned int i = 0; i < cw.height; i++)
	{
		for (unsigned int j = 0; j < cw.width; j++)
			fwprintf(file, L"%wc", cw.map[i][j]);
	}
	fclose(file);
}

parameters loadCrossword(HWND hwnd)
{
	int height, width;
	wchar_t* fileName = loadFile(hwnd);
	FILE* file = _wfopen(fileName, L"r");
	free(fileName);
	if (file == NULL) {
		parameters params = { 0, 15, 15 };
		initCrossword(params);
		return params;
	}

	fwscanf(file, L"%d %d", &height, &width);

	parameters params = { 0, width, height };
	initCrossword(params);
	wchar_t enter;
	fwscanf(file, L"%wc", &enter);
	for (unsigned int i = 0; i < cw.height; i++)
	{
		for (unsigned int j = 0; j < cw.width; j++)
			fwscanf(file, L"%wc", &(cw.map[i][j]));
	}
	fclose(file);

	return params;
}