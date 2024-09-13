package com.wangwu.jymod;

import android.app.AlarmManager;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
public class AssetHelper {
    public static void copyModsToJymodIfNotExists(Context context) {
        ProgressDialog progressDialog = new ProgressDialog(context);
        progressDialog.setMessage("正在复制文件，请稍候...");
        progressDialog.setCancelable(false);
        progressDialog.show();
        ExecutorService executor = Executors.newSingleThreadExecutor();
        Handler handler = new Handler(Looper.getMainLooper());
        executor.execute(() -> {
            AssetManager assetManager = context.getAssets();
            String jymodPath = Environment.getExternalStorageDirectory() + File.separator + "jymod";
            String modsPath = "mods";
            boolean filesCopied = false;
            try {
                // 获取 assets/mods 文件夹中的所有文件夹
                String[] assetFolders = assetManager.list(modsPath);
                if (assetFolders != null) {
                    for (String folder : assetFolders) {
                        String folderPath = jymodPath + File.separator + folder;
                        File targetDir = new File(folderPath);
                        // 如果 jymod 文件夹中不存在该文件夹，则复制
                        if (!targetDir.exists() || !targetDir.isDirectory()) {
                            copyFolder(assetManager, modsPath + File.separator + folder, folderPath);
                            filesCopied = true;
                        }
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
            boolean finalFilesCopied = filesCopied;
            handler.post(() -> {
                if (progressDialog.isShowing()) {
                    progressDialog.dismiss();
                }
                if (finalFilesCopied) {
                    showRestartDialog(context);
                }
            });
        });
    }
    private static void copyFolder(AssetManager assetManager, String srcFolder, String destFolder) throws IOException {
        String[] files = assetManager.list(srcFolder);
        if (files == null || files.length == 0) {
            return;
        }
        File destDir = new File(destFolder);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
        for (String file : files) {
            String srcPath = srcFolder + File.separator + file;
            String destPath = destFolder + File.separator + file;
            if (assetManager.list(srcPath).length == 0) {
                copyFile(assetManager, srcPath, destPath);
            } else {
                copyFolder(assetManager, srcPath, destPath);
            }
        }
    }
    private static void copyFile(AssetManager assetManager, String srcPath, String destPath) throws IOException {
        InputStream in = assetManager.open(srcPath);
        OutputStream out = new FileOutputStream(destPath);
        byte[] buffer = new byte[1024];
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }
        in.close();
        out.close();
    }
    private static void showRestartDialog(Context context) {
        new AlertDialog.Builder(context)
                .setTitle("重启应用")
                .setMessage("文件复制完成，需要重启应用以生效。请点击重启")
                .setCancelable(false)
                .setPositiveButton("重启", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        restartApplication(context);
                    }
                })
                .show();
    }
    private static void restartApplication(Context context) {
        Intent intent = new Intent(context, MainActivity.class); // 替换为你的启动 Activity
        int pendingIntentId = 123456;
        PendingIntent pendingIntent = PendingIntent.getActivity(context, pendingIntentId, intent, PendingIntent.FLAG_CANCEL_CURRENT | PendingIntent.FLAG_IMMUTABLE);
        AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        alarmManager.set(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + 1000, pendingIntent); // 1秒后重启应用
        System.exit(0);
    }
}