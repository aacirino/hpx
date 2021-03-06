////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2014 Peter Dimov
//  Copyright (c) 2020 Agustin Berge
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

// see https://rigtorp.se/spinlock/

#pragma once

#include <hpx/config.hpp>

#include <atomic>

namespace hpx { namespace util { namespace detail {

    /// Lockable spinlock class
    struct spinlock
    {
    public:
        HPX_NON_COPYABLE(spinlock);

    private:
        std::atomic<bool> m;

        HPX_CORE_EXPORT void yield_k(unsigned) noexcept;

    public:
        spinlock() noexcept
          : m(false)
        {
        }

        bool try_lock() noexcept
        {
            // First do a relaxed load to check if lock is free in order to prevent
            // unnecessary cache misses if someone does while(!try_lock())
            return !m.load(std::memory_order_relaxed) &&
                !m.exchange(true, std::memory_order_acquire);
        }

        void lock() noexcept
        {
            // Optimistically assume the lock is free on the first try
            if (!m.exchange(true, std::memory_order_acquire))
                return;

            // Wait for lock to be released without generating cache misses
            unsigned k = 0;
            do
            {
                yield_k(k++);
            } while (!try_lock());
        }

        void unlock() noexcept
        {
            m.store(false, std::memory_order_release);
        }
    };

}}}    // namespace hpx::util::detail
