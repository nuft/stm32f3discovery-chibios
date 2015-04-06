#include <ch.h>
#include <hal.h>
#include <stdint.h>
#include "nrf24l01p.h"
#include "nrf24l01p_registers.h"
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
    // uint8_t config = 0;
    uint8_t config = EN_CRC;
    nrf24l01p_write_register(dev, CONFIG, config);
    // nrf24l01p_write_register(dev, CONFIG, PWR_UP | EN_CRC | CRCO | MASK_RX_DR | MASK_MAX_RT);
    // frequency = 2400 + <channel> [MHz], maximum: 2525MHz
    nrf24l01p_set_channel(dev, channel);
    // 0dBm power, datarate 2M/1M/250K
    nrf24l01p_write_register(dev, RF_SETUP, RF_PWR(3) | RF_DR_250K);
    // Disable retransmission, 1500us delay
    nrf24l01p_write_register(dev, SETUP_RETR, ARD(0) | ARC(0));
    // disable retransmission, 1500us delay
    nrf24l01p_write_register(dev, SETUP_RETR, 0);
    // disable dynamic packet length (DPL)
    nrf24l01p_write_register(dev, FEATURE, 0);
    // disable Enhanced ShockBurst Auto Acknowledgment
    nrf24l01p_write_register(dev, EN_AA, 0);
    // 3 byte address length
    nrf24l01p_write_register(dev, SETUP_AW, AW_3);
    // TX address
    nrf24l01p_set_addr(dev, TX_ADDR, address, 3);
    // clear data fifo
    nrf24l01p_flush_tx(dev);
    // clear IRQ flags
    nrf24l01p_write_register(dev, STATUS, RX_DR | TX_DS | MAX_RT);
    nrf24l01p_write_register(dev, CONFIG, config | PWR_UP);
}

void cmd_radio_tx(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    chprintf(chp, "tx\n");

    static nrf24l01p_t nrf24l01p;
    nrf24l01p_t *nrf = &nrf24l01p;
    spi_init();
    nrf24l01p_init(nrf, &SPID2);
    nrf_setup_ptx(nrf);
    chThdSleepMilliseconds(1);

    event_listener_t radio_event_listener;
    chEvtRegisterMaskWithFlags(&exti_events, &radio_event_listener,
        NRF_INTERRUPT_EVENT, EXTI_EVENT_NRF_IRQ);

    static uint32_t tx_count = 0;
    while (1) {
        // clear interrupts
        nrf24l01p_write_register(nrf, STATUS, RX_DR | TX_DS | MAX_RT);

        static uint32_t tx_buf[8];
        tx_buf[0] = tx_count++;
        // nrf24l01p_write_tx_payload_no_ack(nrf, (uint8_t *) tx_buf, 32);
        nrf24l01p_write_tx_payload(nrf, (uint8_t *) tx_buf, 32);

        nrf_ce_active();
        eventmask_t ret = chEvtWaitAnyTimeout(NRF_INTERRUPT_EVENT, MS2ST(1000));
        nrf_ce_inactive();

        if (ret == 0) {
            chprintf(chp, "TIMEOUT!\n");
            nrf24l01p_flush_tx(nrf);
            palTogglePad(GPIOE, GPIOE_LED3_RED);
            continue;
        }

        palTogglePad(GPIOE, GPIOE_LED7_GREEN);
    }
}

void nrf_setup_prx(nrf24l01p_t *dev)
{
    nrf_ce_inactive();
    // 2 byte CRC, enable RX_DR, mask MAX_RT and TX_DS IRQ
    // nrf24l01p_write_register(dev, CONFIG, PRIM_RX | PWR_UP);
    nrf24l01p_write_register(dev, CONFIG, PRIM_RX | PWR_UP | EN_CRC);
    // nrf24l01p_write_register(dev, CONFIG, PRIM_RX | PWR_UP | EN_CRC | CRCO
    //     | MASK_TX_DS | MASK_MAX_RT);
    // frequency = 2400 + <channel> [MHz], maximum: 2525MHz
    nrf24l01p_set_channel(dev, channel);
    // 0dBm power, datarate 2M/1M/250K
    nrf24l01p_write_register(dev, RF_SETUP, RF_PWR(3) | RF_DR_250K);
    // disable dynamic packet length (DPL)
    nrf24l01p_write_register(dev, FEATURE, 0);
    // 3 byte address length
    nrf24l01p_write_register(dev, SETUP_AW, AW_3);
    // RX address
    nrf24l01p_write_register(dev, EN_RXADDR, ERX_P0);
    nrf24l01p_set_addr(dev, RX_ADDR_P0, address, 3);
    nrf24l01p_write_register(dev, RX_PW_P0, 32);
    // disable Enhanced ShockBurst Auto Acknowledgment
    nrf24l01p_write_register(dev, EN_AA, 0);
    // clear data fifo
    nrf24l01p_flush_rx(dev);
    // clear IRQ flags
    nrf24l01p_write_register(dev, STATUS, RX_DR | TX_DS | MAX_RT);
}

void cmd_radio_rx(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    chprintf(chp, "rx\n");

    static nrf24l01p_t nrf24l01p;
    nrf24l01p_t *nrf = &nrf24l01p;
    spi_init();
    nrf24l01p_init(nrf, &SPID2);
    nrf_setup_prx(nrf);
    chThdSleepMilliseconds(1);

    event_listener_t radio_event_listener;
    chEvtRegisterMaskWithFlags(&exti_events, &radio_event_listener,
        NRF_INTERRUPT_EVENT, EXTI_EVENT_NRF_IRQ);

    uint32_t rx_nb = 0;
    uint32_t drop_count = 0;
    nrf_ce_active();
    while (1) {
        // chThdSleepMilliseconds(1);
        eventmask_t ret = chEvtWaitAnyTimeout(NRF_INTERRUPT_EVENT, MS2ST(1000));

        // nrf_ce_active();
        // eventmask_t ret = chEvtWaitAnyTimeout(NRF_INTERRUPT_EVENT, MS2ST(1000));
        // nrf_ce_inactive();

        if (ret == 0) {
            chprintf(chp, "TIMEOUT!\n");
            nrf24l01p_flush_rx(nrf);
            palTogglePad(GPIOE, GPIOE_LED3_RED);
            continue;
        }

        uint8_t status = nrf24l01p_status(nrf);
        if (status & RX_DR) {
            // clear status flags
            nrf24l01p_write_register(nrf, STATUS, RX_DR | TX_DS | MAX_RT);

            uint8_t len = nrf24l01p_read_rx_payload_len(nrf);
            if (len == 0 || len > 32) { // invalid length
                palTogglePad(GPIOE, GPIOE_LED3_RED);
                nrf24l01p_flush_rx(nrf);
                continue;
            }

            static uint32_t rx_buf[8];
            nrf24l01p_read_rx_payload(nrf, (uint8_t *) rx_buf, len);
            // chprintf(chp, ".");
            if (rx_buf[0] > rx_nb + 1) {
                drop_count += rx_buf[0] - rx_nb;
                // chprintf(chp, "%u\n", (unsigned int)drop_count);
                chprintf(chp, "#");
                palTogglePad(GPIOE, GPIOE_LED5_ORANGE);
            }
            rx_nb = rx_buf[0];
            palTogglePad(GPIOE, GPIOE_LED7_GREEN);
        }
        //  else {
        //     palTogglePad(GPIOE, GPIOE_LED3_RED);
        //     nrf24l01p_write_register(nrf, STATUS, RX_DR | TX_DS | MAX_RT);
        //     nrf24l01p_flush_rx(nrf);
        // }
    }
}
