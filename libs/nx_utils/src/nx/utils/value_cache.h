// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <optional>
#include <chrono>

#include <nx/utils/move_only_func.h>
#include <nx/utils/thread/mutex.h>
#include <nx/utils/elapsed_timer.h>

namespace nx::utils {

/**
 *  Allows caching of the value and automatic cache invalidation. Methods get(), reset() and
 *  update() can be called safely from different threads.
 */
template<class ValueType>
class CachedValue
{
public:
    /**
     *  @param valueGenerator This functor is called from get() and update() to get value, it
     *      must be thread-safe.
     *  @param expirationTime CachedValue will automatically update value on get() or
     *      update() every expirationTime milliseconds. Setting to 0 ms disables expiration.
     *  @note valueGenerator is not called here!
     */
    CachedValue(
        MoveOnlyFunc<ValueType()> valueGenerator,
        std::chrono::milliseconds expirationTime = std::chrono::milliseconds::zero())
    :
        m_valueGenerator(std::move(valueGenerator)),
        m_expirationTime(expirationTime)
    {
    }

    ValueType get() const
    {
        {
            NX_MUTEX_LOCKER lock(&m_mutex);
            if (m_value.has_value() && (m_expirationTime == std::chrono::milliseconds::zero()
                    || !m_timer.hasExpired(m_expirationTime)))
            return *m_value;
        }
        ValueType value = m_valueGenerator();
        {
            NX_MUTEX_LOCKER lock(&m_mutex);
            m_value = std::move(value);
            m_timer.restart();
            return *m_value;
        }
    }

    void reset()
    {
        NX_MUTEX_LOCKER lock(&m_mutex);
        m_value.reset();
    }

    void update()
    {
        ValueType value = m_valueGenerator();
        {
            NX_MUTEX_LOCKER lock(&m_mutex);
            m_value = std::move(value);
            m_timer.restart();
        }
    }

private:
    mutable nx::Mutex m_mutex;

    mutable std::optional<ValueType> m_value;
    const MoveOnlyFunc<ValueType()> m_valueGenerator;

    mutable ElapsedTimer m_timer;
    const std::chrono::milliseconds m_expirationTime;
};

//-------------------------------------------------------------------------------------------------

/**
 * Caches a value and updates its value periodically from a given function.
 * The class is not thread-safe!
 */
template<typename Value, typename StorageType = Value>
class ValueCacheUnSafe
{
public:
    ValueCacheUnSafe(
        nx::utils::MoveOnlyFunc<Value()> generator,
        std::chrono::milliseconds valueUpdatePeriod)
        :
        m_generator(std::move(generator)),
        m_valueUpdatePeriod(valueUpdatePeriod)
    {
    }

    virtual ~ValueCacheUnSafe() = default;

    Value get()
    {
        const auto now = std::chrono::steady_clock::now();
        if (now - m_lastUpdateTime.load() > m_valueUpdatePeriod)
            generateValue();

        return m_value;
    }

protected:
    virtual void generateValue()
    {
        m_value = m_generator();
        m_lastUpdateTime.store(std::chrono::steady_clock::now());
    }

private:
    nx::utils::MoveOnlyFunc<Value()> m_generator;
    const std::chrono::milliseconds m_valueUpdatePeriod;
    std::atomic<std::chrono::steady_clock::time_point> m_lastUpdateTime;
    StorageType m_value = {};
};

//-------------------------------------------------------------------------------------------------

/**
 * Caches a value and updates its value periodically from a given function.
 * Locks internal mutex when value update is needed only.
 * So, this class is suitable for multithreaded environment with a lot of concurrent value access.
 * Value type must be suitable for std::atomic<Value>.
 */
template<typename Value>
class AtomicValueCache:
    public ValueCacheUnSafe<Value, std::atomic<Value>>
{
    using base_type = ValueCacheUnSafe<Value, std::atomic<Value>>;

public:
    using base_type::base_type;

protected:
    virtual void generateValue() override
    {
        NX_MUTEX_LOCKER lock(&m_mutex);
        base_type::generateValue();
    }

private:
    nx::Mutex m_mutex;
};

} // namespace nx::utils

