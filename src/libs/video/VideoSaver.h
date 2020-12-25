#pragma once

#include <memory>
#include <string>
#include <utility>

#if FFMPEG_FOUND
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/avfft.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/file.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
}
#endif

struct Frame {
    unsigned char *buffer = nullptr;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int channels = 3;
    unsigned int index = 0;
    Frame *next = nullptr;
};

class VideoSaver {
  public:
    explicit VideoSaver(std::string videoFileName) : videoFileName(std::move(videoFileName)) {}
    virtual ~VideoSaver() = default;

    virtual bool doInit() = 0;
    virtual void doAcceptFrame(const std::unique_ptr<Frame> &frame) = 0;
    virtual bool doSave() = 0;

    void init(unsigned int frameWidth, unsigned int frameHeight);
    void captureFrame(unsigned int frameWidth, unsigned int frameHeight);
    void save();

    bool isInitialized() { return initialized; }

  protected:
    const std::string videoFileName;
    unsigned int frameWidth = 0;
    unsigned int frameHeight = 0;

  private:
    bool initialized = false;
};

#if FFMPEG_FOUND
class Mp4VideoSaver : public VideoSaver {
  public:
    explicit Mp4VideoSaver(std::string videoFileName) : VideoSaver(std::move(videoFileName)) {}
    ~Mp4VideoSaver() override { this->free(); }

    bool doInit() override;
    void doAcceptFrame(const std::unique_ptr<Frame> &frame) override;
    bool doSave() override;

  private:
    const int fps = 60; // FIXME maybe calculate fps?
    int frameCounter = 0;

    AVCodecContext *cctx = nullptr;
    AVFormatContext *ofctx = nullptr;
    AVFrame *videoFrame = nullptr;
    SwsContext *swsCtx = nullptr;
    AVOutputFormat *oformat = nullptr;

    void cleanUp();
    void remux();
    void free();
};
#endif

struct GifWriter;

class GifVideoSaver : public VideoSaver {
  public:
    explicit GifVideoSaver(std::string videoFileName) : VideoSaver(std::move(videoFileName)) {}
    ~GifVideoSaver() override;

    bool doInit() override;
    void doAcceptFrame(const std::unique_ptr<Frame> &frame) override;
    bool doSave() override;

  private:
    int delay = 1;
    bool scaleDown = true;
    GifWriter *gifWriter = nullptr;
};
