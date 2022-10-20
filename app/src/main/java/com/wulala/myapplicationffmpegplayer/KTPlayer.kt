package com.wulala.myapplicationffmpegplayer

import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.lifecycle.LifecycleEventObserver
import android.util.Log
import android.view.Surface
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleOwner

class KTPlayer : SurfaceHolder.Callback, LifecycleEventObserver {
    companion object {
        init {
            System.loadLibrary("marcffmpegplayer")
        }
    }

    private val TAG = "KTPlayer"

    private var nativePlayerObj: Long? = null

    private var surfaceHolder: SurfaceHolder? = null

    fun setSurfaceView(surfaceView: SurfaceView) {
        if (surfaceHolder != null) {
            surfaceHolder?.removeCallback(this)
        }
        this.surfaceHolder = surfaceView.holder
        this.surfaceHolder?.addCallback(this)
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        setSurfaceNative(nativePlayerObj!!, holder.surface)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
    }

    fun start_rev() {

        Log.d(TAG, "start to rev now")
    }

    private external fun startRevNative(): Long
    private external fun setSurfaceNative(nativeObj: Long, surface: Surface)

    override fun onStateChanged(source: LifecycleOwner, event: Lifecycle.Event) {
        when (event) {
            Lifecycle.Event.ON_RESUME -> {
                // 这里相当于安卓主线程来调用jni的准备函数, 千万不可以阻塞
                nativePlayerObj = startRevNative()
            }
            Lifecycle.Event.ON_PAUSE -> {
            }
            Lifecycle.Event.ON_DESTROY -> {
            }
            else -> {
            }
        }
    }
}