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

//���������, ����������� ��� ���������
struct Crossword
{
	unsigned height; // ������ ���������� (����� �����)
	unsigned width;  // ������ ���������� (����� ��������)
	wchar_t** map;   // ��� ��������� -- ������� (height x width)
};

// ���������, ����������� ������������ �������.

struct Dictionary
{
	unsigned size;  // ���������� ���� � �������
	wchar_t** words;// ������ ���� (������ �����)
	char* selected; // ���� ���������: ���� i-�� ���� ����� true,
					// �� i-�� ����� ������� ������������� ��� ���������
};

enum Orientation
{
	vertical,		// ������ ����� ����� ��������� �����������
	horizontal      // ��� �������������
};

// ���������, ���������� �� ������� ��������� ����������

typedef struct
{
	int density; // ���������: low, med, high
	int width;   // ������
	int height;  // ������
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
