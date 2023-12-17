package com.wangwu.jymod;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.provider.Settings;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class jymodActivity extends SDLActivity
{
    private static final int REQUEST_CODE = 1024;
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
        requestPermission();
        try{
            final String path = Environment.getExternalStorageDirectory().getPath();
            String jyPath = path +"/jymod";
            File jyPathFile = new File(jyPath);
            if (!jyPathFile.exists()){
                jyPathFile.mkdirs();
            }
            File[] files = jyPathFile.listFiles();
            final List<String> list = new ArrayList<String>();
            assert files != null;
            for(File f : files){
                if(f.canRead() && f.isDirectory()){
                    list.add(f.getName());
                }
            }
            AlertDialog.Builder builder1 = new AlertDialog.Builder(this);
            builder1.setTitle("由美好的地狱倾情打造");
            builder1.create().show();
            if(list.size() == 1){
                setPath(jyPath,list.get(0));
            } else if (list.size() > 1) {
                @SuppressLint("HandlerLeak") final Handler mHandler = new Handler() {
                    @Override
                    public void handleMessage(Message msg) {
                        throw new RuntimeException();
                    }
                };
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                builder.setTitle("金庸MOD启动器");
                builder.setItems(list.toArray(new String[list.size()]),new DialogInterface.OnClickListener(){
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        setPath(jyPath,list.get(which));
                        mHandler.sendMessage(Message.obtain());
                    }
                });
                builder.setCancelable(false);
                builder.create().show();

                Looper.loop();
            }
        }catch(RuntimeException e){

        }
    }
    private void setPath(String jyPath,String path){
        String gamePath=jyPath + "/" + path  + "/";
        nativeSetGamePath(gamePath);
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    private void requestPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager()) {
                Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
                intent.setData(Uri.parse("package:" + getPackageName()));
                startActivityForResult(intent, REQUEST_CODE);
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (!(checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED &&
                    checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED)) {
                requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_CODE);
            }
        }
    }
}
