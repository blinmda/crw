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
	// ����� ���� �� ������ �������� �� ������� ����������
	int valid = 0;
	if (y < cw.width && x < cw.height) valid = 1;
	return valid;
}

int isEmptyCell(int x, int y)
{
	// ���������� '*' � ' ' ��� ����������� ������ �����
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

	if (isValidPosition(x - step_x, y - step_y))
		if (!isEmptyCell(x - step_x, y - step_y))
			return -1;

	for (; *word != '\0'; x += step_x, y += step_y, word++)
	{
		if (!isValidPosition(x, y))
			return -1;
		if (!(cw.map[x][y] == ' ' || *word == cw.map[x][y]))
			return -1;
		if (isValidPosition(x + step_y, y + step_x))
			if (!isEmptyCell(x + step_y, y + step_x) && *word != cw.map[x][y])
				return -1;

		if (isValidPosition(x - step_y, y - step_x))
			if (!isEmptyCell(x - step_y, y - step_x) && *word != cw.map[x][y])
				return -1;

		if (cw.map[x][y] == *word)
			intersectionsNumber++;
	}

	if (isValidPosition(x, y))
		if (!isEmptyCell(x, y))
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


// ������ �������� ������� ��, ������� ���� ������ ����������� � ������� ������� 
int selectBestPosition(const wchar_t* word, int* x, int* y, enum Orientation* dir)
{
	int max_beauty = -1;
	for (unsigned int i = 0; i < cw.height; i++)
		for (unsigned int j = 0; j < cw.width; j++)
		{
			int k = rand() % 2;
			int beauty = wordPlaceBeauty(word, i, j, (enum Orientation)k);
			if (beauty > max_beauty)
			{
				max_beauty = beauty;
				*x = i;
				*y = j;
				*dir = (enum Orientation)k;
			}

			k = (k + 1) % 2;//������ ����������
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

struct Position // ������� ����� � ����������
{
	int beauty; // ��������� ������ ��� �������
	int x;      // ���������� �� X
	int y;		// �� Y
	enum Orientation k; // ����������
};

int cmpPositions(struct Position* a, struct Position* b)
{
	return a->beauty - b->beauty; // �������, ������������ ��� ������� ��� ������ �����
}

int selectNotWorstPosition(const wchar_t* word, int* x, int* y, enum Orientation* dir)
{
	// �� ������ ������� -- �� ���� �������� �� ������ � �� ������
	struct Position* positions = (struct Position*)calloc(cw.height * cw.width * 2, sizeof(struct Position));
	if (positions == NULL) exit(MEMORY_ALLOCATION_ERROR);
	int idx = 0;
	for (unsigned int i = 0; i < cw.height; i++)
		for (unsigned int j = 0; j < cw.width; j++)
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
	l = l + (r - l)/2;

	*x = positions[l].x;
	*y = positions[l].y;
	*dir = (enum Orientation)positions[l].k;
	return positions[l].beauty;
}

int selectRandPosition(const wchar_t* word, int* x, int* y, enum Orientation* dir)
{
	//500 ��� �������� ������� ��������� ������� ��� ����� 
	int cycles = 500;
	while (cycles--)
	{
		int i = rand() % cw.height, j = rand() % cw.width;
		int k = rand() % 2;
		int beauty = wordPlaceBeauty(word, i, j, (enum Orientation)k);
		if (beauty >= 0)
		{
			*x = i;
			*y = j;
			*dir = (enum Orientation)k;
			return beauty;
		}

		k = (k + 1) % 2;
		beauty = wordPlaceBeauty(word, i, j, (enum Orientation)k);
		if (beauty >= 0)
		{

			*x = i;
			*y = j;
			*dir = (enum Orientation)k;
			return beauty;
		}
	}
	return -1;
}

void fillCrossword(struct Dictionary dict, int density, HWND parent, HINSTANCE hInst)
{
	initProgressBar(dict.size, parent, hInst); // �������������� ������ �������� ��� ������ ��������

	for (unsigned int i = 0; i < dict.size; i++) // ������������ ����� � ����������
										// ����� ��� ����, ����� ��� ������ ����� ��������� � ����� ������� ����
										// ��������� ������ ���������
	{
		int first = rand() % dict.size, second = rand() % dict.size;
		wchar_t* tmp = dict.words[first]; // ������ ������ ������� ��������� �����
		dict.words[first] = dict.words[second];
		dict.words[second] = tmp;
	}

	int* skippedWords = (int*)calloc(dict.size, sizeof(int)); // �� ������ ���������� ���������� ����� �����
																		 // � ���� ������� �������� ������ ��� ����, ������� ���������� �� �������
	int skippedCnt = 0;

	int (*selector)(const wchar_t*, int*, int*, enum Orientation*) = NULL; // �������, ���������� ����� ��� ������-�� �����
	if (density == DENSITY_LOW) // ���� ��� ����� ��������� � ������ ����������
		selector = selectRandPosition; // ����� ����� ������� � ��������� �����
	else if (density == DENSITY_MEDIUM) // ��� ������� ���������
		selector = selectNotWorstPosition; // �������� �� ������ �����
	else if (density == DENSITY_HIGH) // ��� �������
		selector = selectBestPosition; // �������� ������ �����
	else
		exit(INVALID_PARAMETER_ERROR); // ���-�� ����� �� ���

	for (unsigned int i = 0; i < dict.size; i++) // ��� ���� ���� � ������� 
	{
		wchar_t* word = dict.words[i];
		int x, y;
		enum Orientation dir;

		int beauty = selector(word, &x, &y, &dir); // �������� �������

		if (beauty == 0 && i != 0) // ���� ��������� ����� -- 0, �� �� �� ������ ���������� �����
		{
			skippedWords[skippedCnt++] = i;
			continue;
		}

		if (beauty < 0) continue;

		progressBarNextStep(); // ����� ������������, ������� ������ ��������
		putWord(word, x, y, dir); // ������������� ����� � ���������
	}

	for (int i = 0; i < skippedCnt; i++) // � ������ ���������� ��� ����������� ����� � ������������� �� 
	{
		wchar_t* word = dict.words[skippedWords[i]];
		int x, y;
		enum Orientation dir;

		selector = rand() % 100 > 95 ? selectNotWorstPosition : selector;
		int beauty = selector(word, &x, &y, &dir);

		if (beauty < 0) continue;

		progressBarNextStep();
		putWord(word, x, y, dir);
	}

	for (unsigned int i = 0; i < cw.height; i++)
		for (unsigned int j = 0; j < cw.width; j++)
			if (cw.map[i][j] == L'*') // ������� ��� ���������� �� ���������� 
				cw.map[i][j] = L' ';
	destroyProgressBar(); // ������� ������ �������� 
}

struct Crossword getCrossword()
{
	return cw;
}

//������ � ������� ��������
static HWND hPB;


void initProgressBar(int wordsCount, HWND parent, HINSTANCE inst)
{
	InitCommonControls();
	RECT rcClient;  // Client area of parent window.
	int cyVScroll; // Height of scroll bar arrow.

	GetClientRect(parent, &rcClient); //��������� ���������� ������� ������� ����

	cyVScroll = GetSystemMetrics(SM_CYVSCROLL); // ���������� ������ ������������ ������ ���������

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

//������ � �������

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
	ofn.hwndOwner = hWnd;//���������� ����, ������� ������� ������ �������
	ofn.lpstrFilter = formats; //��������� �� �����, ���������� ���� ����� �������� � ������� �������� � �����
	ofn.nMaxCustFilter = 256; //���������� ������
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
	//����������, ��� ����� ���������, ��������� � ���������� ���� �������(Open) ��� ��������� ���(Save As) ���������� ����� ������ ��������� � ����� ����������
	//����������, ��� ������������ ����� ������� � ���������� ����� ������ ������������ ������ � ���� ����� ��� ����� (File Name
	ofn.lpstrFile = file; //��������� �� �����, ������� �������� ��� �����, ������������, ����� ���������������� ���� ��������������
	ofn.nMaxFile = 512;//ofn.nMaxFile
	ofn.nFilterIndex = 1; //���������� ������ �������� ���������� ������� � ������ ����������
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