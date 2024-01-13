package com.wangwu.jymod;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.Bundle;

import org.libsdl.app.SDLActivity;


public class JYmodActivity extends SDLActivity
{
    private native void nativeSetGamePath(String path);

    protected String[] getLibraries() {
        return new String[] {
                "SDL2",
                "SDL2_image",
                "SDL2_ttf",
                "bass",
                "lua",
                "zlib",
                "main"
        };
    }
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        Intent intent = getIntent();
        String gamePath = intent.getStringExtra("path");
        nativeSetGamePath(gamePath);
    }

    @Override
    protected void onPause() {
        super.onPause();
    }


}
