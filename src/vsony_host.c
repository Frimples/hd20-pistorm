#define _FILE_OFFSET_BITS 64
#define _POSIX_C_SOURCE 200809L
#include "vsony_host.h"
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#define VSONY_MAX_BYTES (2ULL*1024ULL*1024ULL*1024ULL)
struct vsony_host { int fd; uint64_t size; };
struct vsony_host *vsony_open(const char *p,uint64_t bytes){struct vsony_host*h;struct stat st;if(!p)return 0;h=calloc(1,sizeof*h);if(!h)return 0;h->fd=open(p,O_RDWR|O_CREAT,0666);if(h->fd<0){free(h);return 0;}if(bytes==0){if(fstat(h->fd,&st)<0){close(h->fd);free(h);return 0;}bytes=(uint64_t)st.st_size;}if(!bytes||bytes>VSONY_MAX_BYTES||(bytes%512)){close(h->fd);free(h);return 0;}h->size=bytes;if(ftruncate(h->fd,(off_t)bytes)<0){close(h->fd);free(h);return 0;}return h;}
static int range(const struct vsony_host*h,uint64_t o,size_t n){return o<=h->size && (uint64_t)n<=h->size-o;}
int vsony_read(struct vsony_host*h,uint64_t o,void*b,size_t n){if(!h||!range(h,o,n))return -1;return pread(h->fd,b,n,(off_t)o)==(ssize_t)n?0:-1;}
int vsony_write(struct vsony_host*h,uint64_t o,const void*b,size_t n){if(!h||!range(h,o,n))return -1;return pwrite(h->fd,b,n,(off_t)o)==(ssize_t)n?0:-1;}
uint64_t vsony_size(const struct vsony_host*h){return h?h->size:0;}
void vsony_close(struct vsony_host*h){if(h){if(h->fd>=0)close(h->fd);free(h);}}
