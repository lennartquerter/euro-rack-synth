/*
******************************************************************************
* @file           : settings.c
* @author         : Lennart Querter
* @brief          : Flash persistence for masks/root/transpose/calibration
******************************************************************************
*/

#include "app/settings.h"

#include <string.h>

#include "app/quantize.h"

#define SETTINGS_FLASH_BASE   0x08060000UL // sector 7, last 128 KB of 512 KB
#define SETTINGS_FLASH_SIZE   0x20000UL
#define SETTINGS_FLASH_SECTOR FLASH_SECTOR_7

#define SETTINGS_MAGIC   0x32565A51UL // "QZV2"
#define SETTINGS_VERSION 1

struct SETTINGS_record_header
{
    uint32_t magic;
    uint16_t version;
    uint16_t length; // sizeof(struct SETTINGS_data) at write time
};

static struct SETTINGS_data settings;
static uint8_t loaded_from_flash;
static uint8_t dirty;
static uint32_t dirty_since_ms;

static uint32_t next_free_offset;
static uint8_t sector_needs_erase;

// ADC: Vin = 5*Vbias - 4*Vadc with Vbias 1.815V, Vadc = code * 3.3/4095
#define CAL_DEFAULT_ADC_OFFSET_V  9.075f
#define CAL_DEFAULT_ADC_V_PER_LSB (-4.0f * 3.3f / 4095.0f)
// DAC: 0-10V over the full 12-bit range (trimmed with RV1/RV2)
#define CAL_DEFAULT_DAC_OFFSET    0.0f
#define CAL_DEFAULT_DAC_CODE_PER_V (4095.0f / 10.0f)

static uint32_t crc32_calc(const uint8_t* data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFFUL;
    for (uint32_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            crc = (crc >> 1) ^ (0xEDB88320UL & (0UL - (crc & 1)));
        }
    }
    return ~crc;
}

// header + data + crc, padded to word alignment for flash programming
static uint32_t record_size(uint16_t data_length)
{
    uint32_t size = sizeof(struct SETTINGS_record_header) + data_length + 4;
    return (size + 3) & ~3UL;
}

#define SETTINGS_RECORD_SIZE \
    ((sizeof(struct SETTINGS_record_header) + sizeof(struct SETTINGS_data) + 4 + 3) & ~3UL)

void SETTINGS_load_defaults(void)
{
    memset(&settings, 0, sizeof(settings));
    for (uint8_t ch = 0; ch < 2; ch++)
    {
        settings.note_mask[ch] = QUANTIZE_SCALE_CHROMATIC;
        settings.root[ch] = 0;
        settings.transpose[ch] = 0;
        settings.cal[ch].adc_offset_v = CAL_DEFAULT_ADC_OFFSET_V;
        settings.cal[ch].adc_v_per_lsb = CAL_DEFAULT_ADC_V_PER_LSB;
        settings.cal[ch].dac_offset_code = CAL_DEFAULT_DAC_OFFSET;
        settings.cal[ch].dac_code_per_v = CAL_DEFAULT_DAC_CODE_PER_V;
        settings.seq_loaded[ch] = 0;
    }
    settings.mode_override = SETTINGS_MODE_AUTO;
    settings.i2c_instance = 0;
    settings.seq_bpm = 120;
}

void SETTINGS_init(void)
{
    SETTINGS_load_defaults();
    loaded_from_flash = 0;
    dirty = 0;
    sector_needs_erase = 0;
    next_free_offset = 0;

    // walk the record chain, keep the last one with a valid CRC
    uint32_t offset = 0;
    while (offset + sizeof(struct SETTINGS_record_header) <= SETTINGS_FLASH_SIZE)
    {
        struct SETTINGS_record_header header;
        memcpy(&header, (const void*)(SETTINGS_FLASH_BASE + offset), sizeof(header));

        if (header.magic == 0xFFFFFFFFUL)
        {
            break; // erased space: end of chain
        }
        if (header.magic != SETTINGS_MAGIC ||
            header.length == 0 ||
            offset + record_size(header.length) > SETTINGS_FLASH_SIZE)
        {
            // corrupted chain: don't trust anything past this point
            sector_needs_erase = 1;
            break;
        }

        const uint8_t* payload =
            (const uint8_t*)(SETTINGS_FLASH_BASE + offset + sizeof(header));
        uint32_t stored_crc;
        memcpy(&stored_crc, payload + header.length, sizeof(stored_crc));

        if (header.version == SETTINGS_VERSION &&
            header.length == sizeof(struct SETTINGS_data) &&
            stored_crc == crc32_calc(payload, header.length))
        {
            memcpy(&settings, payload, sizeof(settings));
            loaded_from_flash = 1;
        }

        offset += record_size(header.length);
    }
    next_free_offset = offset;
}

struct SETTINGS_data* SETTINGS_get(void)
{
    return &settings;
}

uint8_t SETTINGS_loaded_from_flash(void)
{
    return loaded_from_flash;
}

void SETTINGS_mark_dirty(void)
{
    dirty = 1;
    dirty_since_ms = HAL_GetTick();
}

uint8_t SETTINGS_is_dirty(void)
{
    return dirty;
}

static HAL_StatusTypeDef erase_sector(void)
{
    FLASH_EraseInitTypeDef erase = {0};
    uint32_t sector_error = 0;

    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector = SETTINGS_FLASH_SECTOR;
    erase.NbSectors = 1;

    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &sector_error);
    if (status == HAL_OK)
    {
        next_free_offset = 0;
        sector_needs_erase = 0;
    }
    return status;
}

HAL_StatusTypeDef SETTINGS_save(void)
{
    uint8_t stage[SETTINGS_RECORD_SIZE];
    struct SETTINGS_record_header header;

    header.magic = SETTINGS_MAGIC;
    header.version = SETTINGS_VERSION;
    header.length = sizeof(struct SETTINGS_data);

    memset(stage, 0xFF, sizeof(stage));
    memcpy(stage, &header, sizeof(header));
    memcpy(stage + sizeof(header), &settings, sizeof(settings));
    uint32_t crc = crc32_calc((const uint8_t*)&settings, sizeof(settings));
    memcpy(stage + sizeof(header) + sizeof(settings), &crc, sizeof(crc));

    HAL_StatusTypeDef status = HAL_FLASH_Unlock();
    if (status != HAL_OK)
    {
        return status;
    }

    if (sector_needs_erase ||
        next_free_offset + sizeof(stage) > SETTINGS_FLASH_SIZE)
    {
        status = erase_sector();
        if (status != HAL_OK)
        {
            HAL_FLASH_Lock();
            return status;
        }
    }

    uint32_t address = SETTINGS_FLASH_BASE + next_free_offset;
    for (uint32_t i = 0; i < sizeof(stage) && status == HAL_OK; i += 4)
    {
        uint32_t word;
        memcpy(&word, stage + i, sizeof(word));
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + i, word);
    }
    HAL_FLASH_Lock();

    if (status == HAL_OK)
    {
        next_free_offset += sizeof(stage);
        loaded_from_flash = 1;
        dirty = 0;
    }
    return status;
}

uint8_t SETTINGS_poll_autosave(uint32_t debounce_ms)
{
    if (!dirty || (HAL_GetTick() - dirty_since_ms) < debounce_ms)
    {
        return 0;
    }
    return SETTINGS_save() == HAL_OK;
}
