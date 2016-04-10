#include "stm32f1xx_hal.h"

#define OW_0	                0x00
#define OW_1	                0xff
#define OW_R_1	                0xff

#define OW_OK			1
#define OW_ERROR		2
#define OW_NO_DEVICE	        3

#define OW_SEND_RESET           1
#define OW_NO_RESET             2

#define OW_NO_READ		0xff

#define OW_READ_ROM             0x33
#define OW_CONVERT_T            0x44
#define OW_MATCH_ROM            0x55
#define OW_READ_SCRATCHPAD      0xbe
#define OW_SKIP_ROM             0xcc
#define OW_SEARCH_ROM           0xf0
#define OW_READ_SLOT            0xff

#define RTC_ASYNCH_PREDIV    0x7F
#define RTC_SYNCH_PREDIV     0x0130

#define SLEEP                   1
#define TIMER                   2
#define ERROR                   3
#define WORK                    4


uint8_t OW_Send(uint8_t sendReset, uint8_t *command, uint16_t cLen, uint8_t *data, uint16_t dLen, uint8_t readStart);
uint8_t OW_Reset();
uint8_t OW_Scan(uint8_t *buf, uint8_t num);
void OW_SendBits(uint8_t num_bits);
void OW_Massage(uint8_t flag);
void OW_Output(void);
void OW_toBits(uint8_t byte, uint8_t *bits);
uint8_t OW_toByte(uint8_t *bits);