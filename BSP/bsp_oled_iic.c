#include "bsp_oled_iic.h"
#include "bsp_gpio.h"
#include "com_config.h"
#include "com_type.h"

#define OLED_CMD    0x00U
#define OLED_DATA   0x40U
#define OLED_FONT_WIDTH 6U
#define OLED_MAX_COL (OLED_WIDTH - 1U)

static uint8_t s_oledBuffer[OLED_PAGE_COUNT][OLED_WIDTH];

static void bsp_oled_delay(void);
static void bsp_oled_scl(uint8_t high);
static void bsp_oled_sda(uint8_t high);
static void bsp_oled_iic_start(void);
static void bsp_oled_iic_stop(void);
static void bsp_oled_iic_write_byte(uint8_t data);
static void bsp_oled_write(uint8_t control, uint8_t data);
static void bsp_oled_write_cmd(uint8_t cmd);
static void bsp_oled_write_data_stream(const uint8_t *data, uint8_t len);
static void bsp_oled_set_pos(uint8_t x, uint8_t page);
static uint8_t bsp_oled_char_to_6x8(char ch, uint8_t col);
static void bsp_oled_draw_char(uint8_t x, uint8_t page, char ch);

static void bsp_oled_delay(void)
{
    volatile uint16_t delay = 2U;

    while (delay-- > 0U)
    {
        __NOP();
    }
}

static void bsp_oled_scl(uint8_t high)
{
    bsp_gpio_write(OLED_SCL_PORT, OLED_SCL_PIN, high);
}

static void bsp_oled_sda(uint8_t high)
{
    bsp_gpio_write(OLED_SDA_PORT, OLED_SDA_PIN, high);
}

static void bsp_oled_iic_start(void)
{
    bsp_oled_sda(1U);
    bsp_oled_scl(1U);
    bsp_oled_delay();
    bsp_oled_sda(0U);
    bsp_oled_delay();
    bsp_oled_scl(0U);
}

static void bsp_oled_iic_stop(void)
{
    bsp_oled_sda(0U);
    bsp_oled_scl(1U);
    bsp_oled_delay();
    bsp_oled_sda(1U);
    bsp_oled_delay();
}

static void bsp_oled_iic_write_byte(uint8_t data)
{
    uint8_t bitIndex;

    for (bitIndex = 0U; bitIndex < 8U; bitIndex++)
    {
        bsp_oled_sda((data & 0x80U) != 0U);
        data <<= 1U;
        bsp_oled_delay();
        bsp_oled_scl(1U);
        bsp_oled_delay();
        bsp_oled_scl(0U);
    }

    bsp_oled_sda(1U);
    bsp_oled_delay();
    bsp_oled_scl(1U);
    bsp_oled_delay();
    bsp_oled_scl(0U);
}

static void bsp_oled_write(uint8_t control, uint8_t data)
{
    bsp_oled_iic_start();
    bsp_oled_iic_write_byte(OLED_IIC_ADDR);
    bsp_oled_iic_write_byte(control);
    bsp_oled_iic_write_byte(data);
    bsp_oled_iic_stop();
}

static void bsp_oled_write_cmd(uint8_t cmd)
{
    bsp_oled_write(OLED_CMD, cmd);
}

static void bsp_oled_set_pos(uint8_t x, uint8_t page)
{
    if (page >= OLED_PAGE_COUNT)
    {
        return;
    }

    bsp_oled_write_cmd((uint8_t)(0xB0U + page));
    bsp_oled_write_cmd((uint8_t)(0x10U | ((x >> 4U) & 0x0FU)));
    bsp_oled_write_cmd((uint8_t)(x & 0x0FU));
}

static void bsp_oled_write_data_stream(const uint8_t *data, uint8_t len)
{
    uint8_t index;

    if ((data == NULL) || (len == 0U))
    {
        return;
    }

    bsp_oled_iic_start();
    bsp_oled_iic_write_byte(OLED_IIC_ADDR);
    bsp_oled_iic_write_byte(OLED_DATA);
    for (index = 0U; index < len; index++)
    {
        bsp_oled_iic_write_byte(data[index]);
    }
    bsp_oled_iic_stop();
}

err_t bsp_oled_iic_init(void)
{
    uint16_t wait;

    bsp_gpio_enable_clock(OLED_SCL_PORT);
    bsp_gpio_enable_clock(OLED_SDA_PORT);
    bsp_gpio_set_config(OLED_SCL_PORT, OLED_SCL_PIN, BSP_GPIO_MODE_OUTPUT_50M_OD);
    bsp_gpio_set_config(OLED_SDA_PORT, OLED_SDA_PIN, BSP_GPIO_MODE_OUTPUT_50M_OD);
    bsp_oled_scl(1U);
    bsp_oled_sda(1U);

    for (wait = 0U; wait < 1000U; wait++)
    {
        bsp_oled_delay();
    }

    bsp_oled_write_cmd(0xAEU);
    bsp_oled_write_cmd(0x20U);
    bsp_oled_write_cmd(0x02U);
    bsp_oled_write_cmd(0xB0U);
    bsp_oled_write_cmd(0xC8U);
    bsp_oled_write_cmd(0x00U);
    bsp_oled_write_cmd(0x10U);
    bsp_oled_write_cmd(0x40U);
    bsp_oled_write_cmd(0x81U);
    bsp_oled_write_cmd(0x7FU);
    bsp_oled_write_cmd(0xA1U);
    bsp_oled_write_cmd(0xA6U);
    bsp_oled_write_cmd(0xA8U);
    bsp_oled_write_cmd(0x3FU);
    bsp_oled_write_cmd(0xA4U);
    bsp_oled_write_cmd(0xD3U);
    bsp_oled_write_cmd(0x00U);
    bsp_oled_write_cmd(0xD5U);
    bsp_oled_write_cmd(0x80U);
    bsp_oled_write_cmd(0xD9U);
    bsp_oled_write_cmd(0xF1U);
    bsp_oled_write_cmd(0xDAU);
    bsp_oled_write_cmd(0x12U);
    bsp_oled_write_cmd(0xDBU);
    bsp_oled_write_cmd(0x40U);
    bsp_oled_write_cmd(0x8DU);
    bsp_oled_write_cmd(0x14U);
    bsp_oled_write_cmd(0xAFU);

    bsp_oled_iic_clear();
    bsp_oled_iic_refresh();

    return ERR_OK;
}

void bsp_oled_iic_clear(void)
{
    uint8_t page;
    uint8_t col;

    for (page = 0U; page < OLED_PAGE_COUNT; page++)
    {
        for (col = 0U; col < OLED_WIDTH; col++)
        {
            s_oledBuffer[page][col] = 0U;
        }
    }
}

void bsp_oled_iic_show_string(uint8_t x, uint8_t y, const char *str)
{
    uint8_t col = x;

    if ((str == NULL) || (y >= OLED_PAGE_COUNT))
    {
        return;
    }

    while ((*str != '\0') && (col <= (OLED_MAX_COL - OLED_FONT_WIDTH)))
    {
        bsp_oled_draw_char(col, y, *str);
        col = (uint8_t)(col + OLED_FONT_WIDTH);
        str++;
    }
}

void bsp_oled_iic_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len)
{
    char text[11];
    uint8_t index;

    if ((len == 0U) || (len >= sizeof(text)))
    {
        return;
    }

    text[len] = '\0';
    for (index = 0U; index < len; index++)
    {
        text[len - index - 1U] = (char)('0' + (num % 10U));
        num /= 10U;
    }

    bsp_oled_iic_show_string(x, y, text);
}

void bsp_oled_iic_show_bitmap(uint8_t x,
                              uint8_t page,
                              uint8_t width,
                              uint8_t pageCount,
                              const uint8_t *bitmap)
{
    uint8_t pageOffset;
    uint8_t col;

    if ((bitmap == NULL) ||
        (width == 0U) ||
        (pageCount == 0U) ||
        (x >= OLED_WIDTH) ||
        (page >= OLED_PAGE_COUNT))
    {
        return;
    }

    for (pageOffset = 0U; pageOffset < pageCount; pageOffset++)
    {
        if ((uint8_t)(page + pageOffset) >= OLED_PAGE_COUNT)
        {
            break;
        }

        for (col = 0U; col < width; col++)
        {
            if ((uint8_t)(x + col) >= OLED_WIDTH)
            {
                break;
            }

            s_oledBuffer[page + pageOffset][x + col] =
                bitmap[(uint16_t)pageOffset * width + col];
        }
    }
}

void bsp_oled_iic_show_chinese16(uint8_t x, uint8_t page, const uint8_t *font16x16)
{
    bsp_oled_iic_show_bitmap(x, page, 16U, 2U, font16x16);
}

void bsp_oled_iic_show_frame(const uint8_t *frame)
{
    uint8_t page;
    uint8_t col;

    if (frame == NULL)
    {
        return;
    }

    for (page = 0U; page < OLED_PAGE_COUNT; page++)
    {
        for (col = 0U; col < OLED_WIDTH; col++)
        {
            s_oledBuffer[page][col] = frame[(uint16_t)page * OLED_WIDTH + col];
        }
    }
}

void bsp_oled_iic_refresh(void)
{
    uint8_t page;

    for (page = 0U; page < OLED_PAGE_COUNT; page++)
    {
        bsp_oled_set_pos(0U, page);
        bsp_oled_write_data_stream(s_oledBuffer[page], OLED_WIDTH);
    }
}

static void bsp_oled_draw_char(uint8_t x, uint8_t page, char ch)
{
    uint8_t col;

    if ((page >= OLED_PAGE_COUNT) || (x > (OLED_MAX_COL - OLED_FONT_WIDTH)))
    {
        return;
    }

    for (col = 0U; col < OLED_FONT_WIDTH; col++)
    {
        s_oledBuffer[page][x + col] = bsp_oled_char_to_6x8(ch, col);
    }
}

static uint8_t bsp_oled_char_to_6x8(char ch, uint8_t col)
{
    static const uint8_t blank[OLED_FONT_WIDTH] = { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static const uint8_t dash[OLED_FONT_WIDTH] = { 0x08U, 0x08U, 0x08U, 0x08U, 0x08U, 0x00U };
    static const uint8_t digits[10][OLED_FONT_WIDTH] =
    {
        { 0x3EU, 0x51U, 0x49U, 0x45U, 0x3EU, 0x00U },
        { 0x00U, 0x42U, 0x7FU, 0x40U, 0x00U, 0x00U },
        { 0x42U, 0x61U, 0x51U, 0x49U, 0x46U, 0x00U },
        { 0x21U, 0x41U, 0x45U, 0x4BU, 0x31U, 0x00U },
        { 0x18U, 0x14U, 0x12U, 0x7FU, 0x10U, 0x00U },
        { 0x27U, 0x45U, 0x45U, 0x45U, 0x39U, 0x00U },
        { 0x3CU, 0x4AU, 0x49U, 0x49U, 0x30U, 0x00U },
        { 0x01U, 0x71U, 0x09U, 0x05U, 0x03U, 0x00U },
        { 0x36U, 0x49U, 0x49U, 0x49U, 0x36U, 0x00U },
        { 0x06U, 0x49U, 0x49U, 0x29U, 0x1EU, 0x00U }
    };
    static const uint8_t upper[26][OLED_FONT_WIDTH] =
    {
        { 0x7EU, 0x11U, 0x11U, 0x11U, 0x7EU, 0x00U },
        { 0x7FU, 0x49U, 0x49U, 0x49U, 0x36U, 0x00U },
        { 0x3EU, 0x41U, 0x41U, 0x41U, 0x22U, 0x00U },
        { 0x7FU, 0x41U, 0x41U, 0x22U, 0x1CU, 0x00U },
        { 0x7FU, 0x49U, 0x49U, 0x49U, 0x41U, 0x00U },
        { 0x7FU, 0x09U, 0x09U, 0x09U, 0x01U, 0x00U },
        { 0x3EU, 0x41U, 0x49U, 0x49U, 0x7AU, 0x00U },
        { 0x7FU, 0x08U, 0x08U, 0x08U, 0x7FU, 0x00U },
        { 0x00U, 0x41U, 0x7FU, 0x41U, 0x00U, 0x00U },
        { 0x20U, 0x40U, 0x41U, 0x3FU, 0x01U, 0x00U },
        { 0x7FU, 0x08U, 0x14U, 0x22U, 0x41U, 0x00U },
        { 0x7FU, 0x40U, 0x40U, 0x40U, 0x40U, 0x00U },
        { 0x7FU, 0x02U, 0x0CU, 0x02U, 0x7FU, 0x00U },
        { 0x7FU, 0x04U, 0x08U, 0x10U, 0x7FU, 0x00U },
        { 0x3EU, 0x41U, 0x41U, 0x41U, 0x3EU, 0x00U },
        { 0x7FU, 0x09U, 0x09U, 0x09U, 0x06U, 0x00U },
        { 0x3EU, 0x41U, 0x51U, 0x21U, 0x5EU, 0x00U },
        { 0x7FU, 0x09U, 0x19U, 0x29U, 0x46U, 0x00U },
        { 0x46U, 0x49U, 0x49U, 0x49U, 0x31U, 0x00U },
        { 0x01U, 0x01U, 0x7FU, 0x01U, 0x01U, 0x00U },
        { 0x3FU, 0x40U, 0x40U, 0x40U, 0x3FU, 0x00U },
        { 0x1FU, 0x20U, 0x40U, 0x20U, 0x1FU, 0x00U },
        { 0x3FU, 0x40U, 0x38U, 0x40U, 0x3FU, 0x00U },
        { 0x63U, 0x14U, 0x08U, 0x14U, 0x63U, 0x00U },
        { 0x07U, 0x08U, 0x70U, 0x08U, 0x07U, 0x00U },
        { 0x61U, 0x51U, 0x49U, 0x45U, 0x43U, 0x00U }
    };
    static const uint8_t lower[26][OLED_FONT_WIDTH] =
    {
        { 0x20U, 0x54U, 0x54U, 0x54U, 0x78U, 0x00U },
        { 0x7FU, 0x48U, 0x44U, 0x44U, 0x38U, 0x00U },
        { 0x38U, 0x44U, 0x44U, 0x44U, 0x20U, 0x00U },
        { 0x38U, 0x44U, 0x44U, 0x48U, 0x7FU, 0x00U },
        { 0x38U, 0x54U, 0x54U, 0x54U, 0x18U, 0x00U },
        { 0x08U, 0x7EU, 0x09U, 0x01U, 0x02U, 0x00U },
        { 0x0CU, 0x52U, 0x52U, 0x52U, 0x3EU, 0x00U },
        { 0x7FU, 0x08U, 0x04U, 0x04U, 0x78U, 0x00U },
        { 0x00U, 0x44U, 0x7DU, 0x40U, 0x00U, 0x00U },
        { 0x20U, 0x40U, 0x44U, 0x3DU, 0x00U, 0x00U },
        { 0x7FU, 0x10U, 0x28U, 0x44U, 0x00U, 0x00U },
        { 0x00U, 0x41U, 0x7FU, 0x40U, 0x00U, 0x00U },
        { 0x7CU, 0x04U, 0x18U, 0x04U, 0x78U, 0x00U },
        { 0x7CU, 0x08U, 0x04U, 0x04U, 0x78U, 0x00U },
        { 0x38U, 0x44U, 0x44U, 0x44U, 0x38U, 0x00U },
        { 0x7CU, 0x14U, 0x14U, 0x14U, 0x08U, 0x00U },
        { 0x08U, 0x14U, 0x14U, 0x18U, 0x7CU, 0x00U },
        { 0x7CU, 0x08U, 0x04U, 0x04U, 0x08U, 0x00U },
        { 0x48U, 0x54U, 0x54U, 0x54U, 0x20U, 0x00U },
        { 0x04U, 0x3FU, 0x44U, 0x40U, 0x20U, 0x00U },
        { 0x3CU, 0x40U, 0x40U, 0x20U, 0x7CU, 0x00U },
        { 0x1CU, 0x20U, 0x40U, 0x20U, 0x1CU, 0x00U },
        { 0x3CU, 0x40U, 0x30U, 0x40U, 0x3CU, 0x00U },
        { 0x44U, 0x28U, 0x10U, 0x28U, 0x44U, 0x00U },
        { 0x0CU, 0x50U, 0x50U, 0x50U, 0x3CU, 0x00U },
        { 0x44U, 0x64U, 0x54U, 0x4CU, 0x44U, 0x00U }
    };

    if (col >= OLED_FONT_WIDTH)
    {
        return 0U;
    }
    if ((ch >= '0') && (ch <= '9'))
    {
        return digits[ch - '0'][col];
    }
    if ((ch >= 'a') && (ch <= 'z'))
    {
        return lower[ch - 'a'][col];
    }
    if ((ch >= 'A') && (ch <= 'Z'))
    {
        return upper[ch - 'A'][col];
    }
    if (ch == '-')
    {
        return dash[col];
    }

    return blank[col];
}
