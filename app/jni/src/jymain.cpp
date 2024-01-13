
// 主程序
// 本程序为游泳的鱼编写。
// 版权所无，您可以以任何方式使用代码


#include <cstdio>
#include <ctime>
#include <android/log.h>
#include <SDL_rwops.h>

#include "jymain.h"
#include "sdlfun.h"
#include "charset.h"
#include "mainmap.h"
#include "aes.h"
#include <jni.h>
#include <unistd.h>

// 全程变量
SDL_Window* g_Window = NULL;
SDL_Renderer* g_Renderer = NULL;
SDL_Texture* g_Texture = NULL;
SDL_Texture* g_TextureShow = NULL;
SDL_Texture* g_TextureTmp = NULL;

SDL_Surface* g_Surface = NULL;          // 游戏使用的视频表面
Uint32 g_MaskColor32 = 0xff706020;      // 透明色

int g_Rotate = 0;                       //屏幕是否旋转
int g_ScreenW = 480;                    // 屏幕宽高
int g_ScreenH = 320;
int g_ScreenBpp = 16;                   // 屏幕色深
int g_FullScreen = 0;
int g_EnableSound = 1;                  // 声音开关 0 关闭 1 打开
int g_MusicVolume = 32;                 // 音乐声音大小
int g_SoundVolume = 32;                 // 音效声音大小

int g_XScale = 18;                      //贴图x,y方向一半大小
int g_YScale = 9;

int g_XScaleConst = 18;                      //贴图x,y方向一半大小
int g_YScaleConst = 9;


//各个地图绘制时xy方向需要多绘制的余量。保证可以全部显示
int g_MMapAddX;
int g_MMapAddY;
int g_SMapAddX;
int g_SMapAddY;
int g_WMapAddX;
int g_WMapAddY;

int g_MAXCacheNum = 1000;               //最大缓存数量
int g_LoadFullS = 1;                    //是否全部加载S文件
int g_LoadMMapType = 0;                 //是否全部加载M文件
int g_LoadMMapScope = 0;
int g_PreLoadPicGrp = 1;                //是否预先加载贴图文件的grp
int IsDebug = 1;                        //是否打开跟踪文件
char JYMain_Lua[255];                   //lua主函数
int g_MP3 = 0;                          //是否打开MP3
char g_MidSF2[255];                     //音色库对应的文件
float g_Zoom = 1;                       //图片放大


int g_D1X = -1;
int g_D1Y = -1;
int g_D2X = -1;
int g_D2Y = -1;
int g_D3X = -1;
int g_D3Y = -1;
int g_D4X = -1;
int g_D4Y = -1;
int g_C1X = -1;
int g_C1Y = -1;
int g_C2X = -1;
int g_C2Y = -1;
int g_AX = -1;
int g_AY = -1;
int g_BX = -1;
int g_BY = -1;

int g_KeyScale = 100;
int g_Control = 0;

int g_KeyRepeatDelay = 150;		//第一次键盘重复等待ms数
int g_KeyRePeatInterval = 30;	//一秒钟重复次数

int g_RealScreenW = 0;      //实际的窗口宽度
int g_RealScreenH = 0;      //实际的窗口高度
int g_percentValue = 100;      //缩放的比例

char *JY_CurrentPath;

lua_State* pL_main = NULL;
ParticleExample g_Particle;
FILE* debugFP;      //debug.txt的文件指针，就不用每次都打开一遍了



//定义的lua接口函数名
const struct luaL_Reg jylib[] =
{
    {"Debug", HAPI_Debug},
    {"Error", HAPI_Error},

    {"GetKey", HAPI_GetKey},
    {"GetKeyState", HAPI_GetKeyState},
    {"EnableKeyRepeat", HAPI_EnableKeyRepeat},

    {"Delay", HAPI_Delay},
    {"GetTime", HAPI_GetTime},

    {"CharSet", HAPI_CharSet},
    {"DrawStr", HAPI_DrawStr},

    {"SetClip", HAPI_SetClip},
    {"FillColor", HAPI_FillColor},
    {"Background", HAPI_Background},
    {"DrawRect", HAPI_DrawRect},

    {"ShowSurface", HAPI_ShowSurface},
    {"ShowSlow", HAPI_ShowSlow},

    {"PicInit", HAPI_PicInit},
    {"PicGetXY", HAPI_GetPicXY},
    {"PicLoadCache", HAPI_LoadPic},
    {"PicLoadFile", HAPI_PicLoadFile},

    {"FullScreen", HAPI_FullScreen},

    {"LoadPicture", HAPI_LoadPicture},

    {"PlayMIDI", HAPI_PlayMIDI},
    {"PlayWAV", HAPI_PlayWAV},
    {"PlayMPEG", HAPI_PlayMPEG},

    {"LoadMMap", HAPI_LoadMMap},
    {"DrawMMap", HAPI_DrawMMap},
    {"GetMMap", HAPI_GetMMap},
    {"UnloadMMap", HAPI_UnloadMMap},

    {"LoadSMap", HAPI_LoadSMap},
    {"SaveSMap", HAPI_SaveSMap},
    {"GetS", HAPI_GetS},
    {"SetS", HAPI_SetS},
    {"GetD", HAPI_GetD},
    {"SetD", HAPI_SetD},
    {"DrawSMap", HAPI_DrawSMap},

    {"LoadWarMap", HAPI_LoadWarMap},
    {"GetWarMap", HAPI_GetWarMap},
    {"SetWarMap", HAPI_SetWarMap},
    {"CleanWarMap", HAPI_CleanWarMap},

    {"DrawWarMap", HAPI_DrawWarMap},
    {"SaveSur", HAPI_SaveSur},
    {"LoadSur", HAPI_LoadSur},
    {"FreeSur", HAPI_FreeSur},
    {"GetScreenW", HAPI_ScreenWidth},
    {"GetScreenH", HAPI_ScreenHeight},
    {"LoadPNGPath", HAPI_LoadPNGPath},
    {"LoadPNG", HAPI_LoadPNG},
    {"GetPNGXY", HAPI_GetPNGXY},
    {"PlayMovie", HAPI_PlayMovie},
    {"LoadLuaFile", HAPI_LoadLuaFile},
    {"GetFileLength", HAPI_GetFileLength},
    {"RestResolution", HAPI_RestResolution},
    {"GetRealResolution", HAPI_GetRealResolution},
    {"GetScale", HAPI_GetScale},
    {"GetPlatform", HAPI_GetPlatform},
    {NULL, NULL}
};



const struct luaL_Reg bytelib[] =
{
    {"create", Byte_create},
    {"loadfile", Byte_loadfile},
    {"loadfilefromzip", Byte_loadfilefromzip},
    {"savefile", Byte_savefile},
    {"unzip", Byte_unzip},
    {"zip", Byte_zip},
    {"get16", Byte_get16},
    {"set16", Byte_set16},
    {"getu16", Byte_getu16},
    {"setu16", Byte_setu16},
    {"get32", Byte_get32},
    {"set32", Byte_set32},
    {"get64", Byte_get64},
    {"set64", Byte_set64},
    {"getstr", Byte_getstr},
    {"setstr", Byte_setstr},
    {NULL, NULL}
};

static const struct luaL_Reg configLib[] = {

    { "GetPath", Config_GetPath },
    { NULL, NULL }
};

void GetModes(int *width, int *height)
{
    char buf[10];
    FILE *fp = fopen(_("resolution.txt"), "r");

    if (!fp) {
        JY_Error("GetModes: cannot open resolution.txt");
        return;
    }

    //宽
    memset(buf, 0, 10);
    fgets(buf, 10, fp);
    *width = atoi(buf);

    //高
    memset(buf, 0, 10);
    fgets(buf, 10, fp);
    *height = atoi(buf);

    JY_Debug("GetModes: width=%d, height=%d", *width, *height);

    fclose(fp);
}

// 主程序
int main(int argc, char* argv[])
{

    __android_log_print(ANDROID_LOG_VERBOSE, "jymod", "main start");

    //remove(_(DEBUG_FILE));
    //remove(_(ERROR_FILE));    //设置stderr输出到文件

    const char* debugFilename = _(DEBUG_FILE);
    debugFP = fopen(debugFilename, "w");

    if(debugFP == NULL){
        JY_Debug("cannot open %s", debugFilename);
    }
    const char* configFilename;
    if (access(_(CONFIG_FILE), F_OK)==0){
        configFilename= _(CONFIG_FILE);
    } else if  (access(_(CONFIG_FILE2), F_OK)==0){
        configFilename = _(CONFIG_FILE2);
    } else{
        configFilename = CONFIG_FILE2;
    }
    JY_Debug("user config: %s",configFilename);

    pL_main = luaL_newstate();
    luaL_openlibs(pL_main);

    //注册lua函数
    lua_newtable(pL_main);
    luaL_setfuncs(pL_main, jylib, 0);
    lua_pushvalue(pL_main, -1);
    lua_setglobal(pL_main, "lib");

    lua_newtable(pL_main);
    luaL_setfuncs(pL_main, bytelib, 1);
    lua_pushvalue(pL_main, -1);
    lua_setglobal(pL_main, "Byte");

    lua_newtable(pL_main);
    luaL_setfuncs(pL_main, configLib, 0);
    lua_pushvalue(pL_main, -1);
    lua_setglobal(pL_main, "config");

    JY_Debug("Lua_Config();");
    Lua_Config(pL_main, configFilename);        //读取lua配置文件，设置参数


    JY_Debug("InitSDL();");
    InitSDL();           //初始化SDL

    JY_Debug("InitGame();");
    InitGame();          //初始化游戏数据

    JY_Debug("LoadMB();");
    LoadMB(HZMB_FILE);  //加载汉字字符集转换码表

    JY_Debug("Lua_Main();");
    Lua_Main(pL_main);          //调用Lua主函数，开始游戏

    JY_Debug("ExitGame();");
    ExitGame();       //释放游戏数据

    JY_Debug("ExitSDL();");
    ExitSDL();        //退出SDL

    JY_Debug("main() end;");

    //关闭lua
    lua_close(pL_main);

    fclose(debugFP);

    return 0;
}


//Lua主函数
int Lua_Main(lua_State* pL_main)
{
    int result = 0;

    //初始化lua

    //加载lua文件
    if(strlen(JYMain_Lua) != 0) {

        result = luaL_loadfile(pL_main, JYMain_Lua);
        switch (result) {
            case LUA_ERRSYNTAX:
                JY_Error("load lua file %s error: syntax error!\n", JYMain_Lua);
                break;
            case LUA_ERRMEM:
                JY_Error("load lua file %s error: memory allocation error!\n", JYMain_Lua);
                break;
            case LUA_ERRFILE:
                JY_Error("load lua file %s error: can not open file!\n", JYMain_Lua);
                break;
        }

        result = lua_pcall(pL_main, 0, LUA_MULTRET, 0);
    }

    //调用lua的主函数JY_Main
    lua_getglobal(pL_main, "JY_Main");
    result = lua_pcall(pL_main, 0, 0, 0);

    return 0;
}


//Lua读取配置信息
int Lua_Config(lua_State* pL_main,const char* filename)
{
    //加载并执行LUA
    LoadLuaFile(filename, 2);


    lua_getglobal(pL_main, "CONFIG");            //读取config定义的值
//    if (getfield(pL_main, "Width") != 0)
//    {
//        g_ScreenW = getfield(pL_main, "Width");
//    }
//    if (getfield(pL_main, "Height") != 0)
//    {
//        g_ScreenH = getfield(pL_main, "Height");
//    }
    g_ScreenBpp = getfield(pL_main, "bpp");
    g_FullScreen = getfield(pL_main, "FullScreen");
    g_XScale = getfield(pL_main, "XScale");
    g_YScale = getfield(pL_main, "YScale");

    g_XScaleConst = g_XScale;
    g_YScaleConst = g_YScale;

    g_EnableSound = getfield(pL_main, "EnableSound");
    IsDebug = getfield(pL_main, "Debug");
    g_MMapAddX = getfield(pL_main, "MMapAddX");
    g_MMapAddY = getfield(pL_main, "MMapAddY");
    g_SMapAddX = getfield(pL_main, "SMapAddX");
    g_SMapAddY = getfield(pL_main, "SMapAddY");
    g_WMapAddX = getfield(pL_main, "WMapAddX");
    g_WMapAddY = getfield(pL_main, "WMapAddY");
    g_SoundVolume = getfield(pL_main, "SoundVolume");
    g_MusicVolume = getfield(pL_main, "MusicVolume");

    g_MAXCacheNum = getfield(pL_main, "MAXCacheNum");
    g_LoadFullS = getfield(pL_main, "LoadFullS");
    g_MP3 = getfield(pL_main, "MP3");

    g_Zoom = (float)(getfield(pL_main, "Zoom") / 100.0);

    getfieldstr(pL_main, "MidSF2", g_MidSF2);

    getfieldstr(pL_main, "JYMain_Lua", JYMain_Lua);

    g_D1X =getfield(pL_main, "D1X");
    g_D1Y =getfield(pL_main, "D1Y");
    g_D2X =getfield(pL_main, "D2X");
    g_D2Y =getfield(pL_main, "D2Y");
    g_D3X =getfield(pL_main, "D3X");
    g_D3Y =getfield(pL_main, "D3Y");
    g_D4X =getfield(pL_main, "D4X");
    g_D4Y =getfield(pL_main, "D4Y");
    g_C1X =getfield(pL_main, "C1X");
    g_C1Y =getfield(pL_main, "C1Y");
    g_C2X =getfield(pL_main, "C2X");
    g_C2Y =getfield(pL_main, "C2Y");
    g_AX =getfield(pL_main, "AX");
    g_AY =getfield(pL_main, "AY");
    g_BX =getfield(pL_main, "BX");
    g_BY =getfield(pL_main, "BY");

    g_KeyScale = getfield(pL_main, "KeyScale");
    if (g_KeyScale > 1000){
        g_KeyScale = 100;
    }

    g_KeyRepeatDelay =getfield(pL_main, "KeyRepeatDelay");
    g_KeyRePeatInterval =getfield(pL_main, "KeyRePeatInterval");
    g_Control = getfield(pL_main, "ControlType");
    return 0;
}

//读取lua表中的整型
int getfield(lua_State* pL, const char* key)
{
    int result;
    lua_getfield(pL, -1, key);
    result = (int)lua_tonumber(pL, -1);
    lua_pop(pL, 1);
    return result;
}

//读取lua表中的字符串
int getfieldstr(lua_State* pL, const char* key, char* str)
{
    const char* tmp;
    lua_getfield(pL, -1, key);
    tmp = (const char*)lua_tostring(pL, -1);
    if (tmp) { strcpy(str, tmp); }
    lua_pop(pL, 1);
    return 0;
}

//以下为几个通用函数

// 调试函数
// 输出到debug.txt中
int JY_Debug(const char* fmt, ...)
{
    time_t t;
    FILE* fp;
    struct tm* newtime;
    va_list argptr;
#ifndef _DEBUG
    if (IsDebug == 0)
    {
        return 0;
    }
#endif
    char string[1024];
    // concatenate all the arguments in one string
    va_start(argptr, fmt);
    vsnprintf(string, sizeof(string), fmt, argptr);
    va_end(argptr);
    time(&t);
    newtime = localtime(&t);
#ifdef _DEBUG
    //fprintf(stderr, "%02d:%02d:%02d %s\n", newtime->tm_hour, newtime->tm_min, newtime->tm_sec, string);
#else
    __android_log_print(ANDROID_LOG_VERBOSE, "jymod", "%02d:%02d:%02d %s", newtime->tm_hour, newtime->tm_min, newtime->tm_sec, string);

    if(debugFP != NULL){
        fprintf(debugFP, "%02d:%02d:%02d %s\n", newtime->tm_hour, newtime->tm_min, newtime->tm_sec, string);
        fflush(debugFP);
    }

#endif
    return 0;
}

// 调试函数
// 输出到error.txt中
int JY_Error(const char* fmt, ...)
{
    //无酒不欢：不再输出error信息
#ifdef _DEBUG
    /*
    time_t t;
    FILE* fp;
    struct tm* newtime;

    va_list argptr;
    char string[1024];

    va_start(argptr, fmt);
    vsnprintf(string, sizeof(string), fmt, argptr);
    va_end(argptr);
    fp=fopen(_(ERROR_FILE),"a+t");
    time(&t);
    newtime = localtime(&t);
     __android_log_print(ANDROID_LOG_VERBOSE, "BlackHil", "%02d:%02d:%02d %s", newtime->tm_hour, newtime->tm_min, newtime->tm_sec, string);
    fprintf(stderr, "%02d:%02d:%02d %s\n", newtime->tm_hour, newtime->tm_min, newtime->tm_sec, string);
    fflush(fp);
    fclose(fp);
    */
#endif
    time_t t;
    FILE* fp;
    struct tm* newtime;
    va_list argptr;
    char string[1024];
    va_start(argptr, fmt);
    vsnprintf(string, sizeof(string), fmt, argptr);
    va_end(argptr);
    time(&t);
    newtime = localtime(&t);
    __android_log_print(ANDROID_LOG_VERBOSE, "jymod", "%02d:%02d:%02d %s\n %s", newtime->tm_hour, newtime->tm_min, newtime->tm_sec, string, lua_tostring(pL_main, -1));

    if(debugFP != NULL){
        fprintf(debugFP, "%02d:%02d:%02d %s\n%s\n", newtime->tm_hour, newtime->tm_min, newtime->tm_sec, string, lua_tostring(pL_main, -1));
        fflush(debugFP);
    }
    return 0;
}

// 限制x大小
int limitX(int x, int xmin, int xmax)
{
    if (x > xmax)
    {
        x = xmax;
    }
    if (x < xmin)
    {
        x = xmin;
    }
    return x;
}

// 返回文件长度，若为0，则文件可能不存在
long FileLength(const char* filename)
{
    /*
    FILE*   f;
    int ll;
    if ((f = fopen(filename, "rb")) == NULL)
    {
        return 0;            // 文件不存在，返回
    }
    fseek(f, 0, SEEK_END);
    ll = ftell(f);    //这里得到的len就是文件的长度了
    fclose(f);
    return ll;
     */
    long ll;
    SDL_RWops* rwops = SDL_RWFromFile(filename, "rb");
    if (rwops == NULL) {
        JY_Debug("cannot load file %s !", filename);
        return 0;
    }
    ll = rwops->size(rwops);
    rwops->close(rwops);

    return ll;
}

char* va(const char* format, ...)
{
    static char string[256];
    va_list     argptr;

    va_start(argptr, format);
    vsnprintf(string, 256, format, argptr);
    va_end(argptr);

    return string;
}


int LoadLuaFile(const char* filename, int sign, int doFlag)
{

    //解密的KEY
    static uint8_t key[] = { 0x3b, 0x6e, 0x17, 0x16, 0x24, 0xae, 0xd2, 0x26, 0xab, 0xf7, 0x25, 0x28, 0x03, 0xca, 0x4a, 0xac };

    int result = 0;
    int len = 0;
    SDL_RWops* rwops = SDL_RWFromFile(filename, "rb");
    if (rwops == NULL) {
        JY_Debug("cannot load file %s !", filename);
        return 1;
    }

    len = rwops->size(rwops);
    char* data = (char*)malloc(len+1);
    memset(data, 0, len+1);
    if (data == NULL)
    {
        JY_Debug("Lua_Config: cannot malloc config memory!\n");
        return 1;
    }

    rwops->read(rwops, data, 1, len);
    rwops->close(rwops);

    //如果有加密
    if(sign == 1)
    {
        char* desData = (char*)malloc(len);
        int i=0;
        char buf[64];
        int minLen = 64;
        while(i < len)
        {
            if(i + 64 > len)
            {
                minLen = len - i;
            }
            memset(buf, 0, 64);
            memcpy(buf, data+i, minLen);
            AES_ECB_decrypt((uint8_t*)buf, key, (uint8_t*)(desData+i), minLen);

            i += 64;
        }

        SafeFree(data);
        data = desData;
    }
    result = luaL_loadstring(pL_main, data);

    //有些时候luaL_loadstring会加载不了， 但是生成lua文件之后又可以加载
    if(result == LUA_ERRSYNTAX)
    {
        JY_Debug("LUA_ERRSYNTAX");
        const char* luafile = _("gamelua.lua");
        FILE* file = fopen(luafile, "wb");
        fwrite(data, 1, strlen(data), file);
        fclose(file);


        result = luaL_loadfile(pL_main, luafile);
        //remove(_("gamelua.lua"));
    }

    switch (result)
    {
        case LUA_ERRSYNTAX:
            JY_Debug("load lua file %s error: syntax error!\n %s\n", filename, lua_tostring(pL_main, -1));

            break;
        case LUA_ERRMEM:
            JY_Debug("load lua file %s error: memory allocation error!\n", filename);
            break;
        case LUA_ERRFILE:
            JY_Debug( "load lua file %s error: can not open file!\n", filename);
            break;
    }

    if(doFlag == 1)
    {
        result = lua_pcall(pL_main, 0, LUA_MULTRET, 0);
    }

    return result;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_wangwu_jymod_JYmodActivity_nativeSetGamePath(JNIEnv *env, jobject thiz, jstring path){
    const char *native_path =env->GetStringUTFChars(path,NULL);
    if (native_path == NULL) {
        return;
    }

    JY_CurrentPath = (char*) malloc(strlen(native_path) + 1);
    strcpy(JY_CurrentPath, native_path);
    __android_log_print(ANDROID_LOG_INFO, "jy", "set path = %s", JY_CurrentPath);
    (*env).ReleaseStringUTFChars(path, native_path);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_wangwu_jymod52_JYmodActivity_nativeSetGamePath(JNIEnv *env, jobject thiz, jstring path){
    const char *native_path =env->GetStringUTFChars(path,NULL);
    if (native_path == NULL) {
        return;
    }

    JY_CurrentPath = (char*) malloc(strlen(native_path) + 1);
    strcpy(JY_CurrentPath, native_path);
    __android_log_print(ANDROID_LOG_INFO, "jy", "set path = %s", JY_CurrentPath);
    (*env).ReleaseStringUTFChars(path, native_path);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_wangwu_jymod54_JYmodActivity_nativeSetGamePath(JNIEnv *env, jobject thiz, jstring path){
    const char *native_path =env->GetStringUTFChars(path,NULL);
    if (native_path == NULL) {
        return;
    }

    JY_CurrentPath = (char*) malloc(strlen(native_path) + 1);
    strcpy(JY_CurrentPath, native_path);
    __android_log_print(ANDROID_LOG_INFO, "jy", "set path = %s", JY_CurrentPath);
    (*env).ReleaseStringUTFChars(path, native_path);
}