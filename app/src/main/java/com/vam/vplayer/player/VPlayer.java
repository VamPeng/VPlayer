package com.vam.vplayer.player;

import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import org.jetbrains.annotations.Nullable;

public class VPlayer implements SurfaceHolder.Callback {

//    static {
//        System.loadLibrary("vplayer");
//    }

    private SurfaceHolder surfaceHolder;

    private OnErrorListener onErrorListener;
    private OnPrepareListener onPrepareListener;
    private OnProgressListener onProgressListener;

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }

    public void setOnPrepareListener(OnPrepareListener onPrepareListener) {
        this.onPrepareListener = onPrepareListener;
    }

    public void setOnProgressListener(OnProgressListener onProgressListener) {
        this.onProgressListener = onProgressListener;
    }

    public void setSurface(SurfaceView surfaceView) {
        if (null != surfaceHolder) {
            surfaceHolder.removeCallback(this);
        }
        surfaceHolder = surfaceView.getHolder();
        surfaceHolder.addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        native_setSurface(surfaceHolder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    public void prepare() {
        native_prepare(dataSource);
    }

    public void start(){
        Log.i("Vam","开始播放");
        native_start();
    }

    public void onPrepared(){
        if (null!=onPrepareListener){
            onPrepareListener.onPrepared();
        }
    }

    public void onProgress(int progress) {
        if (null != onProgressListener) {
            onProgressListener.onProgress(progress);
        }
    }

    public void onError(int errorCode) {
        if (null != onErrorListener) {
            onErrorListener.onError(errorCode);
        }
    }

    private String dataSource;

    public void settDataSource(@Nullable String absolutePath) {
        this.dataSource = absolutePath;
    }

    public interface OnPrepareListener {
        void onPrepared();
    }

    public interface OnErrorListener {
        void onError(int error);
    }

    public interface OnProgressListener {
        void onProgress(int progress);
    }

    public native void native_prepare(String data_);

    public native void native_start();

    public native void native_setSurface(Surface surface_);
}
