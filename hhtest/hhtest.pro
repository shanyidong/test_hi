TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    sample_vgs.c \
    sample_comm_vpss.c \
    sample_comm_vo.c \
    sample_comm_vi.c \
    sample_comm_venc.c \
    sample_comm_vdec.c \
    sample_comm_vda.c \
    sample_comm_sys.c \
    sample_comm_ivs.c \
    sample_comm_audio.c \
    loadbmp.c

LIBS+=/home/yangwenquan/libdownload/Hi3536_SDK_V2.0.6.0/mpp_single/lib/libmpi.a
LIBS+=/home/yangwenquan/libdownload/Hi3536_SDK_V2.0.6.0/mpp_single/lib/libVoiceEngine.a
LIBS+=/home/yangwenquan/libdownload/Hi3536_SDK_V2.0.6.0/mpp_single/lib/libdnvqe.a
LIBS+=/home/yangwenquan/libdownload/Hi3536_SDK_V2.0.6.0/mpp_single/lib/libupvqe.a
LIBS+=/home/yangwenquan/libdownload/Hi3536_SDK_V2.0.6.0/mpp_single/lib/libhdmi.a
LIBS+=/home/yangwenquan/libdownload/Hi3536_SDK_V2.0.6.0/mpp_single/lib/libjpeg6b.a
LIBS+=/home/yangwenquan/libdownload/Hi3536_SDK_V2.0.6.0/mpp_single/lib/libive.a
HEADERS += \
    sample_comm_ivs.h \
    parm.h
LIBS +=/home/yangwenquan/LIB_3536/ffmpeg/libavcodec.so \
/home/yangwenquan/LIB_3536/ffmpeg/libavfilter.so \
/home/yangwenquan/LIB_3536/ffmpeg/libavformat.so \
/home/yangwenquan/LIB_3536/ffmpeg/libavutil.so \
/home/yangwenquan/LIB_3536/ffmpeg/libpostproc.so \
/home/yangwenquan/LIB_3536/ffmpeg/libswresample.so \
/home/yangwenquan/LIB_3536/ffmpeg/libswscale.so \
