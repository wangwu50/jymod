
// 读取idx/grp的贴图文件。
// 为提高速度，采用缓存方式读取。把idx/grp读入内存，然后定义若干个缓存表面
// 经常访问的pic放在缓存表面中

#include <stdlib.h>
#include <SDL_rwops.h>
#include "piccache.h"
#include "jymain.h"
#include "sdlfun.h"

PicFileCache pic_file[PIC_FILE_NUM];
//std::forward_list<CacheNode*> pic_cache;     //pic_cache链表
Uint32 m_color32[256];               // 256调色板
//int CacheFailNum = 0;

void CacheNode::toTexture()
{
    if (s)
    {
        if (t == NULL)
        {
            t = SDL_CreateTextureFromSurface(g_Renderer, s);
            SDL_FreeSurface(s);
            s = NULL;
        }
    }
}

// 初始化Cache数据。游戏开始时调用
int Init_Cache()
{
    int i;
    for (i = 0; i < PIC_FILE_NUM; i++)
    {
        pic_file[i].num = 0;
        pic_file[i].idx = NULL;
        pic_file[i].grp = NULL;
        pic_file[i].fp = NULL;
        //pic_file[i].pcache = NULL;
    }
    return 0;
}

// 初始化贴图cache信息
// PalletteFilename 为256调色板文件。第一次调用时载入
//                  为空字符串则表示重新清空贴图cache信息。在主地图/场景/战斗切换时调用

int JY_PicInit(const char* PalletteFilename)
{
    struct list_head* pos, *p;
    int i;

    LoadPalette(PalletteFilename);   //载入调色板

    //如果链表不为空，删除全部链表
    //for (auto& c : pic_cache)
    //{
    //    delete c;
    //}
    //pic_cache.clear();

    for (i = 0; i < PIC_FILE_NUM; i++)
    {
        pic_file[i].num = 0;
        SafeFree(pic_file[i].idx);
        SafeFree(pic_file[i].grp);

        for (auto& p : pic_file[i].pcache)
        {
            delete p;
        }
        pic_file[i].pcache.clear();
        //SafeFree(pic_file[i].pcache);
        if (pic_file[i].fp)
        {
            fclose(pic_file[i].fp);
            pic_file[i].fp = NULL;
        }
    }
    JY_LoadPNGPath("cloud", 97, 8, 100*g_Zoom, "png");
    //CacheFailNum = 0;
    return 0;
}

// 加载文件信息
// filename 文件名
// id  0 - PIC_FILE_NUM-1
int JY_PicLoadFile(const char* idxfilename, const char* grpfilename, int id, int percent)
{
    int i;
    struct CacheNode* tmpcache;
//    FILE* fp;

    if (id < 0 || id >= PIC_FILE_NUM)    // id超出范围
    {
        return 1;
    }

    if (pic_file[id].pcache.size())          //释放当前文件占用的空间，并清理cache
    {
        int i;
        for (i = 0; i < pic_file[id].num; i++)     //循环全部贴图，
        {
            tmpcache = pic_file[id].pcache[i];
            if (tmpcache)         // 该贴图有缓存则删除
            {
                delete tmpcache;
            }
        }
        //SafeFree(pic_file[id].pcache);
    }
    SafeFree(pic_file[id].idx);
    SafeFree(pic_file[id].grp);
    if (pic_file[id].fp)
    {
        fclose(pic_file[id].fp);
        pic_file[id].fp = NULL;
    }

    // 读取idx文件
    SDL_RWops* rwopsIdx = SDL_RWFromFile(idxfilename, "rb");
    if (rwopsIdx == NULL) {
        JY_Debug("cannot load file %s !", idxfilename);
        return 1;
    }
    //pic_file[id].num = FileLength(idxfilename) / 4;    //idx 贴图个数
    pic_file[id].num = rwopsIdx->size(rwopsIdx) / 4;
    pic_file[id].idx = (int*)malloc((pic_file[id].num + 1) * 4);
    if (pic_file[id].idx == NULL)
    {
        JY_Error("JY_PicLoadFile: cannot malloc idx memory!\n");
        return 1;
    }
    //读取贴图idx文件
    /*
    if ((fp = fopen(idxfilename, "rb")) == NULL)
    {
        JY_Error("JY_PicLoadFile: idx file not open ---%s", idxfilename);
        return 1;
    }

    fread(&pic_file[id].idx[1], 4, pic_file[id].num, fp);
    fclose(fp);
    */

    rwopsIdx->read(rwopsIdx, &pic_file[id].idx[1], 4, pic_file[id].num);
    rwopsIdx->close(rwopsIdx);

    pic_file[id].idx[0] = 0;

    //读取grp文件
    SDL_RWops* rwopsGrp = SDL_RWFromFile(grpfilename, "rb");
    if (rwopsGrp == NULL) {
        JY_Debug("cannot load file %s !", grpfilename);
        return 1;
    }
    pic_file[id].filelength = rwopsGrp->size(rwopsGrp);

    //读取贴图grp文件
    /*
    if ((fp = fopen(grpfilename, "rb")) == NULL)
    {
        JY_Error("JY_PicLoadFile: grp file not open ---%s", grpfilename);
        return 1;
    }
    if (g_PreLoadPicGrp == 1)     //grp文件读入内存
    {
        pic_file[id].grp = (unsigned char*)malloc(pic_file[id].filelength);
        if (pic_file[id].grp == NULL)
        {
            JY_Error("JY_PicLoadFile: cannot malloc grp memory!\n");
            return 1;
        }
        fread(pic_file[id].grp, 1, pic_file[id].filelength, fp);
        fclose(fp);
    }
    else
    {
        pic_file[id].fp = fp;
    }*/

    pic_file[id].grp = (unsigned char*)malloc(pic_file[id].filelength);
    if (pic_file[id].grp == NULL)
    {
        JY_Error("JY_PicLoadFile: cannot malloc grp memory!\n");
        return 1;
    }

    rwopsGrp->read(rwopsGrp, pic_file[id].grp, 1, pic_file[id].filelength);
    rwopsGrp->close(rwopsGrp);

    pic_file[id].pcache.resize(pic_file[id].num);
    if (pic_file[id].pcache.size() == 0)
    {
        JY_Error("JY_PicLoadFile: cannot malloc pcache memory!\n");
        return 1;
    }

    for (i = 0; i < pic_file[id].num; i++)
    {
        pic_file[id].pcache[i] = NULL;
    }

    pic_file[id].percent = percent;

    if(percent == 0)
    {
        percent = (int)(g_Zoom*g_percentValue);
    }
    pic_file[id].percent = percent;
    pic_file[id].type = 0;
    return 0;
}

// 加载并显示贴图
// fileid        贴图文件id
// picid     贴图编号
// x,y       显示位置
//  flag 不同bit代表不同含义，缺省均为0
//  B0    0 考虑偏移xoff，yoff。=1 不考虑偏移量
//  B1    0     , 1 与背景alpla 混合显示, value 为alpha值(0-256), 0表示透明
//  B2            1 全黑
//  B3            1 全白
//  value 按照flag定义，为alpha值，
//  color 颜色
//  width 宽度
//  height 长度
//  rotate 旋转
//  reversal 反转 1水平反转，2竖直反转
//  percent 比例
int JY_LoadPic(int fileid, int picid, int x, int y, int flag, int value, int color,
               int width, int height, double rotate, SDL_RendererFlip reversal, int percent)
{
    struct CacheNode* newcache;
    int xnew, ynew, xoff, yoff;

    if (pic_file[fileid].type == 1)
    {
        JY_LoadPNG(fileid, picid, x, y, flag, value, 0,0,-1,-1,width,height,rotate,reversal, percent);
        return 0;
    }

    double zoom = 1;

    picid = picid / 2;

    if (fileid < 0 || fileid >= PIC_FILE_NUM || picid < 0 || picid >= pic_file[fileid].num)    // 参数错误
    {
        return 1;
    }

    if (pic_file[fileid].pcache[picid] == NULL)     //当前贴图没有加载
    {
        //生成cache数据
        newcache = new CacheNode();
        if (newcache == NULL)
        {
            JY_Error("JY_LoadPic: cannot malloc newcache memory!\n");
            return 1;
        }

        newcache->id = picid;
        newcache->fileid = fileid;
        LoadPic(fileid, picid, newcache);

        //指定比例
        zoom = (double)pic_file[fileid].percent/100.0;
        if(pic_file[fileid].percent > 0 && pic_file[fileid].percent != 100 && zoom != 0 && zoom != 1)
        {
            newcache->w = (int)(zoom * newcache->w);
            newcache->h = (int)(zoom * newcache->h);
            newcache->xoff = (int)(zoom * newcache->xoff);
            newcache->yoff = (int)(zoom * newcache->yoff);
        }
        pic_file[fileid].pcache[picid] = newcache;
    }
    else
    {
        newcache = pic_file[fileid].pcache[picid];
    }

    if (newcache->t == NULL)     //贴图为空，直接退出
    {
        return 1;
    }
    if (width > 0 && height >0)
    {
        xoff = newcache->xoff*width / newcache->w;
        yoff = newcache->yoff*height /newcache->h;
        width = width;
        height = height;
    }
    else if (width > 0 && height <= 0)
    {
        xoff = newcache->xoff*width / newcache->w;
        yoff = newcache->yoff*width / newcache->w;
        width = width;
        height = newcache->h *width/newcache->w;
    }
    else if (width <= 0 && height > 0)
    {
        xoff = newcache->xoff*height / newcache->h;
        yoff = newcache->yoff*height / newcache->h;
        width = newcache->w *height/newcache->h;
        height = height;
    }
    else{
        xoff = newcache->xoff;
        yoff = newcache->yoff;
        width = newcache->w;
        height = newcache->h;
    }

    if (flag & 0x00000001)
    {
        xnew = x;
        ynew = y;
    }
    else
    {
        xnew = x - xoff;
        ynew = y - yoff;
    }
//    JY_Debug("g_zoom:%f,width:%d,height:%d,w:%d,h:%d",g_Zoom,width,height,newcache->w,newcache->h);
    RenderTexture(newcache->t, xnew, ynew, flag, value, color, width, height, rotate, reversal, percent);
    return 0;
}

// 加载贴图到表面
int LoadPic(int fileid, int picid, struct CacheNode* cache)
{

    SDL_RWops* fp_SDL;
    int id1, id2;
    int datalong;
    unsigned char* p, *data;

    SDL_Surface* tmpsurf = NULL, *tmpsur;

    if (pic_file[fileid].idx == NULL)
    {
        JY_Error("LoadPic: fileid %d can not load?\n", fileid);
        return 1;
    }
    id1 = pic_file[fileid].idx[picid];
    id2 = pic_file[fileid].idx[picid + 1];

    // 处理一些特殊情况，按照修改器中的代码
    if (id1 < 0)
    {
        datalong = 0;
    }

    if (id2 > pic_file[fileid].filelength)
    {
        id2 = pic_file[fileid].filelength;
    }

    datalong = id2 - id1;

    if (datalong > 0)
    {
        //读取贴图grp文件，得到原始数据
        if (g_PreLoadPicGrp == 1)           //有预读，从内存中读数据
        {
            data = pic_file[fileid].grp + id1;
            p = NULL;
        }
        else         //没有预读，从文件中读取
        {
            fseek(pic_file[fileid].fp, id1, SEEK_SET);
            data = (unsigned char*)malloc(datalong);
            p = data;
            fread(data, 1, datalong, pic_file[fileid].fp);
        }

        fp_SDL = SDL_RWFromMem(data, datalong);
        if (IMG_isPNG(fp_SDL) == 0)
        {
            int w, h;
            w = *(short*)data;
            h = *(short*)(data + 2);
            cache->xoff = *(short*)(data + 4);
            cache->yoff = *(short*)(data + 6);
            cache->w = w;
            cache->h = h;
            cache->t = CreateTextureFromRLE(data + 8, w, h, datalong - 8);
            //cache->t = SDL_CreateTextureFromSurface(g_Renderer, cache->s);
            //SDL_FreeSurface(cache->s);
            cache->s = NULL;
        }
        else        //读取png格式
        {
            tmpsurf = IMG_LoadPNG_RW(fp_SDL);
            if (tmpsurf == NULL)
            {
                JY_Error("LoadPic: cannot create SDL_Surface tmpsurf!\n");
            }
            cache->xoff = tmpsurf->w / 2;
            cache->yoff = tmpsurf->h;
            cache->w = tmpsurf->w;
            cache->h = tmpsurf->h;
            cache->s = tmpsurf;
            cache->toTexture();
            //cache->t = SDL_CreateTextureFromSurface(g_Renderer, cache->s);
            //SDL_FreeSurface(cache->s);
            //cache->s = NULL;
        }
        //SDL_FreeRW(fp_SDL);
        fp_SDL->close(fp_SDL);
        SafeFree(p);
    }
    else
    {
        cache->s = NULL;
        cache->t = NULL;
        cache->xoff = 0;
        cache->yoff = 0;
    }

    return 0;
}


//得到贴图大小
int JY_GetPicXY(int fileid, int picid, int* w, int* h, int* xoff, int* yoff)
{
    struct CacheNode* newcache;
    int r = JY_LoadPic(fileid, picid, g_ScreenW + 1, g_ScreenH + 1, 1, 0);   //加载贴图到看不见的位置

    *w = 0;
    *h = 0;
    *xoff = 0;
    *yoff = 0;

    if (r != 0)
    {
        return 1;
    }

    newcache = pic_file[fileid].pcache[picid / 2];

    if (newcache->t)        // 已有，则直接显示
    {
        *w = newcache->w;
        *h = newcache->h;
        *xoff = newcache->xoff;
        *yoff = newcache->yoff;
    }

    return 0;
}

//按照原来游戏的RLE格式创建表面
SDL_Texture* CreateTextureFromRLE(unsigned char* data, int w, int h, int datalong)
{
    /*
    int p = 0;
    int i, j;
    int yoffset;
    int row;
    int start;
    int x;
    int solidnum;
    SDL_Surface* ps1, *ps2;
    Uint32* data32 = NULL;

    data32 = (Uint32*)malloc(w * h * 4);
    if (data32 == NULL)
    {
        JY_Error("CreatePicSurface32: cannot malloc data32 memory!\n");
        return NULL;
    }

    for (i = 0; i < w * h; i++)
    {
        data32[i] = 0;
    }

    for (i = 0; i < h; i++)
    {
        yoffset = i * w;
        row = data[p];            // i行数据个数
        start = p;
        p++;
        if (row > 0)
        {
            x = 0;                // i行目前列
            for (;;)
            {
                x = x + data[p];    // i行空白点个数，跳个透明点
                if (x >= w)        // i行宽度到头，结束
                {
                    break;
                }

                p++;
                solidnum = data[p];  // 不透明点个数
                p++;
                for (j = 0; j < solidnum; j++)
                {
                    if (g_Rotate == 0)
                    {
                        data32[yoffset + x] = m_color32[data[p]] | AMASK;
                    }
                    else
                    {
                        data32[h - i - 1 + x * h] = m_color32[data[p]] | AMASK;
                    }
                    p++;
                    x++;
                }
                if (x >= w)
                {
                    break;
                }     // i行宽度到头，结束
                if (p - start >= row)
                {
                    break;
                }    // i行没有数据，结束
            }
            if (p + 1 >= datalong)
            {
                break;
            }
        }
    }
    ps1 = SDL_CreateRGBSurfaceFrom(data32, w, h, 32, w * 4, RMASK, GMASK, BMASK, AMASK);  //创建32位表面
    if (ps1 == NULL)
    {
        JY_Error("CreatePicSurface32: cannot create SDL_Surface ps1!\n");
    }
    ps2 = SDL_ConvertSurfaceFormat(ps1, g_Surface->format->format, 0);
    if (ps2 == NULL)
    {
        JY_Error("CreatePicSurface32: cannot create SDL_Surface ps2!\n");
    }
    auto tex = SDL_CreateTextureFromSurface(g_Renderer, ps2);
    SDL_FreeSurface(ps1);
    SDL_FreeSurface(ps2);
    SafeFree(data32);
    return tex;
     */

    int p=0;
    int i,j;
    int yoffset;
    int row;
    int start;
    int x;
    int solidnum;
    Uint32 *data32=NULL;
    SDL_Texture* tex = NULL;

    data32=(Uint32 *)malloc(w*h*4);
    if(data32==NULL){
        JY_Error("CreatePicSurface32: cannot malloc data32 memory!\n");
        return NULL;
    }


    for(i=0;i<w*h;i++)
        data32[i]=0;

    tex = SDL_CreateTexture(g_Renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,w,h);



    for(i=0;i<h;i++){
        yoffset=i*w;
        row=data[p];            // i行数据个数
        start=p;
        p++;
        if(row>0){
            x=0;                // i行目前列
            for(;;){
                x=x+data[p];    // i行空白点个数，跳个透明点
                if(x>=w)        // i行宽度到头，结束
                    break;

                p++;
                solidnum=data[p];  // 不透明点个数
                p++;
                for(j=0;j<solidnum;j++){
                    data32[yoffset+x]=m_color32[data[p]] | 0xff000000;
                    p++;
                    x++;
                }
                if(x>=w)
                    break;     // i行宽度到头，结束
                if(p-start>=row)
                    break;    // i行没有数据，结束
            }
            if(p+1>=datalong)
                break;
        }
    }

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    SDL_UpdateTexture(tex, NULL, data32, w*4);

    if(tex == NULL)
    {
        JY_Error("CreatePicSurface32: cannot create SDL_CreateTextureFromSurface tex!\n");
    }
    SafeFree(data32);
    return tex;

}

// 读取调色板
// 文件名为空则直接返回
int LoadPalette(const char* filename)
{
    FILE* fp;
    char color[3];
    int i;
    if (strlen(filename) == 0)
    {
        return 1;
    }
    /*
    if ((fp = fopen(filename, "rb")) == NULL)
    {
        JY_Error("palette File not open ---%s", filename);
        return 1;
    }
     */
    SDL_RWops* rwops = SDL_RWFromFile(filename, "rb");
    if (rwops == NULL) {
        JY_Debug("cannot load  file %s !", filename);
        return 1;
    }
    for (i = 0; i < 256; i++)
    {
        //fread(color, 1, 3, fp);
        rwops->read(rwops, color, 1, 3);
        m_color32[i] = color[0] * 4 * 65536l + color[1] * 4 * 256 + color[2] * 4 + 0x000000;

    }


    rwops->close(rwops);
    //fclose(fp);

    return 0;
}


int JY_LoadPNGPath(const char* path, int fileid, int num, int percent, const char* suffix)
{
    int i;
    struct CacheNode* tmpcache;
    if (fileid < 0 || fileid >= PIC_FILE_NUM)    // id超出范围
    {
        return 1;
    }

    if (pic_file[fileid].pcache.size())          //释放当前文件占用的空间，并清理cache
    {
        int i;
        for (i = 0; i < pic_file[fileid].num; i++)     //循环全部贴图，
        {
            tmpcache = pic_file[fileid].pcache[i];
            if (tmpcache)         // 该贴图有缓存则删除
            {
                delete tmpcache;
            }
        }
        //SafeFree(pic_file[fileid].pcache);
    }
    pic_file[fileid].type = 1;
    pic_file[fileid].num = num;
    sprintf(pic_file[fileid].path, "%s", path);

    pic_file[fileid].pcache.resize(pic_file[fileid].num);
    if (pic_file[fileid].pcache.size() == 0)
    {
        JY_Error("JY_LoadPNGPath: cannot malloc pcache memory!\n");
        return 1;
    }
    for (i = 0; i < pic_file[fileid].num; i++)
    {
        pic_file[fileid].pcache[i] = NULL;
    }

    pic_file[fileid].percent = percent;
    sprintf(pic_file[fileid].suffix, "%s", suffix);

    return 0;
}

int JY_LoadPNG(int fileid, int picid, int x, int y, int flag, int value, int px, int py, int pw, int ph,
               int width,int height,double rotate, SDL_RendererFlip reversal,int percent)
{
    struct CacheNode* newcache, *tmpcache;
    SDL_Surface* tmpsur;
    SDL_Rect r;
    SDL_Rect rect1;
    SDL_Rect rect2;
    int w, h;

    picid = picid / 2;

    if (fileid < 0 || fileid >= PIC_FILE_NUM || picid < 0 || picid >= pic_file[fileid].num)    // 参数错误
    {
        return 1;
    }

    if (pic_file[fileid].pcache[picid] == NULL)     //当前贴图没有加载
    {
        char str[512];
        SDL_RWops* fp_SDL;
        double zoom = (double)pic_file[fileid].percent / 100.0;

        //sprintf(str, "%s/%d.png", pic_file[fileid].path, picid);
        sprintf(str,"%s/%d.%s",pic_file[fileid].path, picid,pic_file[fileid].suffix );

        //生成cache数据
        newcache = new CacheNode();
        if (newcache == NULL)
        {
            JY_Error("JY_LoadPNG: cannot malloc newcache memory!\n");
            return 1;
        }

        newcache->id = picid;
        newcache->fileid = fileid;

        fp_SDL = SDL_RWFromFile(str, "rb");
        if (IMG_isPNG(fp_SDL))
        {
            tmpsur = IMG_LoadPNG_RW(fp_SDL);
            if (tmpsur == NULL)
            {
                JY_Error("JY_LoadPNG: cannot create SDL_Surface tmpsurf!\n");
                return 1;
            }

            newcache->xoff = tmpsur->w / 2;
            newcache->yoff = tmpsur->h / 2;
            newcache->w = tmpsur->w;
            newcache->h = tmpsur->h;
            newcache->s = tmpsur;
            newcache->toTexture();
            //newcache->t = SDL_CreateTextureFromSurface(g_Renderer, newcache->s);
            //SDL_FreeSurface(newcache->s);
            //newcache->s = NULL;
        }
        else
        {
            newcache->s = NULL;
            newcache->t = NULL;
            newcache->xoff = 0;
            newcache->yoff = 0;
        }

        SDL_FreeRW(fp_SDL);

        //指定比例
        if (pic_file[fileid].percent > 0 && pic_file[fileid].percent != 100 && zoom != 0 && zoom != 1)
        {
            newcache->w = (int)(zoom * newcache->w);
            newcache->h = (int)(zoom * newcache->h);
            //tmpsur = newcache->t;
            //newcache->s = zoomSurface(tmpsur, zoom, zoom, SMOOTHING_ON);
            newcache->xoff = (int)(zoom * newcache->xoff);
            newcache->yoff = (int)(zoom * newcache->yoff);
            //SDL_SetColorKey(newcache->s,SDL_SRCCOLORKEY|SDL_RLEACCEL ,ConvertColor(g_MaskColor32));  //透明色
            //SDL_FreeSurface(tmpsur);
        }
        pic_file[fileid].pcache[picid] = newcache;
    }
    else     //已加载贴图
    {
        newcache = pic_file[fileid].pcache[picid];
    }

    if (newcache->t == NULL)     //贴图为空，直接退出
    {
        JY_Debug("pic is null");
        return 1;
    }

    if (flag & 0x00000001)
    {
        r.x = x;
        r.y = y;
    }
    else
    {
        r.x = x - newcache->xoff;
        r.y = y - newcache->yoff;
    }

    //SDL_BlitSurface(newcache->s, NULL, g_Surface, &r);


    rect1.x=x;
    rect1.y=y;

    rect1.w = newcache->w;
    rect1.h = newcache->h;

    SDL_QueryTexture(newcache->t, 0, 0, &w, &h);
    rect2.x = 0;
    rect2.y = 0;
    rect2.w = w;
    rect2.h = h;


    if(pw >= 0 && ph >= 0){

        double zoom = (double)pic_file[fileid].percent/100.0;
        rect2.x = px;
        rect2.y = py;
        rect2.w = (int)(pw/zoom);
        rect2.h = (int)(ph/zoom);

        rect1.w = pw-px;
        rect1.h = ph-py;

    }
    if (x == -1 && y == -1){
        rect1.x = 0;
        rect1.y = 0;
        rect1.w = g_ScreenW;
        rect1.h = g_ScreenH;
    }
    if (width > 0 && height > 0)
    {
        rect1.w = width;
        rect1.h = height;
    }
    else if (width > 0 && height <= 0)
    {
        rect1.h = width * rect1.h / rect1.w;
        rect1.w = width;
    }
    rect1.w *= percent / 100.0;
    rect1.h *= percent / 100.0;
    if((flag & 0x2)==0){        // 没有alpla

    }
    else{
        SDL_SetTextureAlphaMod(newcache->t,(Uint8)value);
    }
    SDL_SetRenderTarget(g_Renderer, g_Texture);
    SDL_RenderCopyEx(g_Renderer, newcache->t, &rect2, &rect1,rotate,NULL,reversal);

    //r.w = newcache->w;
    //r.h = newcache->h;
    //SDL_SetRenderTarget(g_Renderer, g_Texture);
    //SDL_RenderCopy(g_Renderer, newcache->t, NULL, &r);

    return 0;
}

int JY_GetPNGXY(int fileid, int picid, int* w, int* h, int* xoff, int* yoff)
{
    int r = JY_LoadPNG(fileid, picid, g_ScreenW+1,g_ScreenH+1,1,0,0,0,0,0);   //加载贴图到看不见的位置

    *w = 0;
    *h = 0;
    *xoff = 0;
    *yoff = 0;

    if (r != 0)
    {
        return 1;
    }

    auto newcache = pic_file[fileid].pcache[picid / 2];

    if (newcache->t)        // 已有，则直接显示
    {
        *w = newcache->w;
        *h = newcache->h;
        *xoff = newcache->xoff;
        *yoff = newcache->yoff;
    }

    return 0;
}


// 把表面blit到背景或者前景表面
// x,y 要加载到表面的左上角坐标
int RenderTexture(SDL_Texture* lps, int x, int y, int flag, int value, int color, int width, int height, double rotate, SDL_RendererFlip reversal, int percent)
{
 /*
    SDL_Rect rect;

    if(value>255)
        value=255;

    rect.x=(Sint16)x;
    rect.y=(Sint16)y;

    rect.w = width;
    rect.h = height;

    if(!lps)
    {
        JY_Error("BlitSurface: lps is null!");
        return 1;
    }

    if((flag & 0x2)==0){        // 没有alpla
        RenderToTexture(lps, NULL, g_Texture, &rect);
    }
    else{  // 有alpha

        if( (flag &0x4) || (flag &0x8) || (flag &0x10)){   // 黑白
            Uint8 r, g, b, a;
            if(flag &0x4)
            {
                r = 0;
                g = 0;
                b = 0;
                a = value;
            }
            else if(flag &0x8)
            {
                r = 0;
                g = 215;
                b = 0;
                a = value;
            }
            else
            {
                r=(Uint8) ((color & 0xff0000) >>16);
                g=(Uint8) ((color & 0xff00)>>8);
                b=(Uint8) ((color & 0xff));
                a = value;

            }

            SDL_SetTextureBlendMode(lps, SDL_BLENDMODE_BLEND);
            SDL_SetTextureColorMod(lps,r,g,b);
            SDL_SetTextureAlphaMod(lps,(Uint8)a);
            RenderToTexture(lps, NULL, g_Texture, &rect);

            SDL_SetTextureColorMod(lps,255,255,255);

        }
        else{

            SDL_SetTextureAlphaMod(lps,(Uint8)value);
            RenderToTexture(lps, NULL, g_Texture, &rect);
        }
    }

    return 0; */

    SDL_Surface* tmps;
    SDL_Rect rect, rect0;
    int i, j;
    //color = ConvertColor(g_MaskColor32);
    if (value > 255)
    {
        value = 255;
    }
    rect.x = x;
    rect.y = y;
    SDL_QueryTexture(lps, NULL, NULL, &rect.w, &rect.h);

    rect.w *= percent / 100.0;
    rect.h *= percent / 100.0;

    if (width > 0 && height > 0)
    {
        rect.w = width;
        rect.h = height;
    }
    else if (width > 0 && height <= 0)
    {
        rect.h = width * rect.h / rect.w;
        rect.w = width;
    }
    rect0 = rect;
    rect0.x = 0;
    rect0.y = 0;
    if (!lps)
    {
        JY_Error("BlitSurface: lps is null!");
        return 1;
    }

    if ((flag & 0x2) == 0)          // 没有alpha
    {
        SDL_SetTextureColorMod(lps, 255, 255, 255);
        SDL_SetTextureBlendMode(lps, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(lps, 255);
        RenderToTexture(lps, NULL, g_Texture, &rect, rotate, NULL, reversal);
        //SDL_BlitSurface(lps, NULL, g_Surface, &rect);
    }
    else    // 有alpha
    {
        if ((flag & 0x4) || (flag & 0x8) || (flag & 0x10))     // 4-黑, 8-白, 16-颜色
        {
            Uint8 r, g, b, a;
            if(flag &0x4)
            {
                r = 0;
                g = 0;
                b = 0;
                a = value;
            }
            else if(flag &0x8)
            {
                r = 0;
                g = 215;
                b = 0;
                a = value;
            }
            else
            {
                r=(Uint8) ((color & 0xff0000) >>16);
                g=(Uint8) ((color & 0xff00)>>8);
                b=(Uint8) ((color & 0xff));
                a = value;

            }
            SDL_SetTextureBlendMode(lps, SDL_BLENDMODE_BLEND);
            SDL_SetTextureColorMod(lps,r,g,b);
            SDL_SetTextureAlphaMod(lps,(Uint8)a);
            RenderToTexture(lps, NULL, g_Texture, &rect, rotate, NULL, reversal);

            SDL_SetTextureColorMod(lps,255,255,255);

        }
        else
        {
            SDL_SetTextureColorMod(lps, 255, 255, 255);
            SDL_SetTextureBlendMode(lps, SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(lps, (Uint8)value);
            RenderToTexture(lps, NULL, g_Texture, &rect,rotate, NULL, reversal);
            //SDL_BlitSurface(lps, NULL, g_Surface, &rect);
        }
    }
    return 0;

}


