/* Copyright (c) 2024 No Tomorrow Games. All rights reserved. */

#include <core/threading/ThreadId.hpp>
#include <core/threading/Threads.hpp>
#include <core/threading/Mutex.hpp>

#include <core/containers/HashMap.hpp>

#include <core/utilities/GlobalContext.hpp>
#include <core/utilities/IdGenerator.hpp>

#include <core/Defines.hpp>

namespace hyperion {
namespace threading {

const ThreadId ThreadId::invalid = ThreadId();

class GlobalThreadIdCache
{
public:
    GlobalThreadIdCache()
    {
    }

    ~GlobalThreadIdCache()
    {
    }

    Name FindNameByIndex(uint32 index)
    {
        Mutex::Guard guard(m_mutex);

        auto it = m_reverseNameMapping.Find(index);

        if (it == m_reverseNameMapping.End())
        {
            return Name();
        }

        return it->second;
    }

    uint32 AllocateIndex(Name name)
    {
        const uint32 index = m_idGenerator.Next();

        {
            Mutex::Guard guard(m_mutex);

            m_nameMapping[name].PushBack(index);
            m_reverseNameMapping[index] = name;
        }

        return index;
    }

    uint32 FindOrAllocateIndex(Name name)
    {
        Mutex::Guard guard(m_mutex);

        if (uint32* indexPtr = FindIndexByName_Internal(name))
        {
            return *indexPtr;
        }

        const uint32 index = m_idGenerator.Next();

        m_nameMapping[name].PushBack(index);
        m_reverseNameMapping[index] = name;

        return index;
    }

private:
    uint32* FindIndexByName_Internal(Name name)
    {
        auto it = m_nameMapping.Find(name);

        if (it != m_nameMapping.End() && it->second.Any())
        {
            return &it->second.Front();
        }

        return nullptr;
    }

    IdGenerator m_idGenerator;

    HashMap<Name, Array<uint32>> m_nameMapping;
    HashMap<uint32, Name> m_reverseNameMapping;

    mutable Mutex m_mutex;
};

static GlobalThreadIdCache& GetStaticThreadIdCache()
{
    static GlobalThreadIdCache cache;

    return cache;
}

static GlobalThreadIdCache& GetDynamicThreadIdCache()
{
    static GlobalThreadIdCache cache;

    return cache;
}

#pragma region ThreadId

static uint32 AllocateThreadId(Name name, uint32 allocateFlags)
{
    uint32 threadIdValue;

    if (allocateFlags & ThreadId::AllocateFlags::DYNAMIC)
    {
        threadIdValue = (allocateFlags & ThreadId::AllocateFlags::FORCE_UNIQUE)
            ? GetDynamicThreadIdCache().AllocateIndex(name)
            : GetDynamicThreadIdCache().FindOrAllocateIndex(name);
    }
    else
    {
        threadIdValue = (allocateFlags & ThreadId::AllocateFlags::FORCE_UNIQUE)
            ? GetStaticThreadIdCache().AllocateIndex(name)
            : GetStaticThreadIdCache().FindOrAllocateIndex(name);

        AssertThrowMsg(threadIdValue < g_maxStaticThreadIds, "Maximum static thread id value exceeded!");

        threadIdValue = 1u << (threadIdValue - 1);
    }

    AssertThrowMsg((((threadIdValue << 4) & g_threadIdMask) >> 4) == threadIdValue,
        "Thread Id value %u exceeds maximum value!",
        threadIdValue);

    return threadIdValue;
}

static uint32 MakeThreadIdValue(Name name, ThreadCategory category, uint32 allocateFlags)
{
    uint32 value = 0;

    value |= (uint32(category) & g_threadCategoryMask);
    value |= (AllocateThreadId(name, allocateFlags) << 4) & g_threadIdMask;
    value |= ((allocateFlags & ThreadId::AllocateFlags::DYNAMIC) ? 1u : 0u) << 31;

    return value;
}

const ThreadId& ThreadId::Current()
{
    return Threads::CurrentThreadId();
}

const ThreadId& ThreadId::Invalid()
{
    return invalid;
}

ThreadId::ThreadId(Name name, bool forceUnique)
    : ThreadId(name, THREAD_CATEGORY_NONE, forceUnique)
{
}

ThreadId::ThreadId(Name name, ThreadCategory category, bool forceUnique)
    : ThreadId(
          name,
          category,
          AllocateFlags::DYNAMIC
              | (forceUnique ? AllocateFlags::FORCE_UNIQUE : AllocateFlags::NONE))
{
}

ThreadId::ThreadId(Name name, ThreadCategory category, uint32 allocateFlags)
    : m_name(name),
      m_value(MakeThreadIdValue(name, category, allocateFlags))
{
}

#pragma endregion ThreadId

#pragma region StaticThreadId

StaticThreadId::StaticThreadId(Name name, bool forceUnique)
    : ThreadId(name, THREAD_CATEGORY_NONE, forceUnique ? AllocateFlags::FORCE_UNIQUE : AllocateFlags::NONE)
{
}

StaticThreadId::StaticThreadId(uint32 staticThreadIndex)
{
    m_name = GetStaticThreadIdCache().FindNameByIndex(staticThreadIndex + 1);
    m_value = ((1u << staticThreadIndex) << 4) & g_threadIdMask;
}

#pragma endregion StaticThreadId

} // namespace threading
} // namespace hyperion