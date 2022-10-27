#pragma once

#define MAX_WORD_LENGTH 31

#include <Windows.h>

enum error
{
	OK = 0,
	NO_SUCH_FILE_OR_DIRECTORY,
	MEMORY_ALLOCATION_ERROR,
	INVALID_PARAMETER_ERROR
};

//Структура, описывающая сам кроссворд
struct Crossword
{
	unsigned height; // высота кроссворда (число строк)
	unsigned width;  // ширина кроссворда (число столбцов)
	wchar_t** map;   // сам кроссворд -- матрица (height x width)
};

// Структура, описывающая используемый словарь.

struct Dictionary
{
	unsigned size;  // количество слов в словаре
	wchar_t** words;// список слов (массив строк)
	char* selected; // Флаг состояния: если i-ый флаг равен true,
					// то i-ое слово выбрано пользователем для генерации
};

enum Orientation
{
	vertical,		// Каждое слово можно поставить вертикально
	horizontal      // или горизонтально
};

// Структура, отвечающая за текущие параметры кроссворда

typedef struct
{
	int density; // плотность: low, med, high
	int width;   // ширина
	int height;  // высота
} parameters;

struct Dictionary readDictionary(const wchar_t* file_name);
void initCrossword(parameters params);
void fillCrossword(struct Dictionary dict, int density, HWND parent, HINSTANCE hInst);
struct Crossword getCrossword();
void saveCrossword(HWND hwnd);
parameters loadCrossword(HWND hwnd);
void initProgressBar(int wordsCount, HWND parent, HINSTANCE inst);
void destroyProgressBar();
void progressBarNextStep();
