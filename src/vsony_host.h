#ifndef VSONY_HOST_H
#define VSONY_HOST_H
#include <stdint.h>
#include <stddef.h>
struct vsony_host;
struct vsony_host *vsony_open(const char *path, uint64_t bytes);
int vsony_read(struct vsony_host *, uint64_t off, void *buf, size_t n);
int vsony_write(struct vsony_host *, uint64_t off, const void *buf, size_t n);
uint64_t vsony_size(const struct vsony_host *);
void vsony_close(struct vsony_host *);
#endif
