/*
******************************************************************************
* @file           : switches.c
* @author         : Lennart Querter
* @brief          : 74HC165 chain read, GPIO buttons, debounce, edge events
******************************************************************************
*/

#include "app/switches.h"

#include "main.h"

/*
 * Which logical switch each 74HC165 chain bit maps to. Bit 0 is the LSB of
 * the second received byte (the shift register closest to the MCU loads
 * first into the received stream). Identity until bring-up says otherwise —
 * fix the panel order here, not in the UI code.
 */
static const uint8_t chain_map[16] = {0, 1, 2,  3,  4,  5,  6,  7,
                                      8, 9, 10, 11, 12, 13, 14, 15};

#define SWITCHES_EVENT_QUEUE_SIZE 16

static SPI_HandleTypeDef* sw_spi;

static uint32_t raw_bits;
static uint32_t stable_bits;
static uint8_t counters[SW_COUNT];

static struct SWITCHES_event event_queue[SWITCHES_EVENT_QUEUE_SIZE];
static uint8_t queue_head;
static uint8_t queue_tail;

void SWITCHES_init(SPI_HandleTypeDef* hspi)
{
    sw_spi = hspi;
    raw_bits = 0;
    stable_bits = 0;
    queue_head = 0;
    queue_tail = 0;
    for (uint8_t i = 0; i < SW_COUNT; i++)
    {
        counters[i] = 0;
    }
    HAL_GPIO_WritePin(SW_LOAD_GPIO_Port, SW_LOAD_Pin, GPIO_PIN_SET);
}

static void queue_push(uint8_t index, uint8_t pressed)
{
    uint8_t next = (uint8_t)((queue_head + 1) % SWITCHES_EVENT_QUEUE_SIZE);
    if (next == queue_tail)
    {
        return; // full: drop the newest, the scan will re-detect held keys
    }
    event_queue[queue_head].index = index;
    event_queue[queue_head].pressed = pressed;
    queue_head = next;
}

static uint32_t sample_inputs(void)
{
    uint8_t chain[2] = {0, 0};

    // ~PL low latches the parallel inputs; a few cycles is plenty at 96 MHz
    HAL_GPIO_WritePin(SW_LOAD_GPIO_Port, SW_LOAD_Pin, GPIO_PIN_RESET);
    for (volatile uint8_t i = 0; i < 10; i++)
    {
    }
    HAL_GPIO_WritePin(SW_LOAD_GPIO_Port, SW_LOAD_Pin, GPIO_PIN_SET);

    HAL_SPI_Receive(sw_spi, chain, 2, 5);

    uint16_t chain_bits = (uint16_t)((chain[0] << 8) | chain[1]);
#if SWITCHES_CHAIN_ACTIVE_LOW
    chain_bits = (uint16_t)~chain_bits;
#endif

    uint32_t bits = 0;
    for (uint8_t bit = 0; bit < 16; bit++)
    {
        if ((chain_bits >> bit) & 1u)
        {
            bits |= 1UL << chain_map[bit];
        }
    }

    uint32_t gpio = 0;
    gpio |= (uint32_t)(HAL_GPIO_ReadPin(BTN_A_GPIO_Port, BTN_A_Pin) == GPIO_PIN_SET) << 0;
    gpio |= (uint32_t)(HAL_GPIO_ReadPin(BTN_B_GPIO_Port, BTN_B_Pin) == GPIO_PIN_SET) << 1;
    gpio |= (uint32_t)(HAL_GPIO_ReadPin(BTN_C_GPIO_Port, BTN_C_Pin) == GPIO_PIN_SET) << 2;
    gpio |= (uint32_t)(HAL_GPIO_ReadPin(BTN_D_GPIO_Port, BTN_D_Pin) == GPIO_PIN_SET) << 3;
    gpio |= (uint32_t)(HAL_GPIO_ReadPin(ENC_SW_GPIO_Port, ENC_SW_Pin) == GPIO_PIN_SET) << 4;
#if SWITCHES_GPIO_ACTIVE_LOW
    gpio = ~gpio & 0x1F;
#endif
    bits |= gpio << SW_BTN_A;

    return bits;
}

void SWITCHES_scan(void)
{
    raw_bits = sample_inputs();

    for (uint8_t i = 0; i < SW_COUNT; i++)
    {
        uint8_t sample = (raw_bits >> i) & 1u;
        uint8_t stable = (stable_bits >> i) & 1u;

        if (sample == stable)
        {
            counters[i] = 0;
            continue;
        }
        if (++counters[i] < SWITCHES_DEBOUNCE_TICKS)
        {
            continue;
        }
        counters[i] = 0;
        stable_bits ^= 1UL << i;
        queue_push(i, sample);
    }
}

uint8_t SWITCHES_get_event(struct SWITCHES_event* event)
{
    if (queue_tail == queue_head)
    {
        return 0;
    }
    *event = event_queue[queue_tail];
    queue_tail = (uint8_t)((queue_tail + 1) % SWITCHES_EVENT_QUEUE_SIZE);
    return 1;
}

uint8_t SWITCHES_state(uint8_t index)
{
    return (stable_bits >> index) & 1u;
}

uint32_t SWITCHES_raw(void)
{
    return raw_bits;
}
