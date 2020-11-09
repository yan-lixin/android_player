[TOC]

# FFmpeg编码流程

![](./screenshot/ffmpeg解码流程.jpg)

# OpenSL ES

## 概述

OpenSL ES 是无授权费、跨平台、针对嵌入式系统精心优化的硬件音频加速API。该库都允许使用C或C ++来实现高性能，低延迟的音频操作。
Android的OpenSL ES库同样位于NDK的platforms文件夹内。关于OpenSL ES的使用可以进入ndk-sample查看[native-audio工程][1]:

## 开发流程七步曲

  1. 创建引擎并获取引擎接口
  2. 设置混音器
  3. 创建播放器
  4. 设置播放回调函数
  5. 设置播放器状态为播放状态
  6. 手动激活回调函数
  7. 释放

### 1、创建引擎并获取引擎接口

```C++
SLresult result;
// 1.1 创建引擎对象：SLObjectItf engineObject
result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
if (SL_RESULT_SUCCESS != result) {
    return;
}
// 1.2 初始化引擎
result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
if (SL_RESULT_SUCCESS != result) {
    return;
}
// 1.3 获取引擎接口 SLEngineItf engineInterface
result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
if (SL_RESULT_SUCCESS != result) {
    return;
}
```

### 2、设置混音器

```C++
// 2.1 创建混音器：SLObjectItf outputMixObject
result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0,
                                             0, 0);
if (SL_RESULT_SUCCESS != result) {
    return;
}
// 2.2 初始化混音器
result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
if (SL_RESULT_SUCCESS != result) {
    return;
}
//不启用混响可以不用获取混音器接口
// 获得混音器接口
//result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
//                                         &outputMixEnvironmentalReverb);
//if (SL_RESULT_SUCCESS == result) {
//设置混响 ： 默认。
//SL_I3DL2_ENVIRONMENT_PRESET_ROOM: 室内
//SL_I3DL2_ENVIRONMENT_PRESET_AUDITORIUM : 礼堂 等
//const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
//(*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
//       outputMixEnvironmentalReverb, &settings);
//}
```

### 3、创建播放器

```C++
//3.1 配置输入声音信息
//创建buffer缓冲类型的队列 2个队列
SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                   2};
//pcm数据格式
//SL_DATAFORMAT_PCM：数据格式为pcm格式
//2：双声道
//SL_SAMPLINGRATE_44_1：采样率为44100
//SL_PCMSAMPLEFORMAT_FIXED_16：采样格式为16bit
//SL_PCMSAMPLEFORMAT_FIXED_16：数据大小为16bit
//SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT：左右声道（双声道）
//SL_BYTEORDER_LITTLEENDIAN：小端模式
SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                               SL_PCMSAMPLEFORMAT_FIXED_16,
                               SL_PCMSAMPLEFORMAT_FIXED_16,
                               SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                               SL_BYTEORDER_LITTLEENDIAN};

//数据源 将上述配置信息放到这个数据源中
SLDataSource audioSrc = {&loc_bufq, &format_pcm};

//3.2 配置音轨（输出）
//设置混音器
SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
SLDataSink audioSnk = {&loc_outmix, NULL};
//需要的接口 操作队列的接口
const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
const SLboolean req[1] = {SL_BOOLEAN_TRUE};
//3.3 创建播放器
result = (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject, &audioSrc, &audioSnk, 1, ids, req);
if (SL_RESULT_SUCCESS != result) {
    return;
}
//3.4 初始化播放器：SLObjectItf bqPlayerObject
result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
if (SL_RESULT_SUCCESS != result) {
    return;
}
//3.5 获取播放器接口：SLPlayItf bqPlayerPlay
result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
if (SL_RESULT_SUCCESS != result) {
    return;
}
```

### 4、设置播放回调函数

```C++
//4.1 获取播放器队列接口：SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue
(*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);

//4.2 设置回调 void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
(*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);
```

```C++
//4.3 创建回调函数
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    ...
}
```

### 5、设置播放器状态为播放状态

```C++
(*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
```

### 6、手动激活回调函数

```C++
bqPlayerCallback(bqPlayerBufferQueue, this);
```

### 7、释放

```C++
//7.1 设置停止状态
if (bqPlayerPlay) {
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
    bqPlayerPlay = 0;
}
//7.2 销毁播放器
if (bqPlayerObject) {
    (*bqPlayerObject)->Destroy(bqPlayerObject);
    bqPlayerObject = 0;
    bqPlayerBufferQueue = 0;
}
//7.3 销毁混音器
if (outputMixObject) {
    (*outputMixObject)->Destroy(outputMixObject);
    outputMixObject = 0;
}
//7.4 销毁引擎
if (engineObject) {
    (*engineObject)->Destroy(engineObject);
    engineObject = 0;
    engineInterface = 0;
}
```

[1]: https://github.com/googlesamples/android-ndk/blob/master/native-audio/app/src/main/cpp/native-audio-jni.c

# 视频原生绘制

## View

### SurfaceView

​	Activity的View hierachy的树形结构，最顶层的DecorView，也就是根结点视图，在SurfaceFlinger中有对应的Layer。

​	对于具有SurfaceView的窗口来说，每一个SurfaceView在SurfaceFlinger服务中还对应有一个独立的Layer，用来单独描述它的绘图表面，以区别于它的宿主窗口的绘图表面。

​	在WMS和SurfaceFlinger中，它与宿主窗口是分离的。这样的好处是对这个Surface的渲染可以放到单独线程去做。这对于一些游戏、视频等性能相关的应用非常有益，因为它不会影响主线程对事件的响应。但它也有缺点，因为这个Surface不在View hierachy中，它的显示也不受View的属性控制，所以不能进行平移，缩放等变换，一些View中的特性也无法使用。

![surfaceView](/Users/lixin/Documents/源码/安卓高级开发—NDK开发/3.2.4 视频解码与原生绘制播放-3/资料/surfaceView.png)



优点：

​	可以在一个独立的线程中进行绘制，不会影响主线程。

​	使用双缓冲机制，播放视频时画面更流畅。

缺点：	

​	Surface不在View hierachy中，显示也不受View的属性控制，所以不能进行平移，缩放等变换。



> 双缓冲：两张Canvas，一张frontCanvas和一张backCanvas，每次实际显示的是frontCanvas，backCanvas存储的是上一次更改前的视图，当使用lockCanvas（）获取画布，得到的backCanvas而不是正在显示的frontCanvas，之后在获取到的backCanvas上绘制新视图，再unlockCanvasAndPost更新视图，上传的这张canvas将替换原来的frontCanvas作为新的frontCanvas，原来的frontCanvas将切换到后台作为backCanvas。



### TextureView

​	在4.0(API level 14)中引入。和SurfaceView不同，不会在WMS中单独创建窗口，而是作为View hierachy中的一个普通View，因此可以和其它普通View一样进行移动，旋转，缩放，动画等变化。TextureView必须在硬件加速的窗口中。

优点：

​	支持移动、旋转、缩放等动画，支持截图

缺点：

​	必须在硬件加速的窗口中使用，占用内存比SurfaceView高(因为开启了硬件加速)，可能有1〜3帧延迟。





### Surface与SurfaceTexture

​	如果说Surface是画布(画框)， SurfaceTexture则是一幅画。可以使用`new Surface(SurfaceTexture)`创建一个Surface。SurfaceTexture并不直接显示图像，而是转为一个外部纹理(图像)，用于图像的二次处理。

```java
//创建一个纹理id
int[] mTextures = new int[1];
SurfaceTexture mSurfaceTexture = new SurfaceTexture(mTextures[0]);
//摄像头作为图像流 交给SurfaceTexture处理
Camera.setPreviewTexture(mSurfaceTexture);
//OpengGL可以通过 mTextures 对摄像头图像进行二次处理
```



## ANativeWindow

​	ANativeWindow代表的是本地窗口，可以看成NDK提供Native版本的Surface。通过`ANativeWindow_fromSurface`获得ANativeWindow指针，`ANativeWindow_release`进行释放。类似Java，可以对它进行lock、unlockAndPost以及通过`ANativeWindow_Buffer`进行图像数据的修改。

```c++
#include <android/native_window_jni.h>
//先释放之前的显示窗口
if (window) {
	ANativeWindow_release(window);
	window = 0;
}
//创建新的窗口用于视频显示
window = ANativeWindow_fromSurface(env, surface);
//设置窗口属性
ANativeWindow_setBuffersGeometry(window, w,
                                     h,
                                     WINDOW_FORMAT_RGBA_8888);

ANativeWindow_Buffer window_buffer;
if (ANativeWindow_lock(window, &window_buffer, 0)) {
	ANativeWindow_release(window);
	window = 0;
	return;
}
//填充rgb数据给dst_data
uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
//......
ANativeWindow_unlockAndPost(window);
```

在NDK中使用ANativeWindow编译时需要链接NDK中的`libandroid.so`库

```cmake
#编译链接NDK/platforms/android-X/usr/lib/libandroid.so
target_link_libraries(XXX android )
```

