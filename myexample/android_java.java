package com.example.testcusejava;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;

import com.example.testcusejava.R;
import com.example.testcusejava.Testcusenative.MyThread2;
import com.example.testcusejava.Testcusenative.MyThread3;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbEndpoint;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;
import android.hardware.usb.UsbRequest;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Toast;

//bhw   2016.07.19 e

public class Testcusenative extends Activity {  
    public native void libusbfunc();// 调用native方法   
    static {
        System.loadLibrary("xxtest");//加载ndk-build生成的libxxtest.so库
    }

    private Button button,button2;
    private UsbManager usbManager;
    private UsbDevice usbDevice;
    private static final String ACTION_USB_PERMISSION = "com.android.example.USB_PERMISSION";
    private PendingIntent pendingIntent;
    static int fd = -1;

    private Handler handler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            // TODO Auto-generated method stub
            super.handleMessage(msg);
            switch (msg.what) {
                case 3:
                Toast.makeText(Testcusenative.this, "33333333333333333", 0).show();
                break;
                case 4:
                Toast.makeText(Testcusenative.this, "44444444444444444", 0).show();
                break;

            }
        }
    };

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_testcusenative);           
        try {
            Process process = Runtime.getRuntime().exec("su");
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }   
        button = (Button) findViewById(R.id.buttonPlay);
        usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);    
        pendingIntent = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);
        IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        registerReceiver(mUsbReceiver, filter);

        button.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                // TODO Auto-generated method stub
                Log.e("device","call MyThread2 ");
                new MyThread2().start();

            //  libusbfunc();// 调用native方法              }
            });


    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.testcusenative, menu);
        return true;
    }


    public static native void test();
    static{
         //System.loadLibrary("lianyinbao-jni");
    }

    class MyThread2 extends Thread{

        @Override
        public void run() {
            // TODO Auto-generated method stub
            super.run();
            try {
                UsbDevice ud= null;
                //链接到设备的USB设备列表
                HashMap<String, UsbDevice> map = usbManager.getDeviceList();

                Collection<UsbDevice> usbDevices = map.values();
                Iterator<UsbDevice> usbDeviceIter = usbDevices.iterator();
                while (usbDeviceIter.hasNext()){
                    ud = usbDeviceIter.next();  
                    //VendorID 和ProductID  十进制                  
                    if(4703 == ud.getVendorId() && 12587 == ud.getProductId()){//vid pid 3141 25409  ; 4703  12587
                     usbDevice = ud;
                   Log.e("device","find device"); //bhw                 
               }
                    //Log.e("device","find device"); //bhw
           }

                //检测USB设备权限受否授权
           if(usbManager.hasPermission(usbDevice)){
            handler.sendEmptyMessage(3);
            new MyThread3().start();
                    Log.e("device","usbManager.hasPermission");//bhw
                }else{
                    //如果没有授权就授予权限
                    handler.sendEmptyMessage(4);
                    usbManager.requestPermission(usbDevice, pendingIntent); //在此Android系统会弹出对话框，选择是否授予权限                    
                    Log.e("device","usbManager.requestPermission");   //bhw                                                                     
                }               
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    class MyThread3 extends Thread{
        @Override
        public void run() {
            // TODO Auto-generated method stub
            super.run();
            Log.e("device","MyThread3");//bhw
            UsbDeviceConnection connection = usbManager.openDevice(usbDevice);          
            fd = connection.getFileDescriptor();//获取文件描述符
            Log.e("device","MyThread3  "+fd);       
            jnicallfunc();//调用native方法                  }
        }
        public static int openUsbDevice(int vid, int pid) throws InterruptedException {

            Log.e("device","openUsbDevice  "+fd);
            return fd;

        }
        private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {

            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                Log.e( "action", action);

                if (ACTION_USB_PERMISSION.equals(action)) {
                    synchronized (this) {
                        usbDevice = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                        if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                            handler.sendEmptyMessage(1)                  
                            if(usbDevice != null){
                          new MyThread3().start();//MyThread3
                      }
                  } 
                  else {
                    Log.d("denied", "permission denied for device " + usbDevice);
                }
            }
        }
    }
};   
}