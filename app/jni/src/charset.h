#pragma once

#include "SDL_ttf.h"

//CharSet.c

typedef struct UseFont_Type {      // ���嵱ǰʹ�õ�����ṹ
    int size;                      //�ֺţ���λ����
    char *name;                    //�����ļ���
    TTF_Font *font;                //�򿪵�����
}UseFont;

#define FONTNUM 10                 //����ͬʱ�򿪵��������

//��ʼ������
int InitFont();

//�ͷ�����ṹ
int ExitFont();

// ���������ļ������ֺŴ�����
// size Ϊ�����ش�С���ֺ�
TTF_Font *GetFont(const char *filename, int size);

// д�ַ���
// x,y ����
// str �ַ���
// color ��ɫ
// size �����С������Ϊ���塣 
// fontname ������
// charset �ַ��� 0 GBK 1 big5
// OScharset ����
int JY_DrawStr(int x, int y, const char *str, int color, int size, const char *fontname,
    int charset, int OScharset);

//�������ת���ļ�
int LoadMB(const char* mbfile);


// �����ַ���ת��
// flag = 0   Big5 --> GBK     
//      = 1   GBK  --> Big5    
//      = 2   Big5 --> Unicode
//      = 3   GBK  --> Unicode
// ע��Ҫ��֤dest���㹻�Ŀռ䣬һ�㽨��ȡsrc���ȵ�����+1����֤ȫӢ���ַ�Ҳ��ת��Ϊunicode
int  JY_CharSet(const char *src, char *dest, int flag);
