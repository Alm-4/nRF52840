#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/** LED 1 Pin - P0.13 */
#define LED1_PIN    (13)

/** LED 2 Pin - P0.14 */
#define LED2_PIN    (14)

/** LED 3 Pin - P0.15 */
#define LED3_PIN    (15)

/** LED 4 Pin - P0.16 */
#define LED4_PIN    (16)

/** UART base address */
#define UART0_BASE  (0x40002000UL)

/** 2.4GHz radio base address */
#define RD_BASE     (0x40001000UL)

/** Clock base address */
#define CLOCK_BASE  (0x40000000UL)

/**
 * GPIO registers.
 *
 * Refer to 6.9.2 in the nRF52840 Product Specification.
 */
volatile uint32_t *GPIO_OUT    = (volatile uint32_t *)0x50000504;
volatile uint32_t *GPIO_OUTSET = (volatile uint32_t *)0x50000508;
volatile uint32_t *GPIO_OUTCLR = (volatile uint32_t *)0x5000050C;
volatile uint32_t *GPIO_DIR    = (volatile uint32_t *)0x50000514;

/**
 * UART0 registers.
 *
 * Refer to 6.33.10 in the nRF52840 Product Specification.
 */
volatile uint32_t *UART_TASKS_STARTTX = (volatile uint32_t *)(UART0_BASE + 0x008);
volatile uint32_t *UART_EVENTS_TXDRDY = (volatile uint32_t *)(UART0_BASE + 0x11C);
volatile uint32_t *UART_ENABLE        = (volatile uint32_t *)(UART0_BASE + 0x500);
volatile uint32_t *UART_PSEL_TXD      = (volatile uint32_t *)(UART0_BASE + 0x50C);
volatile uint32_t *UART_TXD           = (volatile uint32_t *)(UART0_BASE + 0x51C);
volatile uint32_t *UART_BAUDRATE      = (volatile uint32_t *)(UART0_BASE + 0x524);
volatile uint32_t *UART_TASKS_STARTRX = (volatile uint32_t *)(UART0_BASE + 0x000);
volatile uint32_t *UART_EVENTS_RXDRDY = (volatile uint32_t *)(UART0_BASE + 0x108);
volatile uint32_t *UART_PSEL_RXD      = (volatile uint32_t *)(UART0_BASE + 0x514);
volatile uint32_t *UART_RXD           = (volatile uint32_t *)(UART0_BASE + 0x518);

/**
 * Radio registers.
 *
 * Refer to 6.20.14 in the nRF52840 Product Specification.
 */
volatile uint32_t *TASKS_TXEN    = (volatile uint32_t *)(RD_BASE + 0x000);
volatile uint32_t *TASKS_RXEN    = (volatile uint32_t *)(RD_BASE + 0x004);
volatile uint32_t *TASKS_START   = (volatile uint32_t *)(RD_BASE + 0x008);
volatile uint32_t *TASKS_STOP    = (volatile uint32_t *)(RD_BASE + 0x00C);
volatile uint32_t *TASKS_DISABLE = (volatile uint32_t *)(RD_BASE + 0x010);

volatile uint32_t *EVENTS_READY    = (volatile uint32_t *)(RD_BASE + 0x100);
volatile uint32_t *EVENTS_ADDRESS  = (volatile uint32_t *)(RD_BASE + 0x104);
volatile uint32_t *EVENTS_PAYLOAD  = (volatile uint32_t *)(RD_BASE + 0x108);
volatile uint32_t *EVENTS_END      = (volatile uint32_t *)(RD_BASE + 0x10C);
volatile uint32_t *EVENTS_DISABLED = (volatile uint32_t *)(RD_BASE + 0x110);
volatile uint32_t *EVENTS_CRCOK    = (volatile uint32_t *)(RD_BASE + 0x130);
volatile uint32_t *EVENTS_CRCERROR = (volatile uint32_t *)(RD_BASE + 0x134);

volatile uint32_t *PACKETPTR = (volatile uint32_t *)(RD_BASE + 0x504);
volatile uint32_t *FREQUENCY = (volatile uint32_t *)(RD_BASE + 0x508);
volatile uint32_t *TXPOWER   = (volatile uint32_t *)(RD_BASE + 0x50C);
volatile uint32_t *MODE      = (volatile uint32_t *)(RD_BASE + 0x510);
volatile uint32_t *PCNF0     = (volatile uint32_t *)(RD_BASE + 0x514);
volatile uint32_t *PCNF1     = (volatile uint32_t *)(RD_BASE + 0x518);
volatile uint32_t *CRCCNF    = (volatile uint32_t *)(RD_BASE + 0x534);

volatile uint32_t *BASE0       = (volatile uint32_t *)(RD_BASE + 0x51C);
volatile uint32_t *PREFIX0     = (volatile uint32_t *)(RD_BASE + 0x524);
volatile uint32_t *TXADDRESS   = (volatile uint32_t *)(RD_BASE + 0x52C);
volatile uint32_t *RXADDRESSES = (volatile uint32_t *)(RD_BASE + 0x530);

/**
 * CLOCK registers.
 *
 * Refer to 5.4.3 in the nRF52840 Product Specification.
 */
volatile uint32_t *CLOCK_TASKS_HFCLKSTART    = (volatile uint32_t *)(CLOCK_BASE + 0x000);
volatile uint32_t *CLOCK_EVENTS_HFCLKSTARTED = (volatile uint32_t *)(CLOCK_BASE + 0x100);

/* TXD pin on nRF52840-DK */
#define UART_TXD_PIN            (6UL)
/* RXD pin on nRF52840-DK */
#define UART_RXD_PIN            (8UL)
/* Enable the UART */
#define UART_ENABLE_ENABLED     (4UL)
/* Baud rate of 115200 */
#define UART_BAUDRATE_115200    (0x01D7E000UL)
/* Max radio payload in bytes */
#define RD_MAXLEN               (127UL)
/* Max payload length in bytes */
#define RD_BALEN   (4UL)
/* Radio frequency = 2400 + RD_FREQUENCY (MHz) */
#define RD_FREQUENCY   (7UL)
/* Base address values */
#define RD_BASE0    (0xE7E7E7E7UL)
#define RD_PREFIX0  (0xC2UL)
/* Radio mode: 0 = Nrf_1Mbit*/
#define RD_MODE_NRF_1MBIT   (0UL)
/* Radio TX power: 0x00 = 0 (dBm) */
#define RD_TXPOWER_0DBM     (0x00UL)


void
delay(volatile uint32_t count)
{
    while (count--)
    {
        __asm("nop");
    }
}


void
rd_init(void)
{
    /* Start HXFO clock */
    *CLOCK_TASKS_HFCLKSTART = 1;
    while(!*CLOCK_EVENTS_HFCLKSTARTED) {}

    /* Set the Length field of wireless packets to be 8 bits */
    *PCNF0 = 0x00000008;
    /* Set channel parameters */
    *FREQUENCY = RD_FREQUENCY;
    *MODE = RD_MODE_NRF_1MBIT;
    *TXPOWER = RD_TXPOWER_0DBM;
    /* Max payload length and base address length */
    *PCNF1 = RD_MAXLEN | (RD_BALEN << 16);  /* Fancy method */
    /* Disable CRC */
    *CRCCNF = 0;
    /* Address to transmit and listen on */
    *BASE0 = RD_BASE0;
    *PREFIX0 = RD_PREFIX0;
    *TXADDRESS = 0;
    *RXADDRESSES = 1;
}


/**
 * Transmit a message over radio.
 * @param msg Message to transmit
 */
void
rd_transmit(char *msg)
{
    size_t len = strlen(msg);
    if (len > RD_MAXLEN)
    {
        len = RD_MAXLEN;
    }
    /* Put message into a buffer to transmit it */
    uint8_t send_buff[len + 1];
    send_buff[0] = (uint8_t)len;
    memcpy(&send_buff[1], msg, len);
    *PACKETPTR = (uint32_t)send_buff;

    /* Enable transmitter */
    *EVENTS_READY = 0;
    *EVENTS_END = 0;
    *TASKS_TXEN = 1;
    while(!*EVENTS_READY) {}

    /* Start transmit */
    *TASKS_START = 1;
    while(!*EVENTS_END) {}
}


/**
 * Receive a message over radio.
 * @param out Buffer to store the
 * received message
 */
void
rd_recv(char *out)
{
    uint8_t msg_buff[RD_MAXLEN + 1];
    *PACKETPTR = (uint32_t)msg_buff;

    /* Enable receiver*/
    *EVENTS_READY = 0;
    *EVENTS_END = 0;
    *TASKS_RXEN = 1;
    while(!*EVENTS_READY) {}

    /* Start receiving */
    *TASKS_START = 1;
    while(!*EVENTS_END) {}

    /* Copy out the received data */
    uint8_t len = msg_buff[0];
    if (len > RD_MAXLEN)
    {
        len = RD_MAXLEN;
    }
    memcpy(out, &msg_buff[1], len);
    out[len] = '\0';
}


/**
 * Configure and enable UART0 for transmission.
 * This relies on default reset values for the
 * nRF52840-DK
 */
void
uart_init(void)
{
    /* Connect pins */
    *UART_PSEL_TXD = UART_TXD_PIN;
    *UART_PSEL_RXD = UART_RXD_PIN;
    /* Set the baud rate */
    *UART_BAUDRATE = UART_BAUDRATE_115200;
    /* Enable the UART */
    *UART_ENABLE = UART_ENABLE_ENABLED;
    /* Start UART transmission and reception */
    *UART_TASKS_STARTTX = 1;
    *UART_TASKS_STARTRX = 1;
}


/**
 * Transmit a string over UART and block until its sent.
 * @param s The string to transmit
 */
void
uart_send_string(char *s)
{
    /* Print each byte individually */
    while (*s != '\0')
    {
        /* Clear the TXDRDY event */
        *UART_EVENTS_TXDRDY = 0;
        /* Write to TXD to send the byte */
        *UART_TXD = *s;
        /* Wait for the byte to be sent */
        while (*UART_EVENTS_TXDRDY == 0) { }
        s++;
    }
}


/**
 * Recieve a byte over UART.
 * @param c A pointer to a char
 * to store the received byte
 */
void
uart_recv_byte(char *c)
{
    /* Clear the RXDRDY event */
    *UART_EVENTS_RXDRDY = 0;
    /* Wait for a byte to be received */
    while (*UART_EVENTS_RXDRDY == 0) { }
    /* Read the received byte */
    *c = *UART_RXD;
}


int
main(void)
{
    int counter = 0;
    char buff[128];
    char input[] = " ";
    char msg[] = "$";
    uart_init();
    rd_init();
    uart_send_string(msg);

    for (;;)
    {
        uart_recv_byte(&input[0]);
        /* Listen for \n characters only */
        if (input[0] == '\r')
        {
            continue;
        }
        if (input[0] == '\n')
        {
            buff[counter] = '\0';
            counter = 0;
            rd_transmit(buff);
            uart_send_string("\nTransmitted message\nReceiving message... ");

            /* Start receiving a message. The terminal will
            be unresponsive until a message is received */
            rd_recv(buff);
            uart_send_string("\nMessage:\n");
            uart_send_string(buff);
            continue;
        }
        if ((unsigned long)counter >= sizeof(buff) - 1)
        {
            uart_send_string("\nCommand too long bruh\n");
            counter = 0;
            uart_send_string(msg);
            continue;
        }

        /* Echo back whats sent */
        uart_send_string(input);
        buff[counter] = input[0];
        counter++;
    }

    return 0;
}