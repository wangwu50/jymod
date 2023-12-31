#pragma once

#include "SDL.h"
#include <stdio.h>
#include <vector>
//PicCache.c

// 定义使用的链表
struct CacheNode                        //贴图cache链表节点
{
    CacheNode()
    {}
    ~CacheNode()
    {
        if (s) { SDL_FreeSurface(s); }
        if (t) { SDL_DestroyTexture(t); }
    }
    void toTexture();

    SDL_Surface* s = NULL;                      //此贴图对应的表面
    SDL_Texture* t = NULL;                      //此贴图对应的纹理
    int w = 0;                                  //贴图宽度
    int h = 0;                                  //贴图高度
    int xoff = 0;                               //贴图偏移
    int yoff = 0;
    int id;                                     //贴图编号
    int fileid;                                 //贴图文件编号
};

struct PicFileCache                             //贴图文件链表节点
{
    int num = 0;                                //文件贴图个数
    int* idx = NULL;                            //idx的内容
    int filelength = 0;                         //grp文件长度
    FILE* fp = NULL;                            //grp文件句柄
    unsigned char* grp = NULL;                  //grp的内容
    int width;                                  //指定宽度
    int height;                                 //指定高度
    int percent;                                //指定比例
    std::vector<CacheNode*> pcache;             //文件中所有的贴图对应的cache节点指针，为空则表示没有。
    char path[512];                             //目录
    char suffix[12];                            //后缀名
    int type = 0;                               //0-idx/grp, 1-png path with index.ka
};

#define PIC_FILE_NUM 500                        //???????????(idx/grp)????

int Init_Cache();
int JY_PicInit(const char* PalletteFilename);
int JY_PicLoadFile(const char* idxfilename, const char* grpfilename, int id, int percent);
int JY_LoadPic(int fileid, int picid, int x, int y, int flag, int value,int color = 0, int width = -1, int height = -1, double angle = NULL, SDL_RendererFlip reversal = SDL_FLIP_NONE, int percent = 100);
int LoadPic(int fileid, int picid, struct CacheNode* cache);
int JY_LoadPNGPath(const char* path, int fileid, int num, int percent, const char* suffix);
int JY_LoadPNG(int fileid, int picid, int x, int y, int flag, int value, int px = 0, int py = 0, int pw = -1, int ph = -1, int width = -1, int height = -1, double angle = NULL, SDL_RendererFlip reversal = SDL_FLIP_NONE, int percent = 100);
int JY_GetPNGXY(int fileid, int picid, int* w, int* h, int* xoff, int* yoff);
int JY_GetPicXY(int fileid, int picid, int* w, int* h, int* xoff, int* yoff);
SDL_Texture* CreateTextureFromRLE(unsigned char* data, int w, int h, int datalong);
int LoadPalette(const char* filename);
int RenderTexture(SDL_Texture* lps, int x, int y, int flag, int value, int color, int width = -1, int height = -1, double angle = NULL , SDL_RendererFlip reversal = SDL_FLIP_NONE, int percent = 100);

