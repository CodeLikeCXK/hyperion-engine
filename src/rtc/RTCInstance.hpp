/* Copyright (c) 2024 No Tomorrow Games. All rights reserved. */
#ifndef HYPERION_RTC_INSTANCE_HPP
#define HYPERION_RTC_INSTANCE_HPP

#include <rtc/RTCClientList.hpp>
#include <rtc/RTCServer.hpp>
#include <rtc/RTCTrack.hpp>
#include <rtc/RTCStream.hpp>
#include <rtc/RTCStreamEncoder.hpp>

#include <core/memory/RefCountedPtr.hpp>
#include <core/memory/UniquePtr.hpp>

namespace hyperion {

class HYP_API RTCInstance
{
public:
    RTCInstance(RTCServerParams serverParams);
    RTCInstance(const RTCInstance& other) = delete;
    RTCInstance& operator=(const RTCInstance& other) = delete;
    RTCInstance(RTCInstance&& other) = delete;
    RTCInstance& operator=(RTCInstance&& other) = delete;
    ~RTCInstance() = default;

    const RC<RTCServer>& GetServer() const
    {
        return m_server;
    }

    RC<RTCTrackBase> CreateTrack(RTCTrackType trackType);
    RC<RTCStream> CreateStream(RTCStreamType streamType, UniquePtr<RTCStreamEncoder>&& encoder);

private:
    RC<RTCServer> m_server;
};

} // namespace hyperion

#endif