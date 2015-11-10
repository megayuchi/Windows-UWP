#pragma once

int ReadStreamFunc(void* ptr, uint8_t* buf, int buf_size);
int64_t SeekStreamFunc(void* ptr, int64_t pos, int whence);