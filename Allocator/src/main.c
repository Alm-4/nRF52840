#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "myalloc.h"

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
 * Select UART0 registers.
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

/* TXD pin on nRF52840-DK */
#define UART_TXD_PIN            (6UL)
/* RXD pin on nRF52840-DK */
#define UART_RXD_PIN            (8UL)
/* Enable the UART */
#define UART_ENABLE_ENABLED     (4UL)
/* Baud rate of 115200 */
#define UART_BAUDRATE_115200    (0x01D7E000UL)


void
delay(volatile uint32_t count)
{
    while (count--)
    {
        __asm("nop");
    }
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


/**
 * Process a command received over UART.
 * @param command The command string to process
 * @returns A pointer to the allocated memory block
 * if the command was "alloc" or NULL otherwise
 */
void *
do_command(char *command)
{
    if (strcmp(command, "status") == 0)
    {
        uart_send_string("\n");
        char * str = print_status();
        while (str != NULL)
        {
            uart_send_string(str);
            str = print_status();
        }

        return NULL;
    }

    char *end;
    if (strncmp(command, "alloc ", 6) == 0)
    {
        char *arg = command + 6;
        int size = strtol(arg, &end, 10);
        /* Validate argument */
        if (end == arg || size <= 0)
        {
            uart_send_string("\nInvalid value for alloc\n");
            return NULL;
        }
        /* Skip any leading whitespace */
        while (*end == ' ')
        {
            end++;
        }
        if (*end != '\0')
        {
            uart_send_string("\nInvalid value for alloc\n");
            return NULL;
        }

        void *ptr = myalloc(size);
        if (ptr == NULL)
        {
            uart_send_string("\nAlloc failed\n");
        }

        return ptr;
    }
    else if (strncmp(command, "free ", 5) == 0)
    {
        char *arg = command + 5;
        uint32_t addr = strtol(arg, &end, 0);
        /* Validate argument */
        if (end == arg || addr <= 0)
        {
            uart_send_string("\nInvalid value for free\n");
            return NULL;
        }
        /* Skip any leading whitespace */
        while (*end == ' ')
        {
            end++;
        }
        if (*end != '\0')
        {
            uart_send_string("\nInvalid value for free\n");
            return NULL;
        }
        myfree((void *)addr);
    }

    return NULL;
}


int
main(void)
{
    int counter = 0;
    char buff[128];
    char input[] = " ";
    char msg[] = "$";

    uart_init();
    uart_send_string(msg);

    /* Loop infinitely mimicking a terminal */
    for (;;)
    {
        uart_recv_byte(&input[0]);
        if (input[0] == '\r')
        {
            continue;
        }
        if (input[0] == '\n')
        {
            buff[counter] = '\0';
            counter = 0;
            void * ptr = do_command(buff);
            if (ptr != NULL)
            {
                char addr_msg[32];
                sprintf(addr_msg, "\nAllocated at address: %p", ptr);
                uart_send_string(addr_msg);
            }
            uart_send_string("\n");
            uart_send_string(msg);
            continue;
        }
        if ((unsigned long)counter >= sizeof(buff) - 1)
        {
            uart_send_string("\nCommand too long bruh\n");
            counter = 0;
            uart_send_string(msg);
            continue;
        }

        uart_send_string(input);
        buff[counter] = input[0];
        counter++;
    }

    return 0;
}