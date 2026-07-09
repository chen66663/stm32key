#include "bsp_gpio.h"

void bsp_gpio_enable_clock(GPIO_TypeDef *port)
{
    if (port == GPIOA)
    {
        RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    }
    else if (port == GPIOB)
    {
        RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    }
    else if (port == GPIOC)
    {
        RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    }
    else if (port == GPIOD)
    {
        RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
    }
    else if (port == GPIOE)
    {
        RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;
    }
    else if (port == GPIOF)
    {
        RCC->APB2ENR |= RCC_APB2ENR_IOPFEN;
    }
    else if (port == GPIOG)
    {
        RCC->APB2ENR |= RCC_APB2ENR_IOPGEN;
    }
}

void bsp_gpio_set_config(GPIO_TypeDef *port, uint32_t pin, uint32_t config)
{
    volatile uint32_t *reg = (pin < 8U) ? &port->CRL : &port->CRH;
    uint32_t shift = (pin & 0x7U) * 4U;

    *reg = (*reg & ~(0xFUL << shift)) | ((config & 0xFU) << shift);
}

void bsp_gpio_write(GPIO_TypeDef *port, uint32_t pin, uint8_t high)
{
    if (high != 0U)
    {
        port->BSRR = 1UL << pin;
    }
    else
    {
        port->BRR = 1UL << pin;
    }
}

uint8_t bsp_gpio_read(GPIO_TypeDef *port, uint32_t pin)
{
    return ((port->IDR & (1UL << pin)) != 0U) ? 1U : 0U;
}
