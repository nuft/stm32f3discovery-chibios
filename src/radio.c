#include <ch.h>
#include <hal.h>
#include <stdint.h>
#include "nrf24l01p.h"
#include "radio.h"
#include "exti.h"
#include <chprintf.h>

const uint8_t address[] = {0x2A, 0x2A, 0x2A};
const uint8_t channel = 0;

#define NRF_INTERRUPT_EVENT 1

static void spi_init(void)
{
    /*
     * SPI2 configuration structure.
     * SPI2 is on APB1 @ 36MHz / 4 = 9MHz
     * CPHA=0, CPOL=0, 8bits frames, MSb transmitted first.
     */
    static SPIConfig spi_cfg = {
        .end_cb = NULL,
        .ssport = GPIOB,
        .sspad = GPIOB_NRF_CS,
        .cr1 =  SPI_CR1_BR_0 | SPI_CR1_BR_1
    };

    spiStart(&SPID2, &spi_cfg);
}

static void nrf_ce_active(void)
{
    palSetPad(GPIOB, GPIOB_NRF_CE);
}

static void nrf_ce_inactive(void)
{
    palClearPad(GPIOB, GPIOB_NRF_CE);
}

void nrf_setup_ptx(nrf24l01p_t *dev)
{
    nrf_ce_inactive();
    // 2 byte CRC, enable TX_DS, RX_DR and MAX_RT IRQ
    nrf24l01p_write_register(dev, CONFIG, PWR_UP | EN_CRC | CRCO);
    // frequency = 2400 + <channel> [MHz], maximum: 2525MHz
    nrf24l01p_set_channel(dev, channel);
    // 0dBm power, datarate 2M/1M/250K
    nrf24l01p_write_register(dev, RF_SETUP, RF_PWR(3) | RF_DR_250K);
    // Disable retransmission, 1500us delay
    nrf24l01p_write_register(dev, SETUP_RETR, ARD(5) | ARC(0));
    // enable dynamic packet length (DPL)
    nrf24l01p_write_register(dev, FEATURE, EN_DPL | EN_ACK_PAY);
    // 3 byte address length
    nrf24l01p_write_register(dev, SETUP_AW, AW_3);
    // TX address
    nrf24l01p_set_addr(dev, TX_ADDR, address, 3);
    // RX address
    nrf24l01p_set_addr(dev, RX_ADDR_P0, address, 3);
    nrf24l01p_write_register(dev, DYNPD, DPL_P0);
    // clear data fifo
    nrf24l01p_flush_tx(dev);
    nrf24l01p_flush_rx(dev);
    // clear IRQ flags
    nrf24l01p_write_register(dev, STATUS, RX_DR | TX_DS | MAX_RT);
}

void cmd_radio_tx(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    static nrf24l01p_t nrf24l01p;
    nrf24l01p_t *nrf = &nrf24l01p;
    spi_init();
    nrf24l01p_init(nrf, &SPID2);
    nrf_setup_ptx(nrf);

    event_listener_t radio_event_listener;
    chEvtRegisterMaskWithFlags(&exti_events, &radio_event_listener,
        NRF_INTERRUPT_EVENT, EXTI_EVENT_NRF_IRQ);

    static uint8_t tx_buf[32];
    static uint32_t rx_buf[8];

    while (1) {
        // clear interrupts
        nrf24l01p_write_register(nrf, STATUS, RX_DR | TX_DS | MAX_RT);

        nrf24l01p_write_tx_payload(nrf, tx_buf, 32);

        nrf_ce_active();
        eventmask_t ret = chEvtWaitAnyTimeout(NRF_INTERRUPT_EVENT, MS2ST(10));
        nrf_ce_inactive();

        if (ret == 0) {
            nrf24l01p_flush_tx(nrf);
            continue;
        }

        uint8_t status = nrf24l01p_status(nrf);
        if (status & RX_DR) {
            uint8_t len = nrf24l01p_read_rx_payload_len(nrf);
            if (len == 0 || len > 32) { // invalid length
                nrf24l01p_flush_rx(nrf);
                continue;
            }
            nrf24l01p_read_rx_payload(nrf, rx_buf, len);
        } else if (status & MAX_RT) {
            nrf24l01p_flush_tx(nrf);
        }
    }
}

void nrf_setup_prx(nrf24l01p_t *dev)
{
    nrf_ce_inactive();
    // 2 byte CRC, enable RX_DR, mask MAX_RT and TX_DS IRQ
    nrf24l01p_write_register(dev, CONFIG, PRIM_RX | PWR_UP | EN_CRC | CRCO
        | MASK_TX_DS | MASK_MAX_RT);
    // frequency = 2400 + <channel> [MHz], maximum: 2525MHz
    nrf24l01p_set_channel(dev, channel);
    // 0dBm power, datarate 2M/1M/250K
    nrf24l01p_write_register(dev, RF_SETUP, RF_PWR(3) | RF_DR_250K);
    // enable dynamic packet length (DPL)
    nrf24l01p_write_register(dev, FEATURE, EN_DPL | EN_ACK_PAY);
    // enable DPL for pipe 0
    nrf24l01p_write_register(dev, DYNPD, DPL_P0);
    // nrf24l01p_write_register(dev, RX_PW_P0, 32);
    // 3 byte address length
    nrf24l01p_write_register(dev, SETUP_AW, AW_3);
    // RX address
    nrf24l01p_write_register(dev, EN_RXADDR, ERX_P0);
    nrf24l01p_set_addr(dev, RX_ADDR_P0, address, 3);
    // enable Enhanced ShockBurst Auto Acknowledgment
    nrf24l01p_write_register(dev, EN_AA, ENAA_P0);
    // clear data fifo
    nrf24l01p_flush_tx(dev);
    nrf24l01p_flush_rx(dev);
    // default ack payload
    uint8_t ack[] = {0};
    nrf24l01p_write_ack_payload(dev, 0, &ack[0], sizeof(ack));
    // clear IRQ flags
    nrf24l01p_write_register(dev, STATUS, RX_DR | TX_DS | MAX_RT);
}

void cmd_radio_rx(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    static nrf24l01p_t nrf24l01p;
    nrf24l01p_t *nrf = &nrf24l01p;
    spi_init();
    nrf24l01p_init(nrf, &SPID2);
    nrf_setup_prx(nrf);

    event_listener_t radio_event_listener;
    chEvtRegisterMaskWithFlags(&exti_events, &radio_event_listener,
        NRF_INTERRUPT_EVENT, EXTI_EVENT_NRF_IRQ);

    static uint8_t ack[32];
    static uint8_t rx_buf[32];

    nrf_ce_active();

    while (1) {
        nrf24l01p_write_register(nrf, STATUS, RX_DR | TX_DS | MAX_RT);
        eventmask_t ret = chEvtWaitAnyTimeout(NRF_INTERRUPT_EVENT, MS2ST(100));

        if (ret == 0) {
            nrf24l01p_flush_rx(nrf);
            continue;
        }

        uint8_t status = nrf24l01p_status(nrf);
        if (status & RX_DR) {
            uint8_t len = nrf24l01p_read_rx_payload_len(nrf);
            if (len == 0 || len > 32) { // invalid length
                nrf24l01p_flush_rx(nrf);
                continue;
            }
            nrf24l01p_read_rx_payload(nrf, rx_buf, len);
        }

        nrf24l01p_write_ack_payload(nrf, 0, ack, 32);
    }
}
