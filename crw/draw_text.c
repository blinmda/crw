#include "draw.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static GLuint currentTexture;

void loadFontTexture(const char* fileName)
{
	int x, y, n;
	unsigned char* data = stbi_load(fileName, &x, &y, &n, 0);
	glGenTextures(1, &currentTexture); //������� ��� �������� (�������������� ��), 
	//��� ��������� � �������� ������� ������ ���������� �������, ������� ����� �������������, � ��������� �� � �������, �������� � �������� ������� ���������
	glBindTexture(GL_TEXTURE_2D, currentTexture);//����������� ��� �������� � ���������������� �������� ������� ��������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//������ ��������� ��������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//� ���������� ������� �������� ����������� ����� ����������� �����������
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);
}


void drawChar(char c, float fontSize, float x, float y)
{
	float rectCoord[] = { x,y,
						  x + fontSize,y,
						  x + fontSize, y + fontSize,
						  x, y + fontSize };
	float rectTex[] = { 0,1, 1,1, 1,0, 0,0 };

	static float charSize = 1 / 16.0f;
	int x_offset = c & 0b1111, y_offset = c >> 4;

	//���������� ��������� ����� �� ������
	//�������� ��������
	rectTex[0] = rectTex[6] = x_offset * charSize;
	rectTex[2] = rectTex[4] = rectTex[0] + charSize;
	rectTex[5] = rectTex[7] = y_offset * charSize;
	rectTex[1] = rectTex[3] = rectTex[5] + charSize;

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, currentTexture);


	glPushMatrix();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, rectCoord);
	glTexCoordPointer(2, GL_FLOAT, 0, rectTex);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}