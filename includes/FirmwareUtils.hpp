#ifndef __FIRMWARE_UTILS_HPP__
#define __FIRMWARE_UTILS_HPP__

// Tells you how long a c type array that was static declared with emtpy [] length
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))


// Pointer to device unique id
// see http://blog.gorski.pm/stm32-unique-id

#define DEV_ID0 (*(uint32_t *)0x1FFFF7AC)
#define DEV_ID1 (*(uint32_t *)0x1FFFF7B0)
#define DEV_ID2 (*(uint32_t *)0x1FFFF7B4)

#endif