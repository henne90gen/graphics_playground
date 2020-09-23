#include <catch.hpp>

#include "audio/WavLoader.h"

TEST_CASE("WavReader can read simple wav file") {
    WavFile wav = {};
    REQUIRE(loadWavFile("../../src/test/audio_test.wav", wav));
    REQUIRE(wav.header.numChannels == 2);
    REQUIRE(wav.header.sampleRate == 8000);
}
