#include "unity.h"
#include "nfw_memory.h"

void setUp(void)
{
}

void tearDown(void)
{
}

TEST_CASE("Memory allocation returns a usable block", "[nfw_memory]")
{
    unsigned char *buffer = (unsigned char *)nfwMalloc(16U);

    TEST_ASSERT_NOT_NULL(buffer);

    for (size_t i = 0U; i < 16U; i++)
    {
        buffer[i] = (unsigned char)i;
    }

    for (size_t i = 0U; i < 16U; i++)
    {
        TEST_ASSERT_EQUAL_UINT8((unsigned char)i, buffer[i]);
    }

    nfwFree(buffer);
}

TEST_CASE("Freeing NULL is safe", "[nfw_memory]")
{
    nfwFree(NULL);
}