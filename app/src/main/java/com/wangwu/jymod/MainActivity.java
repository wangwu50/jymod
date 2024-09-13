package com.wangwu.jymod;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.BasicFileAttributeView;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.FileTime;
import java.time.Instant;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.TimeUnit;

public class MainActivity extends Activity {
    private static final int REQUEST_CODE = 1024;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        requestPermission();
        AssetHelper.copyModsToJymodIfNotExists(this);
        final String path = Environment.getExternalStorageDirectory().getPath();
        String jyPath = path +"/jymod";
        File jyPathFile = new File(jyPath);
        if (!jyPathFile.exists()){
            jyPathFile.mkdirs();
        }
        File[] files = jyPathFile.listFiles();
        final List<String> list = new ArrayList<String>();
        assert files != null;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
        // ������޸�ʱ������
        Arrays.sort(files, (f1, f2) -> {
            try {
                BasicFileAttributes attr1 = Files.readAttributes(f1.toPath(), BasicFileAttributes.class);
                BasicFileAttributes attr2 = Files.readAttributes(f2.toPath(), BasicFileAttributes.class);
                return attr2.lastAccessTime().compareTo(attr1.lastAccessTime());
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        });
        }
        for(File f : files){
            if(f.canRead() && f.isDirectory()){
                list.add(f.getName());
            }
        }
        ListView listView = findViewById(R.id.list_view);
        ArrayAdapter<String> adapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, list);
        listView.setAdapter(adapter);
        Intent intent = new Intent(this, JYmodActivity.class);


        listView.setOnItemClickListener((parent, view, position, id) -> {
            System.out.println(list.get(position));
            String gamePath=jyPath + "/" + list.get(position)  + "/";
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                FileTime newAccessTime = FileTime.from(Instant.ofEpochSecond(System.currentTimeMillis() - TimeUnit.DAYS.toMillis(1)));
                Path gameFile = Paths.get(gamePath);
                BasicFileAttributeView attributes = Files.getFileAttributeView(gameFile, BasicFileAttributeView.class);
                try {
                    attributes.setTimes(null,newAccessTime,null);
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            }
            intent.putExtra("path", gamePath);
            startActivity(intent);
        });


    }
    private void requestPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager()) {
                Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
                intent.setData(Uri.parse("package:" + getPackageName()));
                startActivityForResult(intent, REQUEST_CODE);
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (!(checkSelfPermission(android.Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED &&
                    checkSelfPermission(android.Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED)) {
                requestPermissions(new String[]{android.Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_CODE);
            }
        }
    }
}