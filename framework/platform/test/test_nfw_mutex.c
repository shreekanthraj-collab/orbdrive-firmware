#include "unity.h"
#include "nfw_mutex.h"

void setUp(void)
{
}

void tearDown(void)
{
}

TEST_CASE("Mutex can be created and deleted", "[nfw_mutex]")
{
    NfwMutex_t mutex = nfwMutexCreate();

    TEST_ASSERT_NOT_NULL(mutex);

    nfwMutexDelete(mutex);
}

TEST_CASE("Mutex can be locked and unlocked", "[nfw_mutex]")
{
    NfwMutex_t mutex = nfwMutexCreate();

    TEST_ASSERT_NOT_NULL(mutex);

    TEST_ASSERT_TRUE(nfwMutexLock(mutex));

    nfwMutexUnlock(mutex);
    nfwMutexDelete(mutex);
}

TEST_CASE("NULL mutex operations are handled safely", "[nfw_mutex]")
{
    TEST_ASSERT_FALSE(nfwMutexLock(NULL));

    nfwMutexUnlock(NULL);
    nfwMutexDelete(NULL);
}