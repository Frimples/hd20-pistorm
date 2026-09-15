#ifndef HD20_PROTOCOL_H
#define HD20_PROTOCOL_H
#include <stddef.h>
#include <stdint.h>
#define HD20_BLOCK_SIZE 512u
#define HD20_MAX_BLOCKS 0x1000000ULL
#define HD20_SYNC_READ 0xAAu
#define HD20_SYNC_WRITE 0x96u
#define HD20_CMD_READ 0x00u
#define HD20_CMD_WRITE 0x01u
#define HD20_CMD_WRITE_VERIFY 0x02u
#define HD20_CMD_STATUS 0x03u
#define HD20_OK 0
#define HD20_ERR_FORMAT -1
#define HD20_ERR_RANGE -2
#define HD20_ERR_IO -3
size_t hd20_encode7(const uint8_t *in,size_t n,uint8_t *out,size_t cap);
int hd20_decode7(const uint8_t *in,size_t n,uint8_t *out,size_t cap);
uint8_t hd20_checksum(const uint8_t *p,size_t n);
int hd20_build_status(uint8_t *out,size_t cap,uint32_t blocks);
struct vsony_host;
int hd20_process_command(const uint8_t *cmd,size_t cmd_n,struct vsony_host *disk,uint8_t *resp,size_t cap,size_t *resp_n);
#endif
