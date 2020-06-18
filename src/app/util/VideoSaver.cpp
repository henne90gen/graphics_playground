#include "VideoSaver.h"

#include <iostream>
#include <sstream>
#include <cmath>

#define GIF_FLIP_VERT
#include <gif.h>

#define VIDEO_TMP_FILE "tmp.h264"

const unsigned int SCALED_DOWN_WIDTH = 800;
const unsigned int SCALED_DOWN_HEIGHT = 600;

void VideoSaver::init(unsigned int frameWidth, unsigned int frameHeight) {
    this->frameWidth = frameWidth;
    this->frameHeight = frameHeight;
    if (!doInit()) {
        std::cout << "Could not initialze video saver" << std::endl;
        return;
    }
    initialized = true;
}

void VideoSaver::acceptFrame(const std::unique_ptr<Frame> &frame) {
    if (!isInitialized()) {
        init(frame->width, frame->height);
        if (!isInitialized()) {
            std::cout << "Failed to accept frame" << std::endl;
            return;
        }
    } else if (frame->width != frameWidth || frame->height != frameHeight) {
        // FIXME do something more sensible here
        std::cerr << "Resizing the video is not supported" << std::endl;
        exit(1);
    }

    doAcceptFrame(frame);
}
void VideoSaver::save() {
    if (!doSave()) {
        std::cerr << "Could not save video " << videoFileName << std::endl;
        return;
    }
    std::cout << "Saved video " << videoFileName << std::endl;
}

void Mp4VideoSaver::free() {
    if (videoFrame != nullptr) {
        av_frame_free(&videoFrame);
    }
    if (cctx != nullptr) {
        avcodec_free_context(&cctx);
    }
    if (ofctx != nullptr) {
        avformat_free_context(ofctx);
        ofctx = nullptr;
    }
    if (swsCtx != nullptr) {
        sws_freeContext(swsCtx);
        swsCtx = nullptr;
    }
}

void FreeRemux(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx) {
    if (ifmt_ctx != nullptr) {
        avformat_close_input(&ifmt_ctx);
    }
    if ((ofmt_ctx != nullptr) && ((ofmt_ctx->oformat->flags & AVFMT_NOFILE) == 0)) {
        avio_closep(&ofmt_ctx->pb);
    }
    if (ofmt_ctx != nullptr) {
        avformat_free_context(ofmt_ctx);
    }
}

void Mp4VideoSaver::remux() {
    AVFormatContext *ifmt_ctx = nullptr;
    AVFormatContext *ofmt_ctx = nullptr;
    int err = avformat_open_input(&ifmt_ctx, VIDEO_TMP_FILE, nullptr, nullptr);
    if (err < 0) {
        std::cout << "Failed to open input file for remuxing (" << err << ")" << std::endl;
        FreeRemux(ifmt_ctx, ofmt_ctx);
        return;
    }

    err = avformat_find_stream_info(ifmt_ctx, nullptr);
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

    if ((ofmt_ctx->oformat->flags & AVFMT_NOFILE) == 0) {
        if ((err = avio_open(&ofmt_ctx->pb, videoFileName.c_str(), AVIO_FLAG_WRITE)) < 0) {
            std::cout << "Failed to open output file (" << err << ")" << std::endl;
            FreeRemux(ifmt_ctx, ofmt_ctx);
            return;
        }
    }

    if ((err = avformat_write_header(ofmt_ctx, nullptr)) < 0) {
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

    FreeRemux(ifmt_ctx, ofmt_ctx);
}

bool Mp4VideoSaver::doInit() {
    int width = static_cast<int>(frameWidth);
    int height = static_cast<int>(frameHeight);

    oformat = av_guess_format(nullptr, VIDEO_TMP_FILE, nullptr);
    if (oformat == nullptr) {
        std::cout << "Failed to define output format" << std::endl;
        return false;
    }

    int err = avformat_alloc_output_context2(&ofctx, oformat, nullptr, VIDEO_TMP_FILE);
    if (err < 0) {
        std::cout << "Failed to allocate output context (" << err << ")" << std::endl;
        free();
        return false;
    }

    AVCodec *codec = avcodec_find_encoder(oformat->video_codec);
    if (codec == nullptr) {
        std::cout << "Failed to find encoder" << std::endl;
        free();
        return false;
    }

    AVStream *videoStream = avformat_new_stream(ofctx, codec);
    if (videoStream == nullptr) {
        std::cout << "Failed to create new stream" << std::endl;
        free();
        return false;
    }

    cctx = avcodec_alloc_context3(codec);
    if (cctx == nullptr) {
        std::cout << "Failed to allocate codec context" << std::endl;
        free();
        return false;
    }

    int bitrate = 0; // FIXME what is this?
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
    if ((ofctx->oformat->flags & AVFMT_GLOBALHEADER) != 0) {
        cctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    avcodec_parameters_from_context(videoStream->codecpar, cctx);

    if ((err = avcodec_open2(cctx, codec, nullptr)) < 0) {
        std::cout << "Failed to open codec (" << err << ")" << std::endl;
        free();
        return false;
    }

    if ((oformat->flags & AVFMT_NOFILE) == 0) {
        if ((err = avio_open(&ofctx->pb, VIDEO_TMP_FILE, AVIO_FLAG_WRITE)) < 0) {
            std::cout << "Failed to open file (" << err << ")" << std::endl;
            free();
            return false;
        }
    }

    if ((err = avformat_write_header(ofctx, nullptr)) < 0) {
        std::cout << "Failed to write header (" << err << ")" << std::endl;
        free();
        return false;
    }

    av_dump_format(ofctx, 0, VIDEO_TMP_FILE, 1);
    return true;
}

void Mp4VideoSaver::cleanUp() {
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
    if ((oformat->flags & AVFMT_NOFILE) == 0) {
        int err = avio_close(ofctx->pb);
        if (err < 0) {
            std::cout << "Failed to close file (" << err << ")" << std::endl;
        }
    }

    free();
}

void Mp4VideoSaver::doAcceptFrame(const std::unique_ptr<Frame> &frame) {
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
        if (frame->channels == 1) {
            format = AV_PIX_FMT_GRAY8;
        } else if (frame->channels == 2) {
            format = AV_PIX_FMT_GRAY8A;
        } else if (frame->channels == 3) {
            format = AV_PIX_FMT_RGB24;
        } else if (frame->channels == 4) {
            format = AV_PIX_FMT_RGBA;
        } else {
            std::cout << "Number of channels not supported (" << frame->channels << ")" << std::endl;
            free();
            return;
        }
        swsCtx = sws_getContext(cctx->width, cctx->height, format, cctx->width, cctx->height, AV_PIX_FMT_YUV420P,
                                SWS_BICUBIC, nullptr, nullptr, nullptr);
    }

    // Using a negative stride to flip the image with sws_scale
    int stride[1] = {-1 * static_cast<int>(frame->channels) * cctx->width};

    // Creating a pointer to the pointer that contains the actual start position.
    // The start position is at the end of the buffer, so that the image is flipped with sws_scale
    auto tmp = frame->buffer + (frame->channels * cctx->width * (cctx->height - 1));
    const auto *const *data = (const uint8_t *const *)&tmp;

    // From RGB to YUV
    sws_scale(swsCtx, data, stride, 0, cctx->height, videoFrame->data, videoFrame->linesize);

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
}

bool Mp4VideoSaver::doSave() {
    cleanUp();

    remux();

    return true;
}

GifVideoSaver::~GifVideoSaver() { delete gifWriter; }

auto GifVideoSaver::doInit() -> bool {
    if (gifWriter == nullptr) {
        gifWriter = new GifWriter();
    }
    unsigned int width = frameWidth;
    unsigned int height = frameHeight;
    if (scaleDown) {
        width = SCALED_DOWN_WIDTH;
        height = SCALED_DOWN_HEIGHT;
    }
    if (!GifBegin(gifWriter, videoFileName.c_str(), width, height, delay)) {
        std::cerr << "Could not open " << videoFileName << " for writing." << std::endl;
        return false;
    }
    return true;
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

void scaleDownFrame(Frame *frame, const unsigned int newWidth, const unsigned int newHeight) {
    auto buffer = static_cast<unsigned char *>(malloc(newWidth * newHeight * frame->channels * sizeof(unsigned char)));

    const unsigned int oldWidth = frame->width;
    const unsigned int oldHeight = frame->height;

    frame->width = newWidth;
    frame->height = newHeight;

    for (unsigned int y = 0; y < frame->height; y++) {
        for (unsigned int x = 0; x < frame->width; x++) {
            auto xF = static_cast<float>(x);
            auto yF = static_cast<float>(y);
            float downScaledX = xF / static_cast<float>(frame->width);
            float downScaledY = yF / static_cast<float>(frame->height);
            float upScaledX = downScaledX * static_cast<float>(oldWidth);
            float upScaledY = downScaledY * static_cast<float>(oldHeight);

            unsigned int xLeft = std::floor(upScaledX);
            unsigned int xRight = std::ceil(upScaledX);
            unsigned int yTop = std::floor(upScaledY);
            unsigned int yBottom = std::ceil(upScaledY);

            unsigned int index = (xLeft + yTop * oldWidth) * frame->channels;
            unsigned int topLeft = *reinterpret_cast<unsigned int *>(frame->buffer + index);
            index = (xRight + yTop * oldWidth) * frame->channels;
            unsigned int topRight = *reinterpret_cast<unsigned int *>(frame->buffer + index);

            index = (xLeft + yBottom * oldWidth) * frame->channels;
            unsigned int bottomLeft = *reinterpret_cast<unsigned int *>(frame->buffer + index);
            index = (xRight + yBottom * oldWidth) * frame->channels;
            unsigned int bottomRight = *reinterpret_cast<unsigned int *>(frame->buffer + index);

            unsigned int top;
            averagePixel(topLeft, topRight, &top);
            unsigned int bottom;
            averagePixel(bottomLeft, bottomRight, &bottom);

            unsigned int baseIndex = (x + y * frame->width) * frame->channels;
            auto *pixel = reinterpret_cast<unsigned int *>(buffer + baseIndex);
            averagePixel(top, bottom, pixel);
        }
    }

    free(frame->buffer); // NOLINT(cppcoreguidelines-no-malloc)
    frame->buffer = buffer;
}

void GifVideoSaver::doAcceptFrame(const std::unique_ptr<Frame> &frame) {
    if (scaleDown) {
        scaleDownFrame(frame.get(), SCALED_DOWN_WIDTH, SCALED_DOWN_HEIGHT);
    }
    GifWriteFrame(gifWriter, frame->buffer, frame->width, frame->height, delay);
}

bool GifVideoSaver::doSave() { return GifEnd(gifWriter); }
