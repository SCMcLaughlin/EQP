
#include "random.h"

void random_bytes(void* buffer, int count)
{
    sqlite3_randomness(count, (byte*)buffer);
}

uint16_t random_uint16()
{
    uint16_t ret;
    random_bytes(&ret, sizeof(uint16_t));
    return ret;
}

uint32_t random_uint32()
{
    uint32_t ret;
    random_bytes(&ret, sizeof(uint32_t));
    return ret;
}

uint64_t random_uint64()
{
    uint64_t ret;
    random_bytes(&ret, sizeof(uint64_t));
    return ret;
}

float random_float()
{
    float ret;
    random_bytes(&ret, sizeof(float));
    return ret;
}

int random_chance(float percent)
{
    return (fmod(random_float(), 100.0f) < percent);
}
