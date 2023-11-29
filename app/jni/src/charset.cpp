
// ������ֺ��ַ���ת��


//Ϊ��֤ƽ̨�����ԣ��Լ�������һ��gbk����/����/big5/unicode������ļ�
//ͨ�����ļ������ɽ��и��ָ�ʽ��ת��

#include <stdlib.h>
#include "charset.h"
#include "sdlfun.h"
#include <stdio.h>
#include "jymain.h"
#include <map>

// ��ʾTTF �ַ���
// Ϊ������ʾ�����򽫱����Ѿ��򿪵���Ӧ�ֺŵ�����ṹ�����������Լӿ�����ٶ�
// Ϊ�򻯴��룬û�����������ǲ�������������򿪵����塣
// ���Ƚ��ȳ��ķ�����ѭ���ر��Ѿ��򿪵����塣
// ���ǵ�һ��򿪵����岻�࣬����640*480ģʽʵ����ֻ����16*24*32�������塣
// ��������Ϊ10�Ѿ��㹻��

UseFont Font[FONTNUM];         //�����Ѵ򿪵�����

int currentFont = 0;

//�ַ���ת������
Uint16 gbk_unicode[128][256];
Uint16 gbk_big5[128][256];
Uint16 big5_gbk[128][256];
Uint16 big5_unicode[128][256];

#define MAX_CACHE_CHAR (10000)
int char_count = 0;
std::map<int, SDL_Texture*> chars_cache;
SDL_Texture** chars_record[MAX_CACHE_CHAR] = { 0 };

//��ʼ��
int InitFont()
{
    int i;

    TTF_Init();  // ��ʼ��sdl_ttf

    for (i = 0; i < FONTNUM; i++)     //�������ݳ�ֵ
    {
        Font[i].size = 0;
        Font[i].name = NULL;
        Font[i].font = NULL;
    }

    return 0;
}

//�ͷ�����ṹ
int ExitFont()
{
    JY_Debug("%d chars cached.", char_count);
    /*for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 60000; j++)
        {
            if (chars_cache[i][j])
            { SDL_DestroyTexture(chars_cache[i][j]); }
        }
    }*/
    for (auto& c : chars_cache)
    {
        SDL_DestroyTexture(c.second);
    }

    for (int i = 0; i < FONTNUM; i++)    //�ͷ���������
    {
        if (Font[i].font)
        {
            TTF_CloseFont(Font[i].font);
        }
        SafeFree(Font[i].name);
    }

    TTF_Quit();

    return 0;
}


// ���������ļ������ֺŴ�����
// size Ϊ�����ش�С���ֺ�
TTF_Font* GetFont(const char* filename, int size)
{
    int i;
    TTF_Font* myfont = NULL;

    for (i = 0; i < FONTNUM; i++)     //  �ж������Ƿ��Ѵ�
    {
        if ((Font[i].size == size) && (Font[i].name) && (strcmp(filename, Font[i].name) == 0))
        {
            myfont = Font[i].font;
            break;
        }
    }

    if (myfont == NULL)      //û�д�
    {
        myfont = TTF_OpenFont(filename, size);           //��������
        if (myfont == NULL)
        {
            JY_Debug("GetFont error: can not open font file %s\n", filename);
            return NULL;
        }
        Font[currentFont].size = size;
        if (Font[currentFont].font)           //ֱ�ӹرյ�ǰ���塣
        { TTF_CloseFont(Font[currentFont].font); }

        Font[currentFont].font = myfont;

        SafeFree(Font[currentFont].name);
        Font[currentFont].name = (char*)malloc(strlen(filename) + 1);
        strcpy(Font[currentFont].name, filename);

        currentFont++;           // ���Ӷ�����ڼ���
        if (currentFont == FONTNUM)
        { currentFont = 0; }

    }

    return myfont;

}

// д�ַ���
// x,y ����
// str �ַ���
// color ��ɫ
// size �����С������Ϊ���塣
// fontname ������
// charset �ַ��� 0 GBK 1 big5
// OScharset 0 ������ʾ 1 ������ʾ
int JY_DrawStr(int x, int y, const char* str, int color, int size, const char* fontname,
    int charset, int OScharset)
{
    SDL_Color c, c2, white;
    SDL_Surface* fontSurface = NULL, *fontSurface1 = NULL;
    int w, h;
    SDL_Rect rect1, rect2, rect_dest;
    SDL_Rect rect;
    char tmp1[256], tmp2[256];
    TTF_Font* myfont;
    SDL_Surface* tempSurface;
    //float xzoom = (float)g_RealScreenW/(float)g_ScreenW;
    //float yzoom = (float)g_RealScreenH/(float)g_ScreenH;

    //û�����ݲ���ʾ
    if (strlen(str) == 0)
    { return 0; }


    if (strlen(str) > 127)
    {
        JY_Error("JY_DrawStr: string length more than 127: %s", str);
        return 0;
    }

    myfont = GetFont(fontname, size);
    if (myfont == NULL)
    { return 1; }

    c.r = (Uint8)((color & RMASK) >> 16);
    c.g = (Uint8)((color & GMASK) >> 8);
    c.b = (Uint8)((color & BMASK));
    c.a = AMASK;
    c2.r = c.r >> 1;
    c2.b = c.b >> 1;
    c2.g = c.g >> 1;
    c2.a = AMASK;
    white.r = 255;
    white.g = 255;
    white.b = 255;
    white.a = 255;

    if (charset == 0 && OScharset == 0)  //GBK -->unicode����
    {
        JY_CharSet(str, tmp2, 3);
    }
    else if (charset == 0 && OScharset == 1)  //GBK -->unicode����
    {
        JY_CharSet(str, tmp1, 1);
        JY_CharSet(tmp1, tmp2, 2);
    }
    else if (charset == 1 && OScharset == 0)   //big5-->unicode����
    {
        JY_CharSet(str, tmp1, 0);
        JY_CharSet(tmp1, tmp2, 3);
    }
    else if (charset == 1 && OScharset == 1)    ////big5-->unicode����
    {
        JY_CharSet(str, tmp2, 2);
    }
    else
    {
        strcpy(tmp2, str);
    }

    SDL_RenderGetClipRect(g_Renderer, &rect);
    if (rect.w == 0) { rect.w = g_ScreenW - rect.x; }
    if (rect.h == 0) { rect.h = g_ScreenH - rect.y; }

    TTF_SizeUNICODE(myfont, (Uint16*)tmp2, &w, &h);

    if ((x >= rect.x + rect.w) || (x + w + 1) <= rect.x ||
        (y >= rect.y + rect.h) || (y + h + 1) <= rect.y)        // �����ü���Χ����ʾ
    {
        return 1;
    }

    //fontSurface=TTF_RenderUNICODE_Solid(myfont, (Uint16*)tmp2, c);  //���ɱ���
    //SDL_SetRenderTarget(g_Renderer, g_TextureTmp);
    Uint16* p = (Uint16*)tmp2;

    //rect1.x = x*xzoom;
    //rect1.y = y*yzoom;
    rect1.x = x;
    rect1.y = y;
    int pi = 0;
    //printf("%d,%d\n", strlen(str), strlen(tmp2));
    for (int i = 0; i < 128; i++)
    {
        int s = size;
        if (*p == 0)
        {
            break;
        }
        if (*p <= 128) { s = size / 2; }
        SDL_Texture* tex = NULL;
        if (chars_cache.count(*p + size * 65536))
        { tex = chars_cache[*p + size * 65536]; }
        if (tex == NULL)
        {
            Uint16 tmp[2] = { 0, 0 };
            tmp[0] = *p;
            SDL_Surface* sur = TTF_RenderUNICODE_Blended(myfont, tmp, white);
            tex = SDL_CreateTextureFromSurface(g_Renderer, sur);
            chars_cache[*p + size * 65536] = tex;
            SDL_FreeSurface(sur);
            char_count++;
#ifdef _DEBUG
            unsigned char out[3] = { 0, 0, 0 };
            out[0] = *(str + pi);
            if (out[0] > 128)
            {
                out[1] = *(str + pi + 1);
            }
            JY_Debug("cache [%d][%s(%d)], total %d", size, out, *p, char_count);
#endif
        }
#ifdef _DEBUG
        if (*p <= 128) { pi++; }
        else { pi += 2; }
#endif
        if (*p != 32)
        {
            SDL_SetRenderTarget(g_Renderer, g_Texture);
            //SDL_SetRenderTarget(g_Renderer, g_TextureTmp);
            SDL_QueryTexture(tex, NULL, NULL, &rect1.w, &rect1.h);
            rect2 = rect1;
            rect2.h = rect2.h;
            //���ֻ��Ƶ���ʱ��texture��

            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
            SDL_SetTextureColorMod(tex, c2.r, c2.g, c2.b);
            SDL_SetTextureAlphaMod(tex, 128);
            rect2.x = rect1.x + 1;
            rect2.y = rect1.y + 1;
            SDL_RenderCopy(g_Renderer, tex, NULL, &rect2);
            //SDL_BlitSurface(*tex, NULL, g_Surface, &rect2);
            rect2.x = rect1.x + 1;
            rect2.y = rect1.y;
            SDL_RenderCopy(g_Renderer, tex, NULL, &rect2);
            //SDL_BlitSurface(*tex, NULL, g_Surface, &rect2);
            rect2.x = rect1.x;
            rect2.y = rect1.y + 1;
            SDL_RenderCopy(g_Renderer, tex, NULL, &rect2);
            //SDL_BlitSurface(*tex, NULL, g_Surface, &rect2);
            SDL_SetTextureColorMod(tex, c.r, c.g, c.b);
            SDL_SetTextureAlphaMod(tex, 255);

            SDL_RenderCopy(g_Renderer, tex, NULL, &rect1);
            //SDL_BlitSurface(*tex, NULL, g_Surface, &rect1);
            s = rect1.w;

            //SDL_SetRenderTarget(g_Renderer, g_Texture);
        }
        rect1.x = rect1.x + s;
        p++;
    }

    //JY_Debug("%d chars cached.", char_count);
    //fontSurface = TTF_RenderUNICODE_Blended(myfont, (Uint16*)tmp2, c);
    //fontSurface1 = TTF_RenderUNICODE_Blended(myfont, (Uint16*)tmp2, c2);
    //SDL_SetSurfaceAlphaMod(fontSurface, 255);
    //SDL_SetSurfaceAlphaMod(fontSurface1, 128);

    if (fontSurface == NULL)
    { return 1; }

    rect1.x = (Sint16)x;
    rect1.y = (Sint16)y;
    rect1.w = (Uint16)fontSurface->w;
    rect1.h = (Uint16)fontSurface->h;

    /*
    if (g_Rotate == 0)
    {
        rect2 = rect1;

        rect_dest.x = rect2.x + 1;
        rect_dest.y = rect2.y;
        //SDL_SetPaletteColors(fontSurface1->format->palette, &c2, 1, 1);
        SDL_BlitSurface(fontSurface1, NULL, g_Surface, &rect_dest);    //����д����Ϸ����--��Ӱɫ

        rect_dest.x = rect2.x;
        rect_dest.y = rect2.y + 1;
        //SDL_SetPaletteColors(fontSurface1->format->palette, &c2, 1, 1);
        SDL_BlitSurface(fontSurface1, NULL, g_Surface, &rect_dest);    //����д����Ϸ����--��Ӱɫ

        rect_dest.x = rect2.x + 1;
        rect_dest.y = rect2.y + 1;
        //SDL_SetPaletteColors(fontSurface1->format->palette, &c2, 1, 1);
        SDL_BlitSurface(fontSurface1, NULL, g_Surface, &rect_dest);    //����д����Ϸ����--��Ӱɫ

        rect_dest.x = rect2.x;
        rect_dest.y = rect2.y;
        //SDL_SetPaletteColors(fontSurface->format->palette, &c, 1, 1);
        SDL_BlitSurface(fontSurface, NULL, g_Surface, &rect_dest);    //����д����Ϸ����
        SDL_FreeSurface(fontSurface1);
    }
    else if (g_Rotate == 1)
    {
        tempSurface = RotateSurface(fontSurface);
        SDL_FreeSurface(fontSurface);
        fontSurface = tempSurface;
        rect2 = RotateRect(&rect1);

        rect_dest.x = rect2.x + 1;
        rect_dest.y = rect2.y;
        SDL_SetPaletteColors(fontSurface1->format->palette, &c2, 1, 1);
        SDL_BlitSurface(fontSurface1, NULL, g_Surface, &rect_dest);    //����д����Ϸ����--��Ӱɫ

        rect_dest.x = rect2.x - 1;
        rect_dest.y = rect2.y;
        SDL_SetPaletteColors(fontSurface1->format->palette, &c2, 1, 1);
        SDL_BlitSurface(fontSurface1, NULL, g_Surface, &rect_dest);    //����д����Ϸ����--��Ӱɫ

        rect_dest.x = rect2.x;
        rect_dest.y = rect2.y + 1;
        SDL_SetPaletteColors(fontSurface1->format->palette, &c2, 1, 1);
        SDL_BlitSurface(fontSurface1, NULL, g_Surface, &rect_dest);    //����д����Ϸ����--��Ӱɫ

        rect_dest.x = rect2.x;
        rect_dest.y = rect2.y - 1;
        SDL_SetPaletteColors(fontSurface1->format->palette, &c2, 1, 1);
        SDL_BlitSurface(fontSurface1, NULL, g_Surface, &rect_dest);    //����д����Ϸ����--��Ӱɫ


        rect_dest.x = rect2.x;
        rect_dest.y = rect2.y;
        SDL_SetPaletteColors(fontSurface->format->palette, &c, 1, 1);
        SDL_BlitSurface(fontSurface, NULL, g_Surface, &rect_dest);    //����д����Ϸ����

    }*/

    //SDL_FreeSurface(fontSurface);   //�ͷű���

    return 0;
}



// �����ַ���ת��
// flag = 0   Big5 --> GBK
//      = 1   GBK  --> Big5
//      = 2   Big5 --> Unicode
//      = 3   GBK  --> Unicode
// ע��Ҫ��֤dest���㹻�Ŀռ䣬һ�㽨��ȡsrc���ȵ�����+2����֤ȫӢ���ַ�Ҳ��ת��Ϊunicode
int  JY_CharSet(const char* src, char* dest, int flag)
{

    Uint8* psrc, *pdest;
    Uint8 b0, b1;
    int d0;
    Uint16 tmpchar;

    psrc = (Uint8*)src;
    pdest = (Uint8*)dest;

    for (;;)
    {
        b0 = *psrc;
        if (b0 == 0)         //�ַ�������
        {
            if ((flag == 0) || (flag == 1))
            {
                *pdest = 0;
                break;
            }
            else      //unicode������־ 0x0000?
            {
                *pdest = 0;
                *(pdest + 1) = 0;
                break;
            }
        }
        if (b0 < 128)        //Ӣ���ַ�
        {
            if ((flag == 0) || (flag == 1))    //��ת��
            {
                *pdest = b0;
                pdest++;
                psrc++;
            }
            else                  //unicode ����Ӹ�0
            {
                *pdest = b0;
                pdest++;
                *pdest = 0;
                pdest++;
                psrc++;
            }
        }
        else                //�����ַ�
        {
            b1 = *(psrc + 1);
            if (b1 == 0)       // ����������
            {
                *pdest = '?';
                *(pdest + 1) = 0;
                break;
            }
            else
            {
                d0 = b0 + b1 * 256;
                switch (flag)
                {
                case 0:   //Big5 --> GBK
                    tmpchar = big5_gbk[b0 - 128][b1];
                    break;
                case 1:   //GBK  --> Big5
                    tmpchar = gbk_big5[b0 - 128][b1];
                    break;
                case 2:   //Big5 --> Unicode
                    tmpchar = big5_unicode[b0 - 128][b1];
                    break;
                case 3:   //GBK  --> Unicode
                    tmpchar = gbk_unicode[b0 - 128][b1];
                    break;
                default:
                    tmpchar = 0;
                }

                if (tmpchar != 0)
                {
                    *(Uint16*)pdest = tmpchar;
                }
                else
                {
                    *pdest = '?';
                    *(pdest + 1) = '?';
                }

                pdest = pdest + 2;
                psrc = psrc + 2;
            }
        }
    }

    return 0;
}


//��������ļ�
//����ļ�˳�� ��GBK���У�unicode��big5��Ȼ��big5���У�unicode��gbk��
int LoadMB(const char* mbfile)
{
    FILE* fp;
    int i, j;

    Uint16 gbk, big5, unicode;
/*
    fp = fopen(mbfile, "rb");
    if (fp == NULL)
    {
        JY_Error("cannot open mbfile");
        return 1;
    }
*/
    SDL_RWops* rwops = SDL_RWFromFile(mbfile, "rb");
    if (rwops == NULL) {
        JY_Debug("cannot open file %s !", mbfile);
        return 1;
    }

    for (i = 0; i < 128; i++)
    {
        for (j = 0; j < 256; j++)
        {
            gbk_unicode[i][j] = 0;
            gbk_big5[i][j] = 0;
            big5_gbk[i][j] = 0;
            big5_unicode[i][j] = 0;
        }
    }

    for (i = 0x81; i <= 0xfe; i++)
    {
        for (j = 0x40; j <= 0xfe; j++)
        {
            if (j != 0x7f)
            {
                //fread(&unicode, 2, 1, fp);
                //fread(&big5, 2, 1, fp);
                rwops->read(rwops, &unicode, 2, 1);
                rwops->read(rwops, &big5, 2, 1);

                gbk_unicode[i - 128][j] = unicode;
                gbk_big5[i - 128][j] = big5;
            }
        }
    }

    for (i = 0xa0; i <= 0xfe; i++)
    {
        for (j = 0x40; j <= 0xfe; j++)
        {
            if (j <= 0x7e || j >= 0xa1)
            {
                //fread(&unicode, 2, 1, fp);
                //fread(&gbk, 2, 1, fp);
                rwops->read(rwops, &unicode, 2, 1);
                rwops->read(rwops, &gbk, 2, 1);

                big5_unicode[i - 128][j] = unicode;
                big5_gbk[i - 128][j] = gbk;
            }
        }
    }

    //fclose(fp);
    rwops->close(rwops);
    return 0;
}



