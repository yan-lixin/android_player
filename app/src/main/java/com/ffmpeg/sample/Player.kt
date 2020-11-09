package com.ffmpeg.sample

import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView

/**
 * @author: lixin
 * Date: 2020/11/3
 * Description: 音视频播放器
 */
class Player : SurfaceHolder.Callback {

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
        // 准备过程错误码
        val ERROR_CODE_FFMPEG_PREPARE = -1000
        // 播放过程错误码
        val ERROR_CODE_FFMPEG_PLAY = -2000
        // 打不开视频
        val FFMPEG_CAN_NOT_OPEN_URL = ERROR_CODE_FFMPEG_PREPARE - 1
        // 找不到媒体流信息
        val FFMPEG_CAN_NOT_FIND_STREAMS = ERROR_CODE_FFMPEG_PREPARE - 2
        // 找不到解码器
        val FFMPEG_FIND_DECODER_FAIL = ERROR_CODE_FFMPEG_PREPARE - 3
        // 无法根据解码器创建上下文
        val FFMPEG_ALLOC_CODEC_CONTEXT_FAIL = ERROR_CODE_FFMPEG_PREPARE - 4
        // 根据流信息 配置上下文参数失败
        val FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL = ERROR_CODE_FFMPEG_PREPARE - 5
        // 打开解码器失败
        val FFMPEG_OPEN_DECODER_FAIL = ERROR_CODE_FFMPEG_PREPARE - 6
        // 没有音视频
        val FFMPEG_NOMEDIA = ERROR_CODE_FFMPEG_PREPARE - 7
        // 读取没提数据包失败
        val FFMPEG_READ_PACKETS_FAIL = ERROR_CODE_FFMPEG_PLAY - 1
    }

    private external fun prepareNative(mediaPath: String?)
    private external fun startNative()
    private external fun stopNative()
    private external fun releaseNative()
    private external fun getDurationNative(): Int
    private external fun seekToNative(playProgress: Int)
    private external fun setSurfaceNative(surface: Surface?)

    // 媒体文件路径
    private var mediaPath: String? = ""

    private var surfaceHolder: SurfaceHolder? = null

    private var onPreparedListener: (() -> Unit)? = null
    private var onErrorListener: ((Int) -> Unit)? = null
    private var onProgressListener: ((Int) -> Unit)? = null

    /**
     * 设置媒体文件路径
     */
    fun setMediaPath(mediaPath: String) {
        this.mediaPath = mediaPath
    }

    /**
     * 播放前准备工作
     */
    fun prepare() {
        prepareNative(mediaPath)
    }

    /**
     * 开始播放
     */
    fun start() {
        startNative()
    }

    fun setOnPrepareListener(onPreparedListener: () -> Unit) {
        this.onPreparedListener = onPreparedListener
    }

    fun setOnErrorListener(onErrorListener: (Int) -> Unit) {
        this.onErrorListener = onErrorListener
    }

    fun setOnProgressListener(onProgressListener: (Int) -> Unit) {
        this.onProgressListener = onProgressListener
    }

    /**
     * 供JNI调用
     * 表示准备完成可以开始播放了
     */
    fun onPrepared() {
        onPreparedListener?.invoke()
    }

    /**
     * 供JNI调用
     * 表示出错了
     */
    fun onError(errorCode: Int) {
        onErrorListener?.invoke(errorCode)
    }

    /**
     * 供JNI调用
     * 表示播放进度调节
     */
    fun onProgress(progress: Int) {
        onProgressListener?.invoke(progress)
    }

    fun setSurfaceView(surfaceView: SurfaceView) {
        surfaceHolder?.removeCallback(this) ?: let {
            surfaceHolder = surfaceView.holder
            surfaceHolder?.addCallback(this)
        }

    }

    /**
     * 画布创建回调
     */
    override fun surfaceCreated(holder: SurfaceHolder?) {

    }

    /**
     * 画布刷新
     */
    override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {
        setSurfaceNative(holder?.surface)
    }

    /**
     * 画布销毁
     */
    override fun surfaceDestroyed(holder: SurfaceHolder?) {

    }

    /**
     * 获取总播放时长
     */
    fun getDuration() = getDurationNative()

    /**
     * 播放进度控制
     */
    fun seekTo(playProgress: Int) {
        Thread {
            seekToNative(playProgress)
        }.start()
    }

    /**
     * 停止播放
     */
    fun stop() {
        stopNative()
    }

    /**
     * 资源释放
     */
    fun release() {
        surfaceHolder?.removeCallback(this)
        releaseNative()
    }

}