package com.wangwu.jymod;
import android.app.Instrumentation;
import android.app.Activity;
import android.os.Bundle;
import android.content.Context;
import android.content.Intent;
import java.io.*;
import java.nio.channels.FileLock;

public class RuntimeSmokeInstrumentation extends Instrumentation {
    @Override public void onCreate(Bundle args) { super.onCreate(args); start(); }
    @Override public void onStart() {
        Bundle result=new Bundle();
        try {
            Context context=getTargetContext();
            File root=new File(context.getFilesDir(),"RuntimeCheck");root.mkdirs();
            try(InputStream in=context.getAssets().open("runtimecheck/config.lua");
                OutputStream out=new FileOutputStream(new File(root,"config.lua"))) {
                byte[] b=new byte[8192];int n;while((n=in.read(b))!=-1)out.write(b,0,n);
            }
            File marker=new File(root,"runtime-result.txt");
            if(marker.exists() && !marker.delete())throw new IOException("Cannot reset test marker");
            int expected=0;
            for(String version:new String[]{"52","54","52"}) {
                context.startActivity(new Intent(context,version.equals("52")?Lua52Activity.class:Lua54Activity.class)
                    .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK).putExtra("path",root.getAbsolutePath()+"/").putExtra("control_type",1));
                expected++;
                long deadline=System.currentTimeMillis()+30000;
                boolean complete=false;
                while(System.currentTimeMillis()<deadline) {
                    if(marker.exists()) {
                        java.util.List<String> lines=new java.util.ArrayList<>();
                        try(BufferedReader reader=new BufferedReader(new FileReader(marker))) {
                            String line;while((line=reader.readLine())!=null)lines.add(line);
                        }
                        if(lines.size()==expected && lines.get(expected-1).equals("Lua 5."+version.charAt(1)+" OK")) {
                            try(RandomAccessFile f=new RandomAccessFile(new File(context.getFilesDir(),"game-session.lock"),"rw")) {
                                FileLock lock=f.getChannel().tryLock();
                                if(lock!=null){lock.release();complete=true;break;}
                            }
                        }
                    }
                    Thread.sleep(100);
                }
                if(!complete)throw new AssertionError("Runtime/exit failed for "+version);
            }
            result.putString("stream","PASS real SDL/JNI Lua 5.2 -> 5.4 -> 5.2, language features, clean exit and lock release\n");
            finish(Activity.RESULT_OK,result);
        } catch(Throwable e) {
            result.putString("stream",e.toString());finish(Activity.RESULT_CANCELED,result);
        }
    }
}
