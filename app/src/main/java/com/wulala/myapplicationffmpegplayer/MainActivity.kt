package com.wulala.myapplicationffmpegplayer

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.view.SurfaceView
import android.view.View
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.wulala.myapplicationffmpegplayer.databinding.ActivityMainBinding


class MainActivity : AppCompatActivity(), View.OnClickListener {

    private lateinit var binding: ActivityMainBinding

    val TAG = "MainActivity";
    private var mSurfaceView: SurfaceView? = null
    private var btn: Button? = null

    private var ktplayer: KTPlayer? = null


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        supportActionBar?.hide()

        mSurfaceView = binding.videoSurfaceView

        ktplayer = KTPlayer()

        lifecycle.addObserver(ktplayer!!)
        ktplayer?.setSurfaceView(mSurfaceView!!)

        btn = binding.playBtn;
        btn!!.setOnClickListener(this)

        checkPermission()
    }

    override fun onClick(v: View?) {
        when (v?.id) {
            R.id.playBtn -> {
                Log.d(TAG, "onClick: ")
                ktplayer?.start_rev()
            }
        }
    }

    private var permissions = arrayOf<String>(Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.INTERNET) // 如果要申请多个动态权限，此处可以写多个

    var mPermissionList: MutableList<String> = ArrayList()
    private val PERMISSION_REQUEST = 1

    // 检查权限
    private fun checkPermission() {
        mPermissionList.clear()

        // 判断哪些权限未授予
        for (permission in permissions) {
            if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                mPermissionList.add(permission)
            }
        }

        // 判断是否为空
        if (mPermissionList.isEmpty()) { // 未授予的权限为空，表示都授予了
        } else {
            //请求权限方法
            val permissions = mPermissionList.toTypedArray() //将List转为数组
            ActivityCompat.requestPermissions(this, permissions, PERMISSION_REQUEST)
        }
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<String?>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when (requestCode) {
            PERMISSION_REQUEST -> {}
            else -> super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        }
    }

}
