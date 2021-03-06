/*
 * Copyright 2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#ifndef HEADER_INTERNAL_REFCOUNT_H
# define HEADER_INTERNAL_REFCOUNT_H

# if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
# include <stdatomic.h>
# define HAVE_C11_ATOMICS
# endif

# if defined(HAVE_C11_ATOMICS) && ATOMIC_INT_LOCK_FREE > 0

# define HAVE_ATOMICS 1

typedef _Atomic int CRYPTO_REF_COUNT;

static ossl_inline int CRYPTO_UP_REF(_Atomic int *val, int *ret, void *lock)
{
    *ret = atomic_fetch_add_explicit(val, 1, memory_order_relaxed) + 1;
    return 1;
}

static ossl_inline int CRYPTO_DOWN_REF(_Atomic int *val, int *ret, void *lock)
{
    *ret = atomic_fetch_sub_explicit(val, 1, memory_order_release) - 1;
    if (*ret == 0)
        atomic_thread_fence(memory_order_acquire);
    return 1;
}

# elif defined(__GNUC__) && defined(__ATOMIC_RELAXED) && __GCC_ATOMIC_INT_LOCK_FREE > 0

# define HAVE_ATOMICS 1

typedef int CRYPTO_REF_COUNT;

static ossl_inline int CRYPTO_UP_REF(int *val, int *ret, void *lock)
{
    *ret = __atomic_fetch_add(val, 1, __ATOMIC_RELAXED) + 1;
    return 1;
}

static ossl_inline int CRYPTO_DOWN_REF(int *val, int *ret, void *lock)
{
    *ret = __atomic_fetch_sub(val, 1, __ATOMIC_RELEASE) - 1;
    if (*ret == 0)
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
    return 1;
}

# else

typedef int CRYPTO_REF_COUNT;

# define CRYPTO_UP_REF(val, ret, lock) CRYPTO_atomic_add(val, 1, ret, lock)
# define CRYPTO_DOWN_REF(val, ret, lock) CRYPTO_atomic_add(val, -1, ret, lock)

# endif
#endif
