#pragma once

#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/avfft.h>
#include <libavdevice/avdevice.h>

#include <libavfilter/avfilter.h>
//#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>

#include <libavformat/avformat.h>
#include <libavformat/avio.h>

// libav resample
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/file.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/time.h>

// hwaccel
#include "libavcodec/vdpau.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_vdpau.h"

// lib swresample
#include <libswscale/swscale.h>
};

class Video;

class VideoSaver {
  public:
    VideoSaver(Video *video, const std::string &videoFileName);

    void run();

  private:
    Video *video = nullptr;
    const std::string &videoFileName;

    const int fps = 60; // FIXME maybe calculate fps?

    AVCodecContext *cctx = nullptr;
    AVFormatContext *ofctx = nullptr;
    AVFrame *videoFrame = nullptr;
    SwsContext *swsCtx = nullptr;
    AVOutputFormat *oformat = nullptr;

    void init();
    void process();
    void cleanUp();
    void remux();
    void free();
};
