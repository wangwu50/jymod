package com.wangwu.jymod;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.pm.ActivityInfo;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.widget.ArrayAdapter;
import android.widget.AdapterView;
import android.view.View;
import android.widget.ListView;
import android.widget.Spinner;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;

public class MainActivity extends Activity {
    private Spinner controls;
    private SharedPreferences prefs;
    private boolean assetsStarted;
    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        setContentView(R.layout.activity_main);
        prefs = getSharedPreferences("JYModPrefs", MODE_PRIVATE);
        controls = findViewById(R.id.control_type_spinner);
        ArrayAdapter<String> adapter = new ArrayAdapter<>(this,
            android.R.layout.simple_spinner_item, new String[]{"有按键", "无按键"});
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        controls.setAdapter(adapter);
        controls.setSelection(prefs.getInt("control_type", 0) == 1 ? 1 : 0);
        controls.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                prefs.edit().putInt("control_type", position).apply();
                if (position == 1 && !prefs.getBoolean("dont_show_instruction", false)) {
                    showControlInstructionDialog();
                }
            }
            @Override public void onNothingSelected(AdapterView<?> parent) {}
        });
        if (!hasStorageAccess()) {
            if (Build.VERSION.SDK_INT >= 30) {
                Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION,
                    Uri.parse("package:" + getPackageName()));
                startActivityForResult(intent, 1024);
            } else if (Build.VERSION.SDK_INT >= 23) {
                requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1024);
            }
        }
    }
    private void showControlInstructionDialog() {
        new AlertDialog.Builder(this)
            .setTitle("操作说明")
            .setView(getLayoutInflater().inflate(R.layout.control_instruction_dialog, null))
            .setPositiveButton("确定", null)
            .setNeutralButton("不再提示", (dialog, which) ->
                prefs.edit().putBoolean("dont_show_instruction", true).apply())
            .show();
    }
    private boolean hasStorageAccess() {
        if (Build.VERSION.SDK_INT >= 30) return Environment.isExternalStorageManager();
        return Build.VERSION.SDK_INT < 23 ||
            checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
    }
    @Override protected void onResume() {
        super.onResume();
        if (hasStorageAccess() && !assetsStarted) {
            assetsStarted = true;
            AssetHelper.copyModsToJymodIfNotExists(this);
        }
        refreshMods();
    }
    @Override public void onRequestPermissionsResult(int request, String[] permissions, int[] results) {
        super.onRequestPermissionsResult(request, permissions, results);
        refreshMods();
    }
    private void refreshMods() {
        File root = new File(Environment.getExternalStorageDirectory(), "jymod");
        if (hasStorageAccess()) root.mkdirs();
        File[] files = root.listFiles(File::isDirectory);
        if (files == null) files = new File[0];
        Arrays.sort(files, (a,b) -> {
            int recent = Long.compare(lastPlayedOrder(b), lastPlayedOrder(a));
            return recent != 0 ? recent : a.getName().compareToIgnoreCase(b.getName());
        });
        ArrayList<File> mods = new ArrayList<>();
        ArrayList<String> labels = new ArrayList<>();
        for (File f : files) if (f.canRead()) { mods.add(f); labels.add(f.getName()); }
        ListView list = findViewById(R.id.list_view);
        list.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, labels));
        list.setOnItemClickListener((parent,view,position,id) -> {
            File mod=mods.get(position);
            launchMod(mod, preferredVersion(mod));
        });
        list.setOnItemLongClickListener((parent,view,position,id) -> {
            chooseVersion(mods.get(position)); return true;
        });
    }
    private long lastPlayedOrder(File mod) {
        return prefs.getLong("last_played:" + mod.getAbsolutePath(), 0L);
    }
    private void recordPlayed(File mod) {
        // A persistent sequence keeps ordering stable even if the device clock changes.
        long order = prefs.getLong("play_sequence", 0L) + 1L;
        prefs.edit().putLong("play_sequence", order)
            .putLong("last_played:" + mod.getAbsolutePath(), order).apply();
    }
    private void chooseVersion(File mod) {
        final String path;
        try { path = mod.getCanonicalPath() + "/"; }
        catch (IOException e) { new AlertDialog.Builder(this).setMessage(e.getMessage()).setPositiveButton("确定",null).show(); return; }
        String key = "lua:" + path;
        int[] selected = {"54".equals(preferredVersion(mod)) ? 1 : 0};
        new AlertDialog.Builder(this).setTitle(mod.getName() + " · 选择 Lua 版本")
            .setSingleChoiceItems(new String[]{"Lua 5.2", "Lua 5.4"}, selected[0], (dialog,which) -> selected[0]=which)
            .setNegativeButton("取消",null)
            .setPositiveButton("开始游戏", (dialog,which) -> {
                String version = selected[0]==0 ? "52" : "54";
                prefs.edit().putString(key,version).apply();
                launchMod(mod,version);
            }).show();
    }
    private String preferredVersion(File mod) {
        String path;
        try { path=mod.getCanonicalPath()+"/"; }
        catch (IOException e) { path=mod.getAbsolutePath()+"/"; }
        String metadata="";
        try (java.io.FileInputStream in=new java.io.FileInputStream(new File(mod,"info.txt"))) {
            byte[] bytes=new byte[8192]; int n=in.read(bytes);
            if(n>0) metadata=new String(bytes,0,n,java.nio.charset.StandardCharsets.ISO_8859_1);
        } catch(IOException ignored) {}
        return LuaRuntimeChoice.resolve(prefs.getString("lua:"+path,null),metadata,mod.getName());
    }
    private void launchMod(File mod,String version) {
        try {
            String path=mod.getCanonicalPath()+"/";
            int control=controls.getSelectedItemPosition();
            prefs.edit().putInt("control_type",control).apply();
            Intent intent=new Intent(this,"54".equals(version)?Lua54Activity.class:Lua52Activity.class);
            intent.putExtra("path",path).putExtra("control_type",control);
            startActivity(intent);
            recordPlayed(mod);
        } catch(IOException e) {
            new AlertDialog.Builder(this).setMessage(e.getMessage()).setPositiveButton("确定",null).show();
        }
    }

}
