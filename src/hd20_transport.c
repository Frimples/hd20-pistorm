#define _POSIX_C_SOURCE 200809L
#include "hd20_transport.h"
#include "vsony_host.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define INMAX 4096
#define OUTMAX 65536
#define HD20_BYTE_NS 34722ULL
struct hd20_transport{struct vsony_host*d;enum hd20_phase p;uint8_t in[INMAX],raw[8],out[OUTMAX];size_t in_n,raw_n,out_n,out_pos;int have_sync;uint8_t length_bytes;uint64_t next_ns;};
static uint64_t now_ns(void){struct timespec x;clock_gettime(CLOCK_MONOTONIC,&x);return (uint64_t)x.tv_sec*1000000000ULL+(uint64_t)x.tv_nsec;}
static void pace(struct hd20_transport*t){uint64_t n=now_ns();if(!t->next_ns)t->next_ns=n;if(n<t->next_ns){struct timespec x={(time_t)((t->next_ns-n)/1000000000ULL),(long)((t->next_ns-n)%1000000000ULL)};nanosleep(&x,NULL);}t->next_ns+=HD20_BYTE_NS;}
static void finish(struct hd20_transport*t){uint8_t r[65536];size_t n=0;int z=hd20_process_command(t->in,t->in_n,t->d,r,sizeof r,&n);if(getenv("HD20_TRACE")){fprintf(stderr,"[HD20] cmd_len=%zu result=%d",t->in_n,z);for(size_t i=0;i<t->in_n&&i<16;i++)fprintf(stderr," %02x",t->in[i]);if(t->in_n>=6)fprintf(stderr," block=%u",((unsigned)t->in[3]<<16)|((unsigned)t->in[4]<<8)|t->in[5]);fprintf(stderr," response_len=%zu first",n);for(size_t i=0;i<n&&i<16;i++)fprintf(stderr," %02x",r[i]);if(n>=42){fprintf(stderr," data");for(size_t i=26;i<42;i++)fprintf(stderr," %02x",r[i]);}if(n>0)fprintf(stderr," chk=%02x verify=%02x",r[n-1],hd20_checksum(r,n-1));fprintf(stderr,"\\n");}if(z==0){if(n+1<OUTMAX){t->out[0]=HD20_SYNC_READ;t->out[1]=HD20_SYNC_READ;size_t e=hd20_encode7(r,n,t->out+2,OUTMAX-3);t->out[e+2]=0;t->out_n=e?e+3:0;t->out_pos=0;}}t->in_n=t->raw_n=0;t->have_sync=0;}
struct hd20_transport*hd20_transport_create(struct vsony_host*d){struct hd20_transport*t=calloc(1,sizeof*t);if(t){t->d=d;t->p=HD20_P2;}return t;}
void hd20_transport_destroy(struct hd20_transport*t){free(t);}
void hd20_transport_reset(struct hd20_transport*t){if(t){t->p=HD20_P4;t->in_n=t->raw_n=t->out_n=t->out_pos=0;t->have_sync=0;t->length_bytes=0;t->next_ns=0;}}
void hd20_transport_phase(struct hd20_transport*t,enum hd20_phase p){if(!t)return;if((t->p==HD20_P1||t->p==HD20_P3)&&p==HD20_P2&&t->in_n)finish(t);t->p=p;if(p==HD20_P4){t->in_n=t->raw_n=0;t->have_sync=0;t->length_bytes=0;}}
int hd20_transport_host_byte(struct hd20_transport*t,uint8_t b){if(!t||(t->p!=HD20_P1&&t->p!=HD20_P3))return -1;if(!t->have_sync){if(b!=HD20_SYNC_READ&&b!=HD20_SYNC_WRITE)return -1;t->have_sync=1;t->in[t->in_n++]=b;t->length_bytes=4;return 0;}if(t->length_bytes){t->length_bytes--;return 0;}if(t->raw_n<8)t->raw[t->raw_n++]=b;if(t->raw_n==8){int n=hd20_decode7(t->raw,8,t->in+t->in_n,INMAX-t->in_n);if(n<0)return n;t->in_n+=(size_t)n;t->raw_n=0;if(t->in_n>=8)finish(t);}return 0;}
int hd20_transport_device_ready(const struct hd20_transport*t){return t&&t->out_pos<t->out_n;}
int hd20_transport_device_byte(struct hd20_transport*t,uint8_t*b){if(!t||!b||t->out_pos>=t->out_n)return -1;pace(t);*b=t->out[t->out_pos++];if(getenv("HD20_TRACE")&&t->out_pos==t->out_n)fprintf(stderr,"[HD20] response_fifo_complete bytes=%zu\\n",t->out_n);return 0;}
