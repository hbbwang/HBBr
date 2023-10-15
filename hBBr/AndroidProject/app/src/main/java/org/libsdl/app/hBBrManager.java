package org.libsdl.app;

import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.nfc.Tag;
import android.os.Build;
import android.os.Environment;
import android.os.LocaleList;
import android.util.Log;
import android.view.WindowManager;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileInputStream;
import java.io.File;
import java.util.Locale;
import java.nio.channels.FileChannel;

public class hBBrManager {
    public static String _exAppStoragePath;
    public static String _storageRootPath;
    public static String _obbPath;
    public static String _assetsPath;
    public static String _dataPath;

    public static File file = null;
    public static File externalStorageRoot = null;

    public static String TAG = "hBBrManager";
    public hBBrManager()
    {

    }
    public hBBrManager(SDLActivity a)
    {
        //a.getWindow().addFlags(WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED);

//        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
//            // 先判断有没有权限
//            if (!Environment.isExternalStorageManager()) {
//                Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
//                intent.setData(Uri.parse("package:" + getPackageName()));
//                startActivityForResult(intent, 100);
//            }
//        }
        Log.v("hBBrManager","hBBrManager: Init Paths...");
        file = a.getExternalFilesDir(null);
        externalStorageRoot = null;
        if (file != null) {
            File temp = file;
            // 遍历目录结构以找到外部存储的根目录
            for (int i = 0 ; i < 4 ; i++) {
                temp = temp.getParentFile();
            }
            externalStorageRoot = temp;
        }
        _storageRootPath = externalStorageRoot.getAbsolutePath() + "/";
        _exAppStoragePath = file.getAbsolutePath()+ "/";
        _obbPath = a.getObbDir().getAbsolutePath();
        //把Resources和Config资产都复制到 android/data/[package]/files/里
        {
            copyFiles(a , "Config", _exAppStoragePath + "Config");
            copyFiles(a , "Resource", _exAppStoragePath+ "Resource");
        }
    }

    public static boolean copyFiles(Context context, String inPath, String outPath) {
        Log.i(TAG, "copyFiles() inPath:" + inPath + ", outPath:" + outPath);
        String[] fileNames = null;
        try {// 获得Assets一共有几多文件
            fileNames = context.getAssets().list(inPath);
        } catch (IOException e1) {
            e1.printStackTrace();
            return false;
        }
        if (fileNames.length > 0) {//如果是目录
            File fileOutDir = new File(outPath);
            if(fileOutDir.isFile()){
                boolean ret = fileOutDir.delete();
                if(!ret){
                    Log.e(TAG, "delete() FAIL:" + fileOutDir.getAbsolutePath());
                }
            }
            if (!fileOutDir.exists()) { // 如果文件路径不存在
                if (!fileOutDir.mkdirs()) { // 创建文件夹
                    Log.e(TAG, "mkdirs() FAIL:" + fileOutDir.getAbsolutePath());
                    return false;
                }
            }
            for (String fileName : fileNames) { //递归调用复制文件夹
                String inDir = inPath;
                String outDir = outPath + File.separator;
                if(!inPath.equals("")) { //空目录特殊处理下
                    inDir = inDir + File.separator;
                }
                copyFiles(context,inDir + fileName, outDir + fileName);
            }
            return true;
        } else {//如果是文件
            try {
                File fileOut = new File(outPath);
                if(fileOut.exists()) {
                    boolean ret = fileOut.delete();
                    if(!ret){
                        Log.e(TAG, "delete() FAIL:" + fileOut.getAbsolutePath());
                    }
                }
                boolean ret = fileOut.createNewFile();
                if(!ret){
                    Log.e(TAG, "createNewFile() FAIL:" + fileOut.getAbsolutePath());
                }
                FileOutputStream fos = new FileOutputStream(fileOut);
                InputStream is = context.getAssets().open(inPath);
                byte[] buffer = new byte[1024];
                int byteCount=0;
                while((byteCount = is.read(buffer)) > 0) {
                    fos.write(buffer, 0, byteCount);
                }
                fos.flush();//刷新缓冲区
                is.close();
                fos.close();
                return true;
            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        }
    }

    public static String getExternalAppPath() {
        return _exAppStoragePath;
    }
    public static String getExternalStoragePath() {
        return _storageRootPath;
    }
}
