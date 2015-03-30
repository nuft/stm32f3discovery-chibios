#ifndef NRF24L01P_H
#define NRF24L01P_H

#include <stdint.h>
#include <hal.h>

// Registers
#define CONFIG              0x00
#define EN_AA               0x01
#define EN_RXADDR           0x02
#define SETUP_AW            0x03
#define SETUP_RETR          0x04
#define RF_CH               0x05
#define RF_SETUP            0x06
#define STATUS              0x07
#define OBSERVE_TX          0x08
#define RPD                 0x09
#define RX_ADDR_P0          0x0A // 5 bytes
#define RX_ADDR_P1          0x0B // 5 bytes
#define RX_ADDR_P2          0x0C
#define RX_ADDR_P3          0x0D
#define RX_ADDR_P4          0x0E
#define RX_ADDR_P5          0x0F
#define TX_ADDR             0x10 // 5 bytes
#define RX_PW_P0            0x11
#define RX_PW_P1            0x12
#define RX_PW_P2            0x13
#define RX_PW_P3            0x14
#define RX_PW_P4            0x15
#define RX_PW_P5            0x16
#define FIFO_STATUS         0x17
#define DYNPD               0x1C
#define FEATURE             0x1D

// CONFIG
#define MASK_RX_DR  (1<<6)
#define MASK_TX_DS  (1<<5)
#define MASK_MAX_RT (1<<4)
#define EN_CRC      (1<<3)
#define CRCO        (1<<2)
#define PWR_UP      (1<<1)
#define PRIM_RX     (1<<0)

// EN_AA
#define ENAA_P5     (1<<5)
#define ENAA_P4     (1<<4)
#define ENAA_P3     (1<<3)
#define ENAA_P2     (1<<2)
#define ENAA_P1     (1<<1)
#define ENAA_P0     (1<<0)

// EN_RXADDR
#define ERX_P5      (1<<5)
#define ERX_P4      (1<<4)
#define ERX_P3      (1<<3)
#define ERX_P2      (1<<2)
#define ERX_P1      (1<<1)
#define ERX_P0      (1<<0)

// SETUP_AW
#define AW(x)       ((x)<<0)    // 1:0
#define AW_3        0x01
#define AW_4        0x02
#define AW_5        0x03

// SETUP_RETR
#define ARD(x)      ((x)<<4)    // 7:4
#define ARC(x)      ((x)<<0)    // 3:0

// RF_SETUP
#define CONT_WAVE   (1<<7)
#define RF_DR_LOW   (1<<5)
#define PLL_LOCK    (1<<4)
#define RF_DR_HIGH  (1<<3)
#define RF_PWR(x)   ((x)<<1)    // 2:1
#define RF_DR_1M    0x00
#define RF_DR_2M    0x08
#define RF_DR_250K  0x20

// STATUS
#define RX_DR           (1<<6)
#define TX_DS           (1<<5)
#define MAX_RT          (1<<4)
#define RX_P_NO         (0x7<<1)    // 3:1
#define GET_RX_P_NO(x)  (((x)>>1) & 0x7)
#define TX_FULL         (1<<0)

// OBSERVE_TX
#define PLOS_CNT(x) (((x)>>4) & 0xF)    // 7:4
#define ARC_CNT(x)  (((x)>>0) & 0xF)    // 3:0

// FIFO_STATUS
#define FIFO_TX_REUSE   (1<<6)
#define FIFO_TX_FULL    (1<<5)
#define FIFO_TX_EMPTY   (1<<4)
#define FIFO_RX_FULL    (1<<1)
#define FIFO_RX_EMPTY   (1<<0)

// DYNPD
#define DPL_P5      (1<<5)
#define DPL_P4      (1<<4)
#define DPL_P3      (1<<3)
#define DPL_P2      (1<<2)
#define DPL_P1      (1<<1)
#define DPL_P0      (1<<0)

// FEATURE
#define EN_DPL      (1<<2)
#define EN_ACK_PAY  (1<<1)
#define EN_DYN_ACK  (1<<0)

typedef struct {
    SPIDriver *spi;
} nrf24l01p_t;

void nrf24l01p_init(nrf24l01p_t *dev, SPIDriver *spi);
uint8_t nrf24l01p_command(nrf24l01p_t *dev, uint8_t cmd);
void nrf24l01p_command_write(nrf24l01p_t *dev, uint8_t cmd, const uint8_t *buf, uint8_t len);
void nrf24l01p_command_read(nrf24l01p_t *dev, uint8_t cmd, uint8_t *buf, uint8_t len);
uint8_t nrf24l01p_read_register(nrf24l01p_t *dev, uint8_t addr);
void nrf24l01p_write_register(nrf24l01p_t *dev, uint8_t addr, uint8_t val);
void nrf24l01p_read_multibyte_register(nrf24l01p_t *dev, uint8_t addr, uint8_t *buf, uint8_t len);
void nrf24l01p_write_multibyte_register(nrf24l01p_t *dev, uint8_t addr, const uint8_t *buf, uint8_t len);
uint8_t nrf24l01p_status(nrf24l01p_t *dev);
void nrf24l01p_flush_tx(nrf24l01p_t *dev);
void nrf24l01p_flush_rx(nrf24l01p_t *dev);
uint8_t nrf24l01p_read_rx_payload_len(nrf24l01p_t *dev);
uint8_t nrf24l01p_read_rx_payload(nrf24l01p_t *dev, uint8_t *buf, uint8_t len);
void nrf24l01p_write_tx_payload(nrf24l01p_t *dev, const uint8_t *buf, uint8_t len);
void nrf24l01p_write_ack_payload(nrf24l01p_t *dev, uint8_t pipe, const uint8_t *buf, uint8_t len);
void nrf24l01p_set_addr(nrf24l01p_t *dev, uint8_t reg, const uint8_t *addr, uint8_t len);
void nrf24l01p_set_channel(nrf24l01p_t *dev, uint8_t ch);
uint8_t nrf24l01p_get_status_register(nrf24l01p_t *dev);

#endif // NRF24L01P_H
