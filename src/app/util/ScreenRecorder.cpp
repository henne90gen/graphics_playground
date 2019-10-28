#include <cmath>

#include "ScreenRecorder.h"

#include <filesystem>
#include <functional>
#include <iomanip>
#include <iostream>

#define GIF_FLIP_VERT

#include <gif.h>

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

#include "Image.h"
#include "OpenGLUtils.h"

#define VIDEO_TMP_FILE "tmp.h264"

std::string generateScreenshotFilename() {
    std::stringstream buffer;
    std::time_t t = std::time(nullptr);
    buffer << std::put_time(std::localtime(&t), "%Y-%m-%d-%H:%M:%S");
    return "../../../screenshot-" + buffer.str() + ".png";
}

std::string generateScreenrecordingFilename(const std::string &directory, unsigned int frameIndex) {
    return directory + std::to_string(frameIndex) + ".png";
}

std::string generateScreenrecordingDirectoryName(unsigned int recordingIndex) {
    std::stringstream buffer;
    std::time_t t = std::time(nullptr);
    buffer << std::put_time(std::localtime(&t), "%Y-%m-%d-%H-%M-%S");
    return "../../../screenrecording-" + buffer.str() + "-" + std::to_string(recordingIndex) + "/";
}

std::string generateScreenrecordingName(unsigned int recordingIndex, std::string fileExtension) {
    std::string dir = generateScreenrecordingDirectoryName(recordingIndex);
    std::string fileName = dir.substr(0, dir.size() - 1);
    return fileName + fileExtension;
}

std::string generateScreenrecordingGifName(unsigned int recordingIndex) {
    return generateScreenrecordingName(recordingIndex, ".gif");
}

std::string generateScreenrecordingMp4Name(unsigned int recordingIndex) {
    return generateScreenrecordingName(recordingIndex, ".mp4");
}

void ScreenRecorder::saveScreenshot(unsigned int windowWidth, unsigned int windowHeight) {
    Image image = {};
    image.fileName = generateScreenshotFilename();
    image.width = windowWidth;
    image.height = windowHeight;
    image.channels = 3;

    const int numberOfPixels = image.width * image.height * image.channels;
    image.pixels = std::vector<unsigned char>(numberOfPixels);

    GL_Call(glPixelStorei(GL_PACK_ALIGNMENT, 1));
    GL_Call(glReadBuffer(GL_FRONT));
    GL_Call(glReadPixels(0, 0, image.width, image.height, GL_RGB, GL_UNSIGNED_BYTE, image.pixels.data()));

    ImageOps::save(image);
    std::cout << "Saved screenshot " << image.fileName << std::endl;
}

void saveFrameToImage(Frame *frame, const std::string &directory) {
    Image image = {};
    image.fileName = generateScreenrecordingFilename(directory, frame->index);
    image.width = frame->width;
    image.height = frame->height;
    image.channels = frame->channels;

    const int numberOfPixels = frame->width * frame->height * frame->channels;
    image.pixels = std::vector<unsigned char>(numberOfPixels);

    unsigned char *ptr = frame->buffer;
    for (unsigned char &pixel : image.pixels) {
        pixel = *ptr;
        ptr++;
    }

    ImageOps::save(image);
}

void ScreenRecorder::saveRecordingAsPng() {
    std::string directory = generateScreenrecordingDirectoryName(recordingIndex);
    if (!std::filesystem::exists(directory)) {
        std::filesystem::create_directory(directory);
    }

    std::function<void(Frame *)> workFunction = [&directory](Frame *currentFrame) {
        saveFrameToImage(currentFrame, directory);
    };
    video.iterateFrames(workFunction);
}

void ScreenRecorder::saveRecordingAsGif() {
    int width = (int)video.getWidth();
    int height = (int)video.getHeight();

    auto fileName = generateScreenrecordingGifName(recordingIndex);
    int delay = 1;
    GifWriter g = {};
    if (!GifBegin(&g, fileName.c_str(), width, height, delay)) {
        std::cerr << "Could not open " << fileName << " for writing." << std::endl;
        video.reset();
        return;
    }

    std::function<void(Frame *)> workFunction = [&g, &delay](Frame *currentFrame) {
        GifWriteFrame(&g, currentFrame->buffer, currentFrame->width, currentFrame->height, delay);
    };
    video.iterateFrames(workFunction);

    if (!GifEnd(&g)) {
        std::cerr << "Could not save to " << fileName << std::endl;
    }
}

void Free(AVFormatContext *ofctx, AVCodecContext *cctx = nullptr, AVFrame *videoFrame = nullptr,
          SwsContext *swsCtx = nullptr) {
    if (videoFrame) {
        av_frame_free(&videoFrame);
    }
    if (cctx) {
        avcodec_free_context(&cctx);
    }
    if (ofctx) {
        avformat_free_context(ofctx);
    }
    if (swsCtx) {
        sws_freeContext(swsCtx);
    }
}

void FreeRemux(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx) {
    if (ifmt_ctx) {
        avformat_close_input(&ifmt_ctx);
    }
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&ofmt_ctx->pb);
    }
    if (ofmt_ctx) {
        avformat_free_context(ofmt_ctx);
    }
}

void Remux(unsigned int recordingIndex, int fps) {
    auto videoFileName = generateScreenrecordingMp4Name(recordingIndex);

    AVFormatContext *ifmt_ctx = nullptr, *ofmt_ctx = nullptr;
    int err = avformat_open_input(&ifmt_ctx, VIDEO_TMP_FILE, 0, 0);
    if (err < 0) {
        std::cout << "Failed to open input file for remuxing (" << err << ")" << std::endl;
        FreeRemux(ifmt_ctx, ofmt_ctx);
        return;
    }

    err = avformat_find_stream_info(ifmt_ctx, 0);
    if (err < 0) {
        std::cout << "Failed to retrieve input stream information (" << err << ")" << std::endl;
        FreeRemux(ifmt_ctx, ofmt_ctx);
        return;
    }

    err = avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, videoFileName.c_str());
    if (err != 0) {
        std::cout << "Failed to allocate output context (" << err << ")" << std::endl;
        FreeRemux(ifmt_ctx, ofmt_ctx);
        return;
    }

    AVStream *inVideoStream = ifmt_ctx->streams[0];
    AVStream *outVideoStream = avformat_new_stream(ofmt_ctx, nullptr);
    if (outVideoStream == nullptr) {
        std::cout << "Failed to allocate output video stream" << std::endl;
        FreeRemux(ifmt_ctx, ofmt_ctx);
        return;
    }

    outVideoStream->time_base = {1, fps};
    avcodec_parameters_copy(outVideoStream->codecpar, inVideoStream->codecpar);
    outVideoStream->codecpar->codec_tag = 0;

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        if ((err = avio_open(&ofmt_ctx->pb, videoFileName.c_str(), AVIO_FLAG_WRITE)) < 0) {
            std::cout << "Failed to open output file (" << err << ")" << std::endl;
            FreeRemux(ifmt_ctx, ofmt_ctx);
            return;
        }
    }

    if ((err = avformat_write_header(ofmt_ctx, 0)) < 0) {
        std::cout << "Failed to write header to output file (" << err << ")" << std::endl;
        FreeRemux(ifmt_ctx, ofmt_ctx);
        return;
    }

    AVPacket videoPkt;
    int ts = 0;
    while (true) {
        if ((err = av_read_frame(ifmt_ctx, &videoPkt)) < 0) {
            break;
        }
        videoPkt.stream_index = outVideoStream->index;
        videoPkt.pts = ts;
        videoPkt.dts = ts;
        videoPkt.duration = av_rescale_q(videoPkt.duration, inVideoStream->time_base, outVideoStream->time_base);
        ts += videoPkt.duration;
        videoPkt.pos = -1;

        if ((err = av_interleaved_write_frame(ofmt_ctx, &videoPkt)) < 0) {
            std::cout << "Failed to mux packet (" << err << ")" << std::endl;
            av_packet_unref(&videoPkt);
            break;
        }
        av_packet_unref(&videoPkt);
    }

    av_write_trailer(ofmt_ctx);
}

void ScreenRecorder::saveRecordingAsMp4() {
    // init
    int width = (int)video.getWidth();
    int height = (int)video.getHeight();
    int fps = 60;    // FIXME maybe calculate fps?
    int bitrate = 0; // FIXME what is this?

    AVOutputFormat *oformat = av_guess_format(nullptr, VIDEO_TMP_FILE, nullptr);
    if (oformat == nullptr) {
        std::cout << "Failed to define output format" << std::endl;
        return;
    }

    AVFormatContext *ofctx;
    int err = avformat_alloc_output_context2(&ofctx, oformat, nullptr, VIDEO_TMP_FILE);
    if (err < 0) {
        std::cout << "Failed to allocate output context (" << err << ")" << std::endl;
        Free(ofctx);
        return;
    }

    AVCodec *codec = avcodec_find_encoder(oformat->video_codec);
    if (codec == nullptr) {
        std::cout << "Failed to find encoder" << std::endl;
        Free(ofctx);
        return;
    }

    AVStream *videoStream = avformat_new_stream(ofctx, codec);
    if (videoStream == nullptr) {
        std::cout << "Failed to create new stream" << std::endl;
        Free(ofctx);
        return;
    }

    AVCodecContext *cctx = avcodec_alloc_context3(codec);
    if (cctx == nullptr) {
        std::cout << "Failed to allocate codec context" << std::endl;
        Free(ofctx, cctx);
        return;
    }

    videoStream->codecpar->codec_id = oformat->video_codec;
    videoStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    videoStream->codecpar->width = width;
    videoStream->codecpar->height = height;
    videoStream->codecpar->format = AV_PIX_FMT_YUV420P;
    videoStream->codecpar->bit_rate = bitrate * 1000;
    videoStream->time_base = {1, fps};

    avcodec_parameters_to_context(cctx, videoStream->codecpar);
    cctx->time_base = {1, fps};
    cctx->max_b_frames = 2;
    cctx->gop_size = 12;
    if (videoStream->codecpar->codec_id == AV_CODEC_ID_H264) {
        av_opt_set(cctx, "preset", "ultrafast", 0);
    }
    if (ofctx->oformat->flags & AVFMT_GLOBALHEADER) {
        cctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    avcodec_parameters_from_context(videoStream->codecpar, cctx);

    if ((err = avcodec_open2(cctx, codec, nullptr)) < 0) {
        std::cout << "Failed to open codec (" << err << ")" << std::endl;
        Free(ofctx, cctx);
        return;
    }

    if (!(oformat->flags & AVFMT_NOFILE)) {
        if ((err = avio_open(&ofctx->pb, VIDEO_TMP_FILE, AVIO_FLAG_WRITE)) < 0) {
            std::cout << "Failed to open file (" << err << ")" << std::endl;
            Free(ofctx, cctx);
            return;
        }
    }

    if ((err = avformat_write_header(ofctx, nullptr)) < 0) {
        std::cout << "Failed to write header (" << err << ")" << std::endl;
        Free(ofctx, cctx);
        return;
    }
    av_dump_format(ofctx, 0, VIDEO_TMP_FILE, 1);

    // process
    int frameCounter = 0;
    AVFrame *videoFrame = nullptr;
    SwsContext *swsCtx = nullptr;
    std::function<void(Frame *)> workFunction = [&videoFrame, &cctx, &swsCtx, &frameCounter,
                                                 &ofctx](Frame *currentFrame) {
        int err;
        if (videoFrame == nullptr) {

            videoFrame = av_frame_alloc();
            videoFrame->format = AV_PIX_FMT_YUV420P;
            videoFrame->width = cctx->width;
            videoFrame->height = cctx->height;

            if ((err = av_frame_get_buffer(videoFrame, 32)) < 0) {
                std::cout << "Failed to allocate picture (" << err << ")" << std::endl;
                return;
            }
        }

        if (swsCtx == nullptr) {
            AVPixelFormat format = AV_PIX_FMT_RGB24;
            if (currentFrame->channels == 1) {
                format = AV_PIX_FMT_GRAY8;
            } else if (currentFrame->channels == 2) {
                format = AV_PIX_FMT_GRAY8A;
            } else if (currentFrame->channels == 3) {
                format = AV_PIX_FMT_RGB24;
            } else if (currentFrame->channels == 4) {
                format = AV_PIX_FMT_RGBA;
            } else {
                std::cout << "Number of channels not supported (" << currentFrame->channels << ")" << std::endl;
                Free(ofctx, cctx, videoFrame, swsCtx);
                return;
            }
            swsCtx = sws_getContext(cctx->width, cctx->height, format, cctx->width, cctx->height, AV_PIX_FMT_YUV420P,
                                    SWS_BICUBIC, nullptr, nullptr, nullptr);
        }

        int inLinesize[1] = {(int)currentFrame->channels * cctx->width};

        // From RGB to YUV
        sws_scale(swsCtx, (const uint8_t *const *)&currentFrame->buffer, inLinesize, 0, cctx->height, videoFrame->data,
                  videoFrame->linesize);

        videoFrame->pts = frameCounter++;

        if ((err = avcodec_send_frame(cctx, videoFrame)) < 0) {
            std::cout << "Failed to send frame (" << err << ")" << std::endl;
            return;
        }

        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = nullptr;
        pkt.size = 0;

        if (avcodec_receive_packet(cctx, &pkt) == 0) {
            pkt.flags |= AV_PKT_FLAG_KEY;
            av_interleaved_write_frame(ofctx, &pkt);
            av_packet_unref(&pkt);
        }
    };
    video.iterateFrames(workFunction);

    // clean up
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;

    for (;;) {
        avcodec_send_frame(cctx, nullptr);
        if (avcodec_receive_packet(cctx, &pkt) == 0) {
            av_interleaved_write_frame(ofctx, &pkt);
            av_packet_unref(&pkt);
        } else {
            break;
        }
    }

    av_write_trailer(ofctx);
    if (!(oformat->flags & AVFMT_NOFILE)) {
        int err = avio_close(ofctx->pb);
        if (err < 0) {
            std::cout << "Failed to close file (" << err << ")" << std::endl;
        }
    }

    Free(ofctx, cctx, videoFrame, swsCtx);

    Remux(recordingIndex, fps);
}

void ScreenRecorder::saveRecording() {
    if (!video.hasFrames()) {
        std::cerr << "There are no frames in this recording." << std::endl;
        return;
    }
    if (recordingType == RecordingType::PNG) {
        saveRecordingAsPng();
    } else if (recordingType == RecordingType::GIF) {
        saveRecordingAsGif();
    } else if (recordingType == RecordingType::MP4) {
        saveRecordingAsMp4();
    } else {
        std::cerr << "Recording type is not supported (" << recordingType << ")" << std::endl;
    }
    recordingIndex++;
    video.reset();
}

void Video::iterateFrames(const std::function<void(Frame *)> &workFunction) {
    Frame *currentFrame = head;
    while (currentFrame != nullptr) {
        if (currentFrame->buffer != nullptr) {
            // only work on frame, if the buffer points to valid memory
            workFunction(currentFrame);
        }

        currentFrame = currentFrame->next;
    }
}

void averagePixel(unsigned int pixel1, unsigned int pixel2, unsigned int *dest) {
    unsigned int r1 = (pixel1 & 0x000000ff);
    unsigned int g1 = (pixel1 & 0x0000ff00) >> 8;
    unsigned int b1 = (pixel1 & 0x00ff0000) >> 16;
    unsigned int r2 = (pixel2 & 0x000000ff);
    unsigned int g2 = (pixel2 & 0x0000ff00) >> 8;
    unsigned int b2 = (pixel2 & 0x00ff0000) >> 16;
    unsigned int r = (r1 + r2) / 2;
    unsigned int g = (g1 + g2) / 2;
    unsigned int b = (b1 + b2) / 2;
    *dest = r + (g << 8) + (b << 16);
}

void copyAndScaleDownFrame(const unsigned char *buffer, Frame *frame, unsigned int screenWidth,
                           unsigned int screenHeight) {
    for (unsigned int y = 0; y < frame->height; y++) {
        for (unsigned int x = 0; x < frame->width; x++) {
            auto xF = (float)x;
            auto yF = (float)y;
            float downScaledX = xF / (float)frame->width;
            float downScaledY = yF / (float)frame->height;
            float upScaledX = downScaledX * (float)screenWidth;
            float upScaledY = downScaledY * (float)screenHeight;

            unsigned int xLeft = std::floor(upScaledX);
            unsigned int xRight = std::ceil(upScaledX);
            unsigned int yTop = std::floor(upScaledY);
            unsigned int yBottom = std::ceil(upScaledY);

            unsigned int index = (xLeft + yTop * screenWidth) * frame->channels;
            unsigned int topLeft = *(unsigned int *)(buffer + index);
            index = (xRight + yTop * screenWidth) * frame->channels;
            unsigned int topRight = *(unsigned int *)(buffer + index);

            index = (xLeft + yBottom * screenWidth) * frame->channels;
            unsigned int bottomLeft = *(unsigned int *)(buffer + index);
            index = (xRight + yBottom * screenWidth) * frame->channels;
            unsigned int bottomRight = *(unsigned int *)(buffer + index);

            unsigned int top;
            averagePixel(topLeft, topRight, &top);
            unsigned int bottom;
            averagePixel(bottomLeft, bottomRight, &bottom);

            unsigned int baseIndex = (x + y * frame->width) * frame->channels;
            auto *pixel = (unsigned int *)(frame->buffer + baseIndex);
            averagePixel(top, bottom, pixel);
        }
    }
}

void Video::recordFrame(unsigned int screenWidth, unsigned int screenHeight) {
    const unsigned int channels = 4;
    const int numberOfPixels = screenWidth * screenHeight * channels;
    auto *buffer = static_cast<unsigned char *>(malloc(numberOfPixels * sizeof(unsigned char)));

    GL_Call(glPixelStorei(GL_PACK_ALIGNMENT, 1));
    GL_Call(glReadBuffer(GL_FRONT));
    GL_Call(glReadPixels(0, 0, screenWidth, screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer));

    tail->width = 800;
    tail->height = 600;
    tail->channels = channels;
    tail->buffer =
          static_cast<unsigned char *>(malloc(tail->width * tail->height * tail->channels * sizeof(unsigned char)));

    copyAndScaleDownFrame(buffer, tail, screenWidth, screenHeight);

    Frame *tmp = tail;
    tail = new Frame();
    tail->index = tmp->index + 1;
    tmp->next = tail;
}

void Video::reset() {
    Frame *currentFrame = head;
    while (currentFrame != nullptr) {
        if (currentFrame->buffer == nullptr) {
            auto next = currentFrame->next;
            free(currentFrame);
            if (next != nullptr) {
                continue;
            }
            break;
        }

        Frame *tmp = currentFrame->next;
        free(currentFrame->buffer);
        free(currentFrame);
        currentFrame = tmp;
    }
    tail = new Frame();
    head = tail;
}
