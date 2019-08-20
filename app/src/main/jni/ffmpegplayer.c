//
// Created by DELL on 2019/7/16.
//

#include "include/com_android_jni_ffmpeg_FfmpegPlayer.h"
#include"include/libavformat/avformat.h"
#include"include/libswscale/swscale.h"
#include"include/libswresample/swresample.h"
#include"include/libavcodec/avcodec.h"
#include"include/libavutil/pixdesc.h"
#include "include/libyuv.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);

#define MAX_AUDIO_FRME_SIZE 48000 * 4

AVFormatContext *ofmt_ctx;
AVStream *video_st;
AVCodecContext *pCodecCtx;
AVCodec *pCodec;
AVPacket enc_pkt;
AVFrame *pFrameYUV;

int framecnt = 0;
int yuv_width;
int yuv_height;
int y_length;
int uv_length;
int64_t start_time;

//Output FFmpeg's av_log()
void custom_log(void *ptr, int level, const char *fmt, va_list vl) {
    FILE *fp = fopen("/storage/emulated/0/av_log.txt", "a+");
    if (fp) {
        vfprintf(fp, fmt, vl);
        fflush(fp);
        fclose(fp);
    }
}

/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT jstring JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_init
        (JNIEnv *env, jobject jo){

//    (*env)->
    return (*env)->NewStringUTF(env,"hello boy!");
}

//视频解码保存
/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    videoDecode
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_videoDecode
        (JNIEnv *env, jobject jo, jstring input_jstr, jstring output_jstr){

    const char* input_cstr = (*env)->GetStringUTFChars(env,input_jstr,NULL);
    const char* output_cstr = (*env)->GetStringUTFChars(env,output_jstr,NULL);

    //1.注册组件
    av_register_all();

    //封装格式上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //2.打开输入视频文件
    if(avformat_open_input(&pFormatCtx,input_cstr,NULL,NULL) != 0){
        LOGE("%s","打开输入视频文件失败");
        return;
    }
    //3.获取视频信息
    if(avformat_find_stream_info(pFormatCtx,NULL) < 0){
        LOGE("%s","获取视频信息失败");
        return;
    }

    //视频解码，需要找到视频对应的AVStream所在pFormatCtx->streams的索引位置
    int video_stream_idx = -1;
    int i = 0;
    for(; i < pFormatCtx->nb_streams;i++){
        //根据类型判断，是否是视频流
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_idx = i;
            break;
        }
    }

    //4.获取视频解码器
    AVCodecContext *pCodeCtx = pFormatCtx->streams[video_stream_idx]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodeCtx->codec_id);
    if(pCodec == NULL){
        LOGE("%s","无法解码");
        return;
    }

    //5.打开解码器
    if(avcodec_open2(pCodeCtx,pCodec,NULL) < 0){
        LOGE("%s","解码器无法打开");
        return;
    }

    //编码数据
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    //像素数据（解码数据）
    AVFrame *frame = av_frame_alloc();
    AVFrame *yuvFrame = av_frame_alloc();

    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓冲区分配内存
    uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodeCtx->width, pCodeCtx->height));
    //初始化缓冲区
    avpicture_fill((AVPicture *)yuvFrame, out_buffer, AV_PIX_FMT_YUV420P, pCodeCtx->width, pCodeCtx->height);


    //输出文件
    FILE* fp_yuv = fopen(output_cstr,"wb");

    //用于像素格式转换或者缩放
    struct SwsContext *sws_ctx = sws_getContext(
            pCodeCtx->width, pCodeCtx->height, pCodeCtx->pix_fmt,
            pCodeCtx->width, pCodeCtx->height, AV_PIX_FMT_YUV420P,
            SWS_BILINEAR, NULL, NULL, NULL);

    int len ,got_frame, framecount = 0;
    //6.一阵一阵读取压缩的视频数据AVPacket
    while(av_read_frame(pFormatCtx,packet) >= 0){
        //解码AVPacket->AVFrame
        len = avcodec_decode_video2(pCodeCtx, frame, &got_frame, packet);

        //Zero if no frame could be decompressed
        //非零，正在解码
        if(got_frame){
            //frame->yuvFrame (YUV420P)
            //转为指定的YUV420P像素帧
            sws_scale(sws_ctx,
                      frame->data,frame->linesize, 0, frame->height,
                      yuvFrame->data, yuvFrame->linesize);

            //向YUV文件保存解码之后的帧数据
            //AVFrame->YUV
            //一个像素包含一个Y
            int y_size = pCodeCtx->width * pCodeCtx->height;
            fwrite(yuvFrame->data[0], 1, y_size, fp_yuv);
            fwrite(yuvFrame->data[1], 1, y_size/4, fp_yuv);
            fwrite(yuvFrame->data[2], 1, y_size/4, fp_yuv);

            LOGI("解码%d帧",framecount++);
        }

        av_free_packet(packet);
    }

    fclose(fp_yuv);

    av_frame_free(&frame);
    avcodec_close(pCodeCtx);
    avformat_free_context(pFormatCtx);

    (*env)->ReleaseStringUTFChars(env,input_jstr,input_cstr);
    (*env)->ReleaseStringUTFChars(env,output_jstr,output_cstr);

    /**
     * 解码完成，返回提示
     */
    jclass jc = (*env)->GetObjectClass(env,jo);
    jmethodID id = (*env)->GetMethodID(env,jc,"showToast","(Ljava/lang/String;)V");
//    jobject fieldjob = (*env)->GetObjectField(env,jo,id);
    (*env)->CallVoidMethod(env,jo,id,(*env)->NewStringUTF(env,"解码已完成,具体请看jason的log打印结果"));

};

//视频解码播放
/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    videoDecodeDisplay
 * Signature: (Ljava/lang/String;Landroid/view/Surface;)V
 */
JNIEXPORT void JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_videoDecodeDisplay
        (JNIEnv *env, jobject jo, jstring input_jstr, jobject surface){

    const char* input_cstr = (*env)->GetStringUTFChars(env,input_jstr,NULL);
    //1.注册组件
    av_register_all();

    //封装格式上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //2.打开输入视频文件
    if(avformat_open_input(&pFormatCtx,input_cstr,NULL,NULL) != 0){
        LOGE("%s","打开输入视频文件失败");
        return;
    }
    //3.获取视频信息
    if(avformat_find_stream_info(pFormatCtx,NULL) < 0){
        LOGE("%s","获取视频信息失败");
        return;
    }

    //视频解码，需要找到视频对应的AVStream所在pFormatCtx->streams的索引位置
    int video_stream_idx = -1;
//    int i = 0;
//    for(; i < pFormatCtx->nb_streams;i++){
//        //根据类型判断，是否是视频流
//        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
//            video_stream_idx = i;
//            break;
//        }
//    }

    if(pFormatCtx->streams[0]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
        video_stream_idx = 0;
    } else{
        return;
    }

    //4.获取视频解码器
    AVCodecContext *pCodeCtx = pFormatCtx->streams[video_stream_idx]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodeCtx->codec_id);
    if(pCodec == NULL){
        LOGE("%s","无法解码");
        return;
    }

    //5.打开解码器
    if(avcodec_open2(pCodeCtx,pCodec,NULL) < 0){
        LOGE("%s","解码器无法打开");
        return;
    }

    //编码数据
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    //像素数据（解码数据）
    AVFrame *yuv_frame = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();

    //native绘制
    //窗体
    ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env,surface);
    //绘制时的缓冲区
    ANativeWindow_Buffer outBuffer;

    int len ,got_frame, framecount = 0;
    //6.一阵一阵读取压缩的视频数据AVPacket
    while(av_read_frame(pFormatCtx,packet) >= 0){
        //解码AVPacket->AVFrame
        len = avcodec_decode_video2(pCodeCtx, yuv_frame, &got_frame, packet);

        //Zero if no frame could be decompressed
        //非零，正在解码
        if(got_frame){
            LOGI("解码%d帧",framecount++);
            //lock
            //设置缓冲区的属性（宽、高、像素格式）
            ANativeWindow_setBuffersGeometry(nativeWindow, pCodeCtx->width, pCodeCtx->height,WINDOW_FORMAT_RGBA_8888);
            ANativeWindow_lock(nativeWindow,&outBuffer,NULL);

            //设置rgb_frame的属性（像素格式、宽高）和缓冲区
            //rgb_frame缓冲区与outBuffer.bits是同一块内存
            avpicture_fill((AVPicture *)rgb_frame, outBuffer.bits, PIX_FMT_RGBA, pCodeCtx->width, pCodeCtx->height);

            //YUV->RGBA_8888
            I420ToARGB(yuv_frame->data[0],yuv_frame->linesize[0],
                       yuv_frame->data[2],yuv_frame->linesize[2],
                       yuv_frame->data[1],yuv_frame->linesize[1],
                       rgb_frame->data[0], rgb_frame->linesize[0],
                       pCodeCtx->width,pCodeCtx->height);

            //unlock
            ANativeWindow_unlockAndPost(nativeWindow);

            usleep(1000 * 16);

        }

        av_free_packet(packet);
    }

    ANativeWindow_release(nativeWindow);
    av_frame_free(&yuv_frame);
    avcodec_close(pCodeCtx);
    avformat_free_context(pFormatCtx);

    (*env)->ReleaseStringUTFChars(env,input_jstr,input_cstr);

    /**
     * 解码完成，返回提示
     */
    jclass jc = (*env)->GetObjectClass(env,jo);
    jmethodID id = (*env)->GetMethodID(env,jc,"showToast","(Ljava/lang/String;)V");
//    jobject fieldjob = (*env)->GetObjectField(env,jo,id);
    (*env)->CallVoidMethod(env,jo,id,(*env)->NewStringUTF(env,"解码已完成"));

};

//音频解码
/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    soundDecode
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_soundDecode
        (JNIEnv *env, jobject jthiz, jstring input_jstr, jstring output_jstr){

    const char* input_cstr = (*env)->GetStringUTFChars(env,input_jstr,NULL);
    const char* output_cstr = (*env)->GetStringUTFChars(env,output_jstr,NULL);
    LOGI("%s","sound");
    //注册组件
    av_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    //打开音频文件
    if(avformat_open_input(&pFormatCtx,input_cstr,NULL,NULL) != 0){
        LOGI("%s","无法打开音频文件");
        return;
    }
    //获取输入文件信息
    if(avformat_find_stream_info(pFormatCtx,NULL) < 0){
        LOGI("%s","无法获取输入文件信息");
        return;
    }
    //获取音频流索引位置
    int i = 0, audio_stream_idx = -1;
    for(; i < pFormatCtx->nb_streams;i++){
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_idx = i;
            break;
        }
    }

    //获取解码器
    AVCodecContext *codecCtx = pFormatCtx->streams[audio_stream_idx]->codec;
    AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
    if(codec == NULL){
        LOGI("%s","无法获取解码器");
        return;
    }
    //打开解码器
    if(avcodec_open2(codecCtx,codec,NULL) < 0){
        LOGI("%s","无法打开解码器");
        return;
    }
    //压缩数据
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    //解压缩数据
    AVFrame *frame = av_frame_alloc();
    //frame->16bit 44100 PCM 统一音频采样格式与采样率
    SwrContext *swrCtx = swr_alloc();

    //重采样设置参数-------------start
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = codecCtx->sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int in_sample_rate = codecCtx->sample_rate;
    //输出采样率
    int out_sample_rate = in_sample_rate;
    //获取输入的声道布局
    //根据声道个数获取默认的声道布局（2个声道，默认立体声stereo）
    //av_get_default_channel_layout(codecCtx->channels);
    uint64_t in_ch_layout = codecCtx->channel_layout;
    //输出的声道布局（立体声）
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    swr_alloc_set_opts(swrCtx,
                       out_ch_layout,out_sample_fmt,out_sample_rate,
                       in_ch_layout,in_sample_fmt,in_sample_rate,
                       0, NULL);
    swr_init(swrCtx);

    //输出的声道个数
    int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);

    //重采样设置参数-------------end

    //JNI begin------------------
    //JasonPlayer
    jclass player_class = (*env)->GetObjectClass(env,jthiz);

    //AudioTrack对象
    jmethodID create_audio_track_mid = (*env)->GetMethodID(env,player_class,"createAudioTrack","(II)Landroid/media/AudioTrack;");
    jobject audio_track = (*env)->CallObjectMethod(env,jthiz,create_audio_track_mid,out_sample_rate,out_channel_nb);

    //调用AudioTrack.play方法
    jclass audio_track_class = (*env)->GetObjectClass(env,audio_track);
    jmethodID audio_track_play_mid = (*env)->GetMethodID(env,audio_track_class,"play","()V");
    (*env)->CallVoidMethod(env,audio_track,audio_track_play_mid);

    //AudioTrack.write
    jmethodID audio_track_write_mid = (*env)->GetMethodID(env,audio_track_class,"write","([BII)I");

    //JNI end------------------
    FILE *fp_pcm = fopen(output_cstr,"wb");

    //16bit 44100 PCM 数据
    uint8_t *out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRME_SIZE);

    int got_frame = 0,index = 0, ret;
    //不断读取压缩数据
    while(av_read_frame(pFormatCtx,packet) >= 0){
        //解码音频类型的Packet
        if(packet->stream_index == audio_stream_idx){
            //解码
            ret = avcodec_decode_audio4(codecCtx,frame,&got_frame,packet);

            if(ret < 0){
                LOGI("%s","解码完成");
            }
            //解码一帧成功
            if(got_frame > 0){
                LOGI("解码：%d",index++);
                swr_convert(swrCtx, &out_buffer, MAX_AUDIO_FRME_SIZE,(const uint8_t **)frame->data,frame->nb_samples);
                //获取sample的size
                int out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb,
                                                                 frame->nb_samples, out_sample_fmt, 1);
                fwrite(out_buffer,1,out_buffer_size,fp_pcm);

                //out_buffer缓冲区数据，转成byte数组
                jbyteArray audio_sample_array = (*env)->NewByteArray(env,out_buffer_size);
                jbyte* sample_bytep = (*env)->GetByteArrayElements(env,audio_sample_array,NULL);
                //out_buffer的数据复制到sampe_bytep
                memcpy(sample_bytep,out_buffer,out_buffer_size);
                //同步
                (*env)->ReleaseByteArrayElements(env,audio_sample_array,sample_bytep,0);

                //AudioTrack.write PCM数据
                (*env)->CallIntMethod(env,audio_track,audio_track_write_mid,
                                      audio_sample_array,0,out_buffer_size);
                //释放局部引用
                (*env)->DeleteLocalRef(env,audio_sample_array);
                usleep(1000 * 16);
            }
        }

        av_free_packet(packet);
    }

    av_frame_free(&frame);
    av_free(out_buffer);

    swr_free(&swrCtx);
    avcodec_close(codecCtx);
    avformat_close_input(&pFormatCtx);

    (*env)->ReleaseStringUTFChars(env,input_jstr,input_cstr);
    (*env)->ReleaseStringUTFChars(env,output_jstr,output_cstr);

};

#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    cameraLiveInit
 * Signature: (IILjava/lang/String;)V
 */
JNIEXPORT jint JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_cameraLiveInit
        (JNIEnv *env, jobject obj, jint width, jint height, jstring jurlpath){

    const char *out_path = (*env)->GetStringUTFChars(env,jurlpath, 0) ;
    //const char *out_path = "rtmp://192.168.9.135:1935/wstv/home";//ws
    yuv_width = width;
    yuv_height = height;
    y_length = width * height;
    uv_length = width * height / 4;

    //FFmpeg av_log() callback
    av_log_set_callback(custom_log);

    av_register_all();
    avformat_network_init();//ws add
    //output initialize
    avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", out_path);
    //output encoder initialize
    pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!pCodec) {
        LOGE("Can not find encoder!\n");
        return -1;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        LOGE("Could not allocate video codec context\n");
        return -1;
    }
    pCodecCtx->pix_fmt =  AV_PIX_FMT_YUV420P;//PIX_FMT_YUV420P新版加
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    pCodecCtx->bit_rate = 400000;
    pCodecCtx->gop_size = 250;
    /* Some formats want stream headers to be separate. */
    if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;

    //H264 codec param
    //pCodecCtx->me_range = 16;
    //pCodecCtx->max_qdiff = 4;
    //pCodecCtx->qcompress = 0.6;
    pCodecCtx->qmin = 10;
    pCodecCtx->qmax = 51;
    //Optional Param
    pCodecCtx->max_b_frames = 0;
    // Set H264 preset and tune
    AVDictionary *param = 0;
    //av_dict_set(&param, "preset", "ultrafast", 0);
    //av_dict_set(&param, "tune", "zerolatency", 0);
    av_opt_set(pCodecCtx->priv_data, "preset", "ultrafast", 0);
    av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);

    if (avcodec_open2(pCodecCtx, pCodec, &param) < 0) {
        LOGE("Failed to open encoder!\n");
        return -1;
    }

    //Add a new stream to output,should be called by the user before avformat_write_header() for muxing
    video_st = avformat_new_stream(ofmt_ctx, pCodec);//avformat_new_stream创建流通道
    if (video_st == NULL) {
        return -1;
    }
    video_st->time_base.num = 1;
    video_st->time_base.den = 30;
    video_st->codec = pCodecCtx;

    //Open output URL,set before avformat_write_header() for muxing
    if (avio_open(&ofmt_ctx->pb, out_path, AVIO_FLAG_READ_WRITE) < 0) {
        LOGE("Failed to open output file!\n");
        return -1;
    }

    //Write File Header
    avformat_write_header(ofmt_ctx, NULL);

    start_time = av_gettime();
    return 0;

};

/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    cameraLiveStart
 * Signature: ([B)V
 */
JNIEXPORT jint JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_cameraLiveStart
        (JNIEnv *env, jobject obj, jbyteArray yuv){

    int ret;
    int enc_got_frame = 0;
    int i = 0;

    pFrameYUV = av_frame_alloc();//旧版 avcodec_alloc_frame() //分配一个AVFrame结构体。
    uint8_t *out_buffer = (uint8_t *) av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
    avpicture_fill((AVPicture *) pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width,pCodecCtx->height);

    //安卓摄像头数据为NV21格式，此处将其转换为YUV420P格式
    jbyte *in = (jbyte *) (*env)->GetByteArrayElements(env,yuv, 0);
    memcpy(pFrameYUV->data[0], in, y_length);
    for (i = 0; i < uv_length; i++) {
        *(pFrameYUV->data[2] + i) = *(in + y_length + i * 2);
        *(pFrameYUV->data[1] + i) = *(in + y_length + i * 2 + 1);
    }

    pFrameYUV->format = AV_PIX_FMT_YUV420P;
    pFrameYUV->width = yuv_width;
    pFrameYUV->height = yuv_height;

    enc_pkt.data = NULL;
    enc_pkt.size = 0;
    av_init_packet(&enc_pkt);//初始化 AVPacker
    ret = avcodec_encode_video2(pCodecCtx, &enc_pkt, pFrameYUV, &enc_got_frame);
    av_frame_free(&pFrameYUV);

    if (enc_got_frame == 1) {
        LOGI("Succeed to encode frame: %5d\tsize:%5d\n", framecnt, enc_pkt.size);
        framecnt++;
        enc_pkt.stream_index = video_st->index;

        //Write PTS
        AVRational time_base = ofmt_ctx->streams[0]->time_base;//{ 1, 1000 };
        AVRational r_framerate1 = {60, 2};//{ 50, 2 };
        AVRational time_base_q = {1, AV_TIME_BASE};
        //Duration between 2 frames (us)
        int64_t calc_duration = (double) (AV_TIME_BASE) * (1 / av_q2d(r_framerate1));    //内部时间戳
        //Parameters
        //enc_pkt.pts = (double)(framecnt*calc_duration)*(double)(av_q2d(time_base_q)) / (double)(av_q2d(time_base));
        enc_pkt.pts = av_rescale_q(framecnt * calc_duration, time_base_q, time_base);
        enc_pkt.dts = enc_pkt.pts;
        enc_pkt.duration = av_rescale_q(calc_duration, time_base_q,time_base); //(double)(calc_duration)*(double)(av_q2d(time_base_q)) / (double)(av_q2d(time_base));
        enc_pkt.pos = -1;

        //Delay
        int64_t pts_time = av_rescale_q(enc_pkt.dts, time_base, time_base_q);
        int64_t now_time = av_gettime() - start_time;
        if (pts_time > now_time)
            av_usleep(pts_time - now_time);

        ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);//将AVPacket（存储视频压缩码流数据）写入文件。
        av_free_packet(&enc_pkt);
    }

    return 0;

};

/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    cameraLiveStop
 * Signature: ()V
 */
JNIEXPORT jint JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_cameraLiveStop
        (JNIEnv *env, jobject obj){

    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(ofmt_ctx->streams[0]->codec->codec->capabilities &
          CODEC_CAP_DELAY))
        return 0;
    while (1) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2(ofmt_ctx->streams[0]->codec, &enc_pkt,
                                    NULL, &got_frame);
        if (ret < 0)
            break;
        if (!got_frame) {
            ret = 0;
            break;
        }
        LOGI("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);

        //Write PTS
        AVRational time_base = ofmt_ctx->streams[0]->time_base;//{ 1, 1000 };
        AVRational r_framerate1 = {60, 2};
        AVRational time_base_q = {1, AV_TIME_BASE};
        //Duration between 2 frames (us)
        int64_t calc_duration = (double) (AV_TIME_BASE) * (1 / av_q2d(r_framerate1));    //内部时间戳
        //Parameters
        enc_pkt.pts = av_rescale_q(framecnt * calc_duration, time_base_q, time_base);
        enc_pkt.dts = enc_pkt.pts;
        enc_pkt.duration = av_rescale_q(calc_duration, time_base_q, time_base);

        //转换PTS/DTS（Convert PTS/DTS）
        enc_pkt.pos = -1;
        framecnt++;
        ofmt_ctx->duration = enc_pkt.duration * framecnt;

        /* mux encoded frame */
        ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }
    //Write file trailer
    av_write_trailer(ofmt_ctx);
    return 0;

};

/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    cameraLiveClose
 * Signature: ()V
 */
JNIEXPORT jint JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_cameraLiveClose
        (JNIEnv *env, jobject obj){

    if (video_st)
        avcodec_close(video_st->codec);
    avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    return 0;

};

/*
 * Class:     com_android_jni_ffmpeg_FfmpegPlayer
 * Method:    cameraLiveJason
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_android_jni_ffmpeg_FfmpegPlayer_cameraLiveJason
        (JNIEnv *env, jobject obj, jstring input_jstr, jstring output_jstr){

    //java string->c char*
    const char *input_cstr = (*env)->GetStringUTFChars(env,input_jstr, JNI_FALSE);
    const char *output_cstr = (*env)->GetStringUTFChars(env,output_jstr, JNI_FALSE);
    //变量初始化
    AVFormatContext *inFmtCtx = NULL, *outFmtCtx = NULL;
    int ret;
    //注册组件
    av_register_all();
    //初始化网络
    avformat_network_init();

    //打开输入文件
    if ((ret = avformat_open_input(&inFmtCtx, input_cstr, 0, 0)) < 0) {
        LOGE( "无法打开文件");
        goto end;
    }
    //获取文件信息
    if ((ret = avformat_find_stream_info(inFmtCtx, 0)) < 0) {
        LOGE( "无法获取文件信息");
        goto end;
    }
    //输出的封装格式上下文，使用RTMP协议推送flv封装格式的流
    avformat_alloc_output_context2(&outFmtCtx, NULL, "flv",output_cstr); //RTMP
    //avformat_alloc_output_context2(&ofmt_ctx, NULL, "mpegts", output_str);//UDP

    int i = 0;
    for (; i < inFmtCtx->nb_streams; i++) {
        //根据输入封装格式中的AVStream流，来创建输出封装格式的AVStream流
        //解码器，解码器上下文都要一致
        AVStream *in_stream = inFmtCtx->streams[i];
        AVStream *out_stream = avformat_new_stream(outFmtCtx, in_stream->codec->codec);
        //复制解码器上下文
        ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
        //全局头
        out_stream->codec->codec_tag = 0;
        if (outFmtCtx->oformat->flags == AVFMT_GLOBALHEADER){
            out_stream->codec->flags = CODEC_FLAG_GLOBAL_HEADER;
        }
    }

    //打开输出的AVIOContext IO流上下文
    AVOutputFormat *ofmt = outFmtCtx->oformat;
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&outFmtCtx->pb, output_cstr, AVIO_FLAG_WRITE);
    }

    //先写一个头
    ret = avformat_write_header(outFmtCtx, NULL);
    if (ret < 0) {
        LOGE( "推流发生错误\n");
        goto end;
    }
    //获取视频流的索引位置
    int videoindex=-1;
    for(i=0; i<inFmtCtx->nb_streams; i++){
        if(inFmtCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
            videoindex=i;
            break;
        }
    }
    int frame_index=0;
    int64_t start_time=av_gettime();
    AVPacket pkt;
    while (1) {
        AVStream *in_stream, *out_stream;
        //读取AVPacket
        ret = av_read_frame(inFmtCtx, &pkt);
        if (ret < 0)
            break;
        //没有封装格式的裸流（例如H.264裸流）是不包含PTS、DTS这些参数的。在发送这种数据的时候，需要自己计算并写入AVPacket的PTS，DTS，duration等参数
        //PTS：Presentation Time Stamp。PTS主要用于度量解码后的视频帧什么时候被显示出来
        //DTS：Decode Time Stamp。DTS主要是标识读入内存中的流在什么时候开始送入解码器中进行解码
        if(pkt.pts==AV_NOPTS_VALUE){
            //Write PTS
            AVRational time_base1=inFmtCtx->streams[videoindex]->time_base;
            //Duration between 2 frames (us)
            int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(inFmtCtx->streams[videoindex]->r_frame_rate);
            //Parameters
            pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
            pkt.dts=pkt.pts;
            pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
        }

        if(pkt.stream_index==videoindex){
            //FFmpeg处理数据速度很快，瞬间就能把所有的数据发送出去，流媒体服务器是接受不了
            //这里采用av_usleep()函数休眠的方式来延迟发送，延时时间根据帧率与时间基准计算得到
            AVRational time_base=inFmtCtx->streams[videoindex]->time_base;
            AVRational time_base_q={1,AV_TIME_BASE};
            int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
            int64_t now_time = av_gettime() - start_time;
            if (pts_time > now_time){
                av_usleep(pts_time - now_time);
            }
        }

        in_stream  = inFmtCtx->streams[pkt.stream_index];
        out_stream = outFmtCtx->streams[pkt.stream_index];
        /* copy packet */
        //Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        //输出进度
        if(pkt.stream_index==videoindex){
            LOGI("第%d帧",frame_index);
            frame_index++;
        }
        //推送
        ret = av_interleaved_write_frame(outFmtCtx, &pkt);

        if (ret < 0) {
            LOGE( "Error muxing packet");
            break;
        }
        av_free_packet(&pkt);

    }
    //输出结尾
    av_write_trailer(outFmtCtx);
    end:
    //释放资源
    avformat_free_context(inFmtCtx);
    avio_close(outFmtCtx->pb);
    avformat_free_context(outFmtCtx);

};