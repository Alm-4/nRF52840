#include <stdint.h>

extern uint32_t __stack_top;
extern uint32_t __data_load;
extern uint32_t __data_start;
extern uint32_t __data_end;
extern uint32_t __bss_start;
extern uint32_t __bss_end;

extern void
__libc_init_array(void);
extern int
main(void);

void
Reset_Handler(void);
void
Default_Handler(void);

/* Core exceptions */
void
NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void
HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void
MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void
BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void
UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void
SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void
DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void
PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void
SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

/* nRF52840-specific interrupts */
void
power_clock_irq_handler(void) __attribute__((weak, alias("Default_Handler")));
void
radio_irq_handler(void) __attribute__((weak, alias("Default_Handler")));
void
uarte0_uart0_irq_handler(void) __attribute__((weak, alias("Default_Handler")));
void
spim0_spis0_twim0_twis0_spi0_twi0_irq_handler(void)
    __attribute__((weak, alias("Default_Handler")));
/* Add more interrupt handlers here as needed */

__attribute__((section(".isr_vector"))) void (*const g_pfnVectors[])(void) = {
    (void *)&__stack_top,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0,
    0,
    0,
    0,
    SVC_Handler,
    DebugMon_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler,

    /* nRF52840-specific interrupts */
    power_clock_irq_handler,
    radio_irq_handler,
    uarte0_uart0_irq_handler,
    spim0_spis0_twim0_twis0_spi0_twi0_irq_handler,
    /* Add more interrupt handlers here as needed */
};

/* System initialization function declaration */
void
system_init(void);

void
Reset_Handler(void)
{
    uint32_t *p_src, *p_dest;

    /* Copy the data segment initializers from flash to SRAM */
    p_src  = &__data_load;
    p_dest = &__data_start;
    while (p_dest < &__data_end)
    {
        *p_dest++ = *p_src++;
    }

    /* Zero fill the bss segment */
    p_dest = &__bss_start;
    while (p_dest < &__bss_end)
    {
        *p_dest++ = 0;
    }

    /* Call the clock system initialization function */
    system_init();

    /* Call static constructors */
    __libc_init_array();

    /* Call the application's entry point */
    main();

    /* Infinite loop */
    for (;;)
        ;
}

void
Default_Handler(void)
{
    for (;;)
        ;
}

/* Disables Access Port Protection (APPROTECT) on nRF52840 devices.
 *
 * APPROTECT, when enabled, blocks debugger access to CPU registers and
 * memory-mapped addresses. Disabling it is crucial for debugging and
 * development purposes, allowing full access to the device.
 *
 * WARNING: This function should only be used during development and removed for
 * production firmware as it reduces the security of the device.
 *
 * For more information, refer to section 4.8.2 "Access port protection" in the
 * nRF52840 Product Specification.
 *
 * Note: The default behaviour of APPROTECT changed in Fxx series revisions of
 * nRF52840.
 */
void
disable_approtect(void)
{
    volatile uint32_t *const UICR_APPROTECT = (volatile uint32_t *)0x10001208;
    volatile uint32_t *const NVMC_CONFIG    = (volatile uint32_t *)0x4001E504;
    volatile uint32_t *const NVMC_READY     = (volatile uint32_t *)0x4001E400;
    volatile uint32_t *const APPROTECT_DISABLE =
        (volatile uint32_t *)0x50000558;

    /* Enable write to UICR */
    *NVMC_CONFIG = 1;
    while (!*NVMC_READY)
        ;

    /* Set UICR.APPROTECT to HwDisabled (0x5A) for devices where access
       port protection is controlled by hardware and software */
    *UICR_APPROTECT = 0x5A;

    /* Wait for write to complete */
    while (!*NVMC_READY)
        ;

    /* Disable write to UICR */
    *NVMC_CONFIG = 0;

    /* Firmware must write APPROTECT.DISABLE to SwDisable (0x5A) */
    *APPROTECT_DISABLE = 0x5A;

    /* Note: No reset is triggered here. The hardware part will take effect
       after the next reset. The software part is immediate but will be reset
       under certain conditions. */
}

void
system_init(void)
{
    disable_approtect();
}
