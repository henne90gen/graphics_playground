#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

int main(int argc, char *argv[]) {
    return Catch::Session().run(argc, argv);
}

#include "WavLoader.h"

TEST_CASE("WavReader can read simple wav file") {
    WavFile wav = {};
    REQUIRE(loadWavFile("../../src/test/audio_test.wav", wav));
    REQUIRE(wav.header.numChannels == 2);
    REQUIRE(wav.header.sampleRate == 8000);
    REQUIRE(wav.header.bitsPerSample == 16);
}
