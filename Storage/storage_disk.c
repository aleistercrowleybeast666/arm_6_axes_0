#include "storage_disk.h"
#include "sdio.h"
#include "board_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#define SD_TRANSFER_TIMEOUT_MS 1000U
static uint8_t dma_sector[512] __attribute__((aligned(4)));
static volatile bool rx_done, tx_done, transfer_error;
static DSTATUS media_status = STA_NOINIT;
static bool Disk_IsPresent(void)
{
    return HAL_GPIO_ReadPin(BOARD_TF_DETECT_PORT, BOARD_TF_DETECT_PIN) == GPIO_PIN_RESET;
}
static DSTATUS Disk_Init(BYTE lun)
{
    (void)lun;
    media_status = STA_NOINIT;
    if (!Disk_IsPresent())
        return media_status;
    if (HAL_SD_Init(Sd_GetHandle()) == HAL_OK &&
        HAL_SD_ConfigWideBusOperation(Sd_GetHandle(), SDIO_BUS_WIDE_4B) == HAL_OK)
        media_status = 0;
    return media_status;
}
static DSTATUS Disk_Status(BYTE lun)
{
    (void)lun;
    if (!Disk_IsPresent())
        media_status = STA_NOINIT;
    return media_status;
}
static DRESULT Disk_Wait(volatile bool *done)
{
    uint32_t began = HAL_GetTick();
    while (!*done && !transfer_error && (uint32_t)(HAL_GetTick() - began) < SD_TRANSFER_TIMEOUT_MS)
        vTaskDelay(1);
    if (!*done || transfer_error)
    {
        (void)HAL_SD_Abort(Sd_GetHandle());
        media_status = STA_NOINIT;
        return RES_ERROR;
    }
    while (HAL_SD_GetCardState(Sd_GetHandle()) != HAL_SD_CARD_TRANSFER)
    {
        if (!Disk_IsPresent() || (uint32_t)(HAL_GetTick() - began) >= SD_TRANSFER_TIMEOUT_MS)
        {
            (void)HAL_SD_Abort(Sd_GetHandle());
            media_status = STA_NOINIT;
            return RES_ERROR;
        }
        vTaskDelay(1);
    }
    return RES_OK;
}
static DRESULT Disk_Read(BYTE lun, BYTE *out, DWORD sector, UINT count)
{
    if (!out || !count)
        return RES_PARERR;
    if (Disk_Status(lun))
        return RES_NOTRDY;
    for (UINT i = 0; i < count; ++i)
    {
        rx_done = transfer_error = false;
        if (HAL_SD_ReadBlocks_DMA(Sd_GetHandle(), dma_sector, sector + i, 1) != HAL_OK)
        {
            media_status = STA_NOINIT;
            return RES_ERROR;
        }
        DRESULT r = Disk_Wait(&rx_done);
        if (r != RES_OK)
            return r;
        memcpy(out + i * 512, dma_sector, 512);
    }
    return RES_OK;
}
static DRESULT Disk_Write(BYTE lun, const BYTE *in, DWORD sector, UINT count)
{
    if (!in || !count)
        return RES_PARERR;
    if (Disk_Status(lun))
        return RES_NOTRDY;
    for (UINT i = 0; i < count; ++i)
    {
        memcpy(dma_sector, in + i * 512, 512);
        tx_done = transfer_error = false;
        if (HAL_SD_WriteBlocks_DMA(Sd_GetHandle(), dma_sector, sector + i, 1) != HAL_OK)
        {
            media_status = STA_NOINIT;
            return RES_ERROR;
        }
        DRESULT r = Disk_Wait(&tx_done);
        if (r != RES_OK)
            return r;
    }
    return RES_OK;
}
static DRESULT Disk_Ioctl(BYTE lun, BYTE command, void *buffer)
{
    if (Disk_Status(lun))
        return RES_NOTRDY;
    if (command == CTRL_SYNC)
    {
        volatile bool done = true;
        return Disk_Wait(&done);
    }
    if (!buffer)
        return RES_PARERR;
    HAL_SD_CardInfoTypeDef card;
    if (HAL_SD_GetCardInfo(Sd_GetHandle(), &card) != HAL_OK)
        return RES_ERROR;
    if (command == GET_SECTOR_COUNT)
        *(DWORD *)buffer = card.LogBlockNbr;
    else if (command == GET_SECTOR_SIZE)
        *(WORD *)buffer = 512;
    else if (command == GET_BLOCK_SIZE)
        *(DWORD *)buffer = 1;
    else
        return RES_PARERR;
    return RES_OK;
}
const Diskio_drvTypeDef *StorageDisk_GetDriver(void)
{
    static const Diskio_drvTypeDef driver = {Disk_Init, Disk_Status, Disk_Read, Disk_Write, Disk_Ioctl};
    return &driver;
}
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *sd)
{
    if (sd == Sd_GetHandle() && sd->Instance == BOARD_SD_INSTANCE)
        rx_done = true;
}
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *sd)
{
    if (sd == Sd_GetHandle() && sd->Instance == BOARD_SD_INSTANCE)
        tx_done = true;
}
void HAL_SD_ErrorCallback(SD_HandleTypeDef *sd)
{
    if (sd == Sd_GetHandle() && sd->Instance == BOARD_SD_INSTANCE)
        transfer_error = true;
}
