#include "VideoSaver.h"

#include "ScreenRecorder.h"

#include <iostream>

#define VIDEO_TMP_FILE "tmp.h264"

void VideoSaver::free() {
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

void VideoSaver::remux() {
    AVFormatContext *ifmt_ctx = nullptr, *ofmt_ctx = nullptr;
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

void VideoSaver::init() {
    int width = (int)video->getWidth();
    int height = (int)video->getHeight();

    oformat = av_guess_format(nullptr, VIDEO_TMP_FILE, nullptr);
    if (oformat == nullptr) {
        std::cout << "Failed to define output format" << std::endl;
        return;
    }

    int err = avformat_alloc_output_context2(&ofctx, oformat, nullptr, VIDEO_TMP_FILE);
    if (err < 0) {
        std::cout << "Failed to allocate output context (" << err << ")" << std::endl;
        free();
        return;
    }

    AVCodec *codec = avcodec_find_encoder(oformat->video_codec);
    if (codec == nullptr) {
        std::cout << "Failed to find encoder" << std::endl;
        free();
        return;
    }

    AVStream *videoStream = avformat_new_stream(ofctx, codec);
    if (videoStream == nullptr) {
        std::cout << "Failed to create new stream" << std::endl;
        free();
        return;
    }

    cctx = avcodec_alloc_context3(codec);
    if (cctx == nullptr) {
        std::cout << "Failed to allocate codec context" << std::endl;
        free();
        return;
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
    if (ofctx->oformat->flags & AVFMT_GLOBALHEADER) {
        cctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    avcodec_parameters_from_context(videoStream->codecpar, cctx);

    if ((err = avcodec_open2(cctx, codec, nullptr)) < 0) {
        std::cout << "Failed to open codec (" << err << ")" << std::endl;
        free();
        return;
    }

    if (!(oformat->flags & AVFMT_NOFILE)) {
        if ((err = avio_open(&ofctx->pb, VIDEO_TMP_FILE, AVIO_FLAG_WRITE)) < 0) {
            std::cout << "Failed to open file (" << err << ")" << std::endl;
            free();
            return;
        }
    }

    if ((err = avformat_write_header(ofctx, nullptr)) < 0) {
        std::cout << "Failed to write header (" << err << ")" << std::endl;
        free();
        return;
    }
    av_dump_format(ofctx, 0, VIDEO_TMP_FILE, 1);
}



void VideoSaver::process() {
    int frameCounter = 0;
    std::function<void(Frame *)> workFunction = [this, &frameCounter](Frame *currentFrame) {
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
              free();
              return;
          }
          swsCtx = sws_getContext(cctx->width, cctx->height, format, cctx->width, cctx->height, AV_PIX_FMT_YUV420P,
                                  SWS_BICUBIC, nullptr, nullptr, nullptr);
      }

      // Using a negative stride to flip the image with sws_scale
      int stride[1] = {-1 * (int)currentFrame->channels * cctx->width};

      // Creating a pointer to the pointer that contains the actual start position.
      // The start position is at the end of the buffer, so that the image is flipped with sws_scale
      auto tmp = currentFrame->buffer + (currentFrame->channels * cctx->width * (cctx->height - 1));
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
    };
    video->iterateFrames(workFunction);
}

void VideoSaver::cleanUp() {
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

    free();
}

void VideoSaver::run() {
    init();

    process();

    cleanUp();

    remux();
}

VideoSaver::VideoSaver(Video *video, const std::string &videoFileName) : video(video), videoFileName(videoFileName) {}
