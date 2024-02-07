package com.wangwu.jymod;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.Bundle;

import org.libsdl.app.SDLActivity;


public class JYModActivity extends SDLActivity
{
    private native void nativeSetGamePath(String path);
    private String mainLibraries = "main52";

    protected String[] getLibraries() {
        return new String[] {
                "SDL2",
                "SDL2_image",
                "SDL2_ttf",
                "bass",
                "lua52",
                "lua54",
                "zlib",
                mainLibraries
        };
    }
    protected String getMainSharedObject() {
        return "lib" + mainLibraries + ".so";
    }
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        Intent intent = getIntent();
        String gamePath = intent.getStringExtra("path");
        nativeSetGamePath(gamePath);
        String mainLib = intent.getStringExtra("version");
        System.out.println(mainLib);
        if (mainLib != null) {
            switch (mainLib) {
                case "lua5.2":
                    mainLibraries = "main52";
                case "lua5.4":
                    mainLibraries = "main54";
                default:
            }
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
    }


}
