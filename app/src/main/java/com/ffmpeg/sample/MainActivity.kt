package com.ffmpeg.sample

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.View
import android.view.WindowManager
import android.widget.SeekBar
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {
    private val TAG = MainActivity::class.java.simpleName

    private val player: Player by lazy { Player() }

    private var isTouch = false
    private var isSeek = false

    private var filePath: String = "${Environment.getExternalStorageDirectory().absolutePath}/DCIM/Camera/test.mp4"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setContentView(R.layout.activity_main)

        player.apply {
            setSurfaceView(surfaceView)
            setMediaPath(filePath)
        }

        initListener()
    }

    private fun initListener() {
        player.apply {
            setOnPrepareListener {
                val duration = getDuration()
                // 如果是直播 duration为0
                // 不为0，显示播放控制进度条
                if (duration != 0) {
                    runOnUiThread {
                        seekBar.visibility = View.VISIBLE
                        Log.e(TAG, "开始播放")
                    }
                }
                start()
            }

            setOnErrorListener {  errorCode ->
                handleError(errorCode)
            }

            setOnProgressListener {  progress ->
                Log.e(TAG, "progress: $progress")
                // 非人为干预进度条，让进度条自然正常播放
                if (!isTouch) {
                    runOnUiThread {
                        val duration = getDuration()
                        Log.e(TAG, "duration: $duration")
                        if(duration != 0) {
                            if (isSeek) {
                                isSeek = false
                                return@runOnUiThread
                            }
                            seekBar.progress = progress * 100 / duration
                        }
                    }
                }
            }

        }
        seekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener{
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {

            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
                isTouch = true
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                isSeek = true
                isTouch = false
                // 获取SeekBar的当前进度（百分比）
                val seekBarProgress = seekBar?.progress ?: 0
                // 将SeekBar的进度转换成真实的播放速度
                val duration = player.getDuration()
                val playProgress = seekBarProgress * duration / 100

                // 手动拖动进度条，要能跳到指定的播放速度 av_seek_frame
                player.seekTo(playProgress)
            }
        })
    }

    private fun handleError(errorCode: Int) {
        Log.e(TAG, "出错了，错误码：$errorCode")
    }

    override fun onResume() {
        super.onResume()
        player.prepare()
    }

    override fun onStop() {
        super.onStop()
        player.stop()
    }

    override fun onDestroy() {
        super.onDestroy()
        player.release()
    }


}
