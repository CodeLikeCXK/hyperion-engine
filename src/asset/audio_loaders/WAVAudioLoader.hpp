/* Copyright (c) 2024 No Tomorrow Games. All rights reserved. */

#pragma once

#include <asset/Assets.hpp>

#include <core/memory/ByteBuffer.hpp>

namespace hyperion {

enum class AudioSourceFormat : uint32;

HYP_CLASS(NoScriptBindings)
class WAVAudioLoader : public AssetLoaderBase
{
    HYP_OBJECT_BODY(WAVAudioLoader);

public:
    struct WAVAudio
    {
        struct RiffHeader
        {
            uint8 chunkId[4];
            uint32 chunkSize;
            uint8 format[4];
        } riffHeader;

        struct WaveFormat
        {
            uint8 subChunkId[4];
            uint32 subChunkSize;
            uint16 audioFormat;
            uint16 numChannels;
            uint32 sampleRate;
            uint32 byteRate;
            uint16 blockAlign;
            uint16 bitsPerSample;
        } waveFormat;

        struct WaveData
        {
            uint8 subChunkId[4];
            uint32 subChunk2Size;
        } waveData;

        ByteBuffer waveBytes;
        SizeType size;
        SizeType frequency;

        AudioSourceFormat format;
    };

    virtual ~WAVAudioLoader() = default;

    virtual AssetLoadResult LoadAsset(LoaderState& state) const override;
};

} // namespace hyperion

