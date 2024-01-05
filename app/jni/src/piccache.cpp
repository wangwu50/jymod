
// ��ȡidx/grp����ͼ�ļ���
// Ϊ����ٶȣ����û��淽ʽ��ȡ����idx/grp�����ڴ棬Ȼ�������ɸ��������
// �������ʵ�pic���ڻ��������

#include <stdlib.h>
#include <SDL_rwops.h>
#include "piccache.h"
#include "jymain.h"
#include "sdlfun.h"

PicFileCache pic_file[PIC_FILE_NUM];
//std::forward_list<CacheNode*> pic_cache;     //pic_cache����
Uint32 m_color32[256];               // 256��ɫ��
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

// ��ʼ��Cache���ݡ���Ϸ��ʼʱ����
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

// ��ʼ����ͼcache��Ϣ
// PalletteFilename Ϊ256��ɫ���ļ�����һ�ε���ʱ����
//                  Ϊ���ַ������ʾ���������ͼcache��Ϣ��������ͼ/����/ս���л�ʱ����

int JY_PicInit(const char* PalletteFilename)
{
    struct list_head* pos, *p;
    int i;

    LoadPalette(PalletteFilename);   //�����ɫ��

    //�������Ϊ�գ�ɾ��ȫ������
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

// �����ļ���Ϣ
// filename �ļ���
// id  0 - PIC_FILE_NUM-1
int JY_PicLoadFile(const char* idxfilename, const char* grpfilename, int id, int percent)
{
    int i;
    struct CacheNode* tmpcache;
//    FILE* fp;

    if (id < 0 || id >= PIC_FILE_NUM)    // id������Χ
    {
        return 1;
    }

    if (pic_file[id].pcache.size())          //�ͷŵ�ǰ�ļ�ռ�õĿռ䣬������cache
    {
        int i;
        for (i = 0; i < pic_file[id].num; i++)     //ѭ��ȫ����ͼ��
        {
            tmpcache = pic_file[id].pcache[i];
            if (tmpcache)         // ����ͼ�л�����ɾ��
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

    // ��ȡidx�ļ�
    SDL_RWops* rwopsIdx = SDL_RWFromFile(idxfilename, "rb");
    if (rwopsIdx == NULL) {
        JY_Debug("cannot load file %s !", idxfilename);
        return 1;
    }
    //pic_file[id].num = FileLength(idxfilename) / 4;    //idx ��ͼ����
    pic_file[id].num = rwopsIdx->size(rwopsIdx) / 4;
    pic_file[id].idx = (int*)malloc((pic_file[id].num + 1) * 4);
    if (pic_file[id].idx == NULL)
    {
        JY_Error("JY_PicLoadFile: cannot malloc idx memory!\n");
        return 1;
    }
    //��ȡ��ͼidx�ļ�
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

    //��ȡgrp�ļ�
    SDL_RWops* rwopsGrp = SDL_RWFromFile(grpfilename, "rb");
    if (rwopsGrp == NULL) {
        JY_Debug("cannot load file %s !", grpfilename);
        return 1;
    }
    pic_file[id].filelength = rwopsGrp->size(rwopsGrp);

    //��ȡ��ͼgrp�ļ�
    /*
    if ((fp = fopen(grpfilename, "rb")) == NULL)
    {
        JY_Error("JY_PicLoadFile: grp file not open ---%s", grpfilename);
        return 1;
    }
    if (g_PreLoadPicGrp == 1)     //grp�ļ������ڴ�
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

// ���ز���ʾ��ͼ
// fileid        ��ͼ�ļ�id
// picid     ��ͼ���
// x,y       ��ʾλ��
//  flag ��ͬbit����ͬ���壬ȱʡ��Ϊ0
//  B0    0 ����ƫ��xoff��yoff��=1 ������ƫ����
//  B1    0     , 1 �뱳��alpla �����ʾ, value Ϊalphaֵ(0-256), 0��ʾ͸��
//  B2            1 ȫ��
//  B3            1 ȫ��
//  value ����flag���壬Ϊalphaֵ��
//  color ��ɫ
//  width ���
//  height ����
//  rotate ��ת
//  reversal ��ת 1ˮƽ��ת��2��ֱ��ת
//  percent ����
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

    if (fileid < 0 || fileid >= PIC_FILE_NUM || picid < 0 || picid >= pic_file[fileid].num)    // ��������
    {
        return 1;
    }

    if (pic_file[fileid].pcache[picid] == NULL)     //��ǰ��ͼû�м���
    {
        //����cache����
        newcache = new CacheNode();
        if (newcache == NULL)
        {
            JY_Error("JY_LoadPic: cannot malloc newcache memory!\n");
            return 1;
        }

        newcache->id = picid;
        newcache->fileid = fileid;
        LoadPic(fileid, picid, newcache);

        //ָ������
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

    if (newcache->t == NULL)     //��ͼΪ�գ�ֱ���˳�
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

// ������ͼ������
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

    // ����һЩ��������������޸����еĴ���
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
        //��ȡ��ͼgrp�ļ����õ�ԭʼ����
        if (g_PreLoadPicGrp == 1)           //��Ԥ�������ڴ��ж�����
        {
            data = pic_file[fileid].grp + id1;
            p = NULL;
        }
        else         //û��Ԥ�������ļ��ж�ȡ
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
        else        //��ȡpng��ʽ
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


//�õ���ͼ��С
int JY_GetPicXY(int fileid, int picid, int* w, int* h, int* xoff, int* yoff)
{
    struct CacheNode* newcache;
    int r = JY_LoadPic(fileid, picid, g_ScreenW + 1, g_ScreenH + 1, 1, 0);   //������ͼ����������λ��

    *w = 0;
    *h = 0;
    *xoff = 0;
    *yoff = 0;

    if (r != 0)
    {
        return 1;
    }

    newcache = pic_file[fileid].pcache[picid / 2];

    if (newcache->t)        // ���У���ֱ����ʾ
    {
        *w = newcache->w;
        *h = newcache->h;
        *xoff = newcache->xoff;
        *yoff = newcache->yoff;
    }

    return 0;
}

//����ԭ����Ϸ��RLE��ʽ��������
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
        row = data[p];            // i�����ݸ���
        start = p;
        p++;
        if (row > 0)
        {
            x = 0;                // i��Ŀǰ��
            for (;;)
            {
                x = x + data[p];    // i�пհ׵����������͸����
                if (x >= w)        // i�п�ȵ�ͷ������
                {
                    break;
                }

                p++;
                solidnum = data[p];  // ��͸�������
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
                }     // i�п�ȵ�ͷ������
                if (p - start >= row)
                {
                    break;
                }    // i��û�����ݣ�����
            }
            if (p + 1 >= datalong)
            {
                break;
            }
        }
    }
    ps1 = SDL_CreateRGBSurfaceFrom(data32, w, h, 32, w * 4, RMASK, GMASK, BMASK, AMASK);  //����32λ����
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
        row=data[p];            // i�����ݸ���
        start=p;
        p++;
        if(row>0){
            x=0;                // i��Ŀǰ��
            for(;;){
                x=x+data[p];    // i�пհ׵����������͸����
                if(x>=w)        // i�п�ȵ�ͷ������
                    break;

                p++;
                solidnum=data[p];  // ��͸�������
                p++;
                for(j=0;j<solidnum;j++){
                    data32[yoffset+x]=m_color32[data[p]] | 0xff000000;
                    p++;
                    x++;
                }
                if(x>=w)
                    break;     // i�п�ȵ�ͷ������
                if(p-start>=row)
                    break;    // i��û�����ݣ�����
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

// ��ȡ��ɫ��
// �ļ���Ϊ����ֱ�ӷ���
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
    if (fileid < 0 || fileid >= PIC_FILE_NUM)    // id������Χ
    {
        return 1;
    }

    if (pic_file[fileid].pcache.size())          //�ͷŵ�ǰ�ļ�ռ�õĿռ䣬������cache
    {
        int i;
        for (i = 0; i < pic_file[fileid].num; i++)     //ѭ��ȫ����ͼ��
        {
            tmpcache = pic_file[fileid].pcache[i];
            if (tmpcache)         // ����ͼ�л�����ɾ��
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

    if (fileid < 0 || fileid >= PIC_FILE_NUM || picid < 0 || picid >= pic_file[fileid].num)    // ��������
    {
        return 1;
    }

    if (pic_file[fileid].pcache[picid] == NULL)     //��ǰ��ͼû�м���
    {
        char str[512];
        SDL_RWops* fp_SDL;
        double zoom = (double)pic_file[fileid].percent / 100.0;

        //sprintf(str, "%s/%d.png", pic_file[fileid].path, picid);
        sprintf(str,"%s/%d.%s",pic_file[fileid].path, picid,pic_file[fileid].suffix );

        //����cache����
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

        //ָ������
        if (pic_file[fileid].percent > 0 && pic_file[fileid].percent != 100 && zoom != 0 && zoom != 1)
        {
            newcache->w = (int)(zoom * newcache->w);
            newcache->h = (int)(zoom * newcache->h);
            //tmpsur = newcache->t;
            //newcache->s = zoomSurface(tmpsur, zoom, zoom, SMOOTHING_ON);
            newcache->xoff = (int)(zoom * newcache->xoff);
            newcache->yoff = (int)(zoom * newcache->yoff);
            //SDL_SetColorKey(newcache->s,SDL_SRCCOLORKEY|SDL_RLEACCEL ,ConvertColor(g_MaskColor32));  //͸��ɫ
            //SDL_FreeSurface(tmpsur);
        }
        pic_file[fileid].pcache[picid] = newcache;
    }
    else     //�Ѽ�����ͼ
    {
        newcache = pic_file[fileid].pcache[picid];
    }

    if (newcache->t == NULL)     //��ͼΪ�գ�ֱ���˳�
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
    if((flag & 0x2)==0){        // û��alpla

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
    int r = JY_LoadPNG(fileid, picid, g_ScreenW+1,g_ScreenH+1,1,0,0,0,0,0);   //������ͼ����������λ��

    *w = 0;
    *h = 0;
    *xoff = 0;
    *yoff = 0;

    if (r != 0)
    {
        return 1;
    }

    auto newcache = pic_file[fileid].pcache[picid / 2];

    if (newcache->t)        // ���У���ֱ����ʾ
    {
        *w = newcache->w;
        *h = newcache->h;
        *xoff = newcache->xoff;
        *yoff = newcache->yoff;
    }

    return 0;
}


// �ѱ���blit����������ǰ������
// x,y Ҫ���ص���������Ͻ�����
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

    if((flag & 0x2)==0){        // û��alpla
        RenderToTexture(lps, NULL, g_Texture, &rect);
    }
    else{  // ��alpha

        if( (flag &0x4) || (flag &0x8) || (flag &0x10)){   // �ڰ�
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

    if ((flag & 0x2) == 0)          // û��alpha
    {
        SDL_SetTextureColorMod(lps, 255, 255, 255);
        SDL_SetTextureBlendMode(lps, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(lps, 255);
        RenderToTexture(lps, NULL, g_Texture, &rect, rotate, NULL, reversal);
        //SDL_BlitSurface(lps, NULL, g_Surface, &rect);
    }
    else    // ��alpha
    {
        if ((flag & 0x4) || (flag & 0x8) || (flag & 0x10))     // 4-��, 8-��, 16-��ɫ
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


