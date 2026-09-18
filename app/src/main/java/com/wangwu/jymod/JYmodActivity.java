package com.wangwu.jymod;

import android.content.pm.ActivityInfo;
import android.os.Bundle;
import java.io.File;
import java.io.RandomAccessFile;
import java.nio.channels.FileLock;
import org.libsdl.app.SDLActivity;

public abstract class JYmodActivity extends SDLActivity {
    private native void nativeSetGamePath(String path);
    private native void nativeSetControlType(int type);
    protected abstract String luaVersion();
    private RandomAccessFile sessionFile;
    private FileLock sessionLock;

    @Override protected String[] getLibraries() {
        return new String[]{"SDL2", "SDL2_image", "SDL2_ttf", "bass",
            "lua" + luaVersion(), "zlib", "main" + luaVersion()};
    }
    // SDL invokes this before setting up its surface or starting its game thread.
    @Override public void loadLibraries() {
        String path = getIntent().getStringExtra("path");
        if (path == null || !new File(path).isDirectory())
            throw new IllegalArgumentException("请选择有效的 MOD 目录");
        try {
            sessionFile = new RandomAccessFile(new File(getFilesDir(), "game-session.lock"), "rw");
            sessionLock = sessionFile.getChannel().tryLock();
            if (sessionLock == null) throw new IllegalStateException("已有游戏正在运行，请先退出游戏再切换 Lua 版本");
        } catch (Exception e) {
            releaseSession();
            throw new IllegalStateException("无法启动游戏：" + e.getMessage(), e);
        }
        super.loadLibraries();
        nativeSetGamePath(path);
        nativeSetControlType(getIntent().getIntExtra("control_type", 0));
        android.util.Log.i("JYDualLua", "Starting Lua " + luaVersion() + " MOD=" + path);
    }
    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    }
    private void releaseSession() {
        try { if (sessionLock != null) sessionLock.release(); } catch (Exception ignored) {}
        try { if (sessionFile != null) sessionFile.close(); } catch (Exception ignored) {}
        sessionLock = null; sessionFile = null;
    }
    @Override protected void onDestroy() {
        // Successful SDL shutdown exits this game process, releasing the OS lock.
        try { super.onDestroy(); } finally { releaseSession(); }
    }
}
