#pragma once

template <typename T, unsigned long N>
char(&ArrayLength(T (&array)[N]))[N];

#define ARRAY_LENGTH(array) (sizeof(ArrayLength(array)))