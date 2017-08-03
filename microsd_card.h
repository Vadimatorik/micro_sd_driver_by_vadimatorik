#pragma once

#include "user_hardware_interface.h"

/*
#include "crc.h"                        // Для использования CRC7.
#include "eflib_config.h"                // FreeRTOS.
#include "eflib_config.h"
#include "port_spi.h"                    // Работа с SPI.
#include "errno.h"                        // Коды возвращаемых ошибок.
#include "port_gpio.h"                    // Для дергания CS.
#include "diskio.h"                        // Используем enum отсюда. Для повышения совместимости.
#include "port_uart.h"
#include <string.h>
#include "ff.h"
*/

enum class EC_SD_RESULT {
    OK      = 0,        // 0: Successful
    ERROR   = 1,        // 1: R/W Error
    WRPRT   = 2,        // 2: Write Protected
    NOTRDY  = 3,        // 3: Not Ready
    PARERR  = 4         // 4: Invalid Parameter
};

enum class MICRO_SD_TYPE {
    ERROR           = 0,
    SD_V2_BLOCK     = 1,
    SD_V2_BYTE      = 2,
    SD_V1           = 3,
    MMC_V3          = 4
};

// Ответы, которые приходят от microsd после приема порции в 512 байт.
enum class MICRO_SD_ANSWER_WRITE {
    DATA_IN_OK              = 0b101,            // Данные приняты.
    DATA_IN_CRC_ERROR       = 0b1011,           // CRC ошибка. Данные не приняты.
    DATA_IN_WRITE_ERROR     = 0b1101,           // Ошибка записи.
    DATA_IN_ANSWER_ERROR    = 0                 // Ответ просто не пришел.
};


// Тип возвращаемого значения полностью совместим с FATFS от chan-а FRESULT классом!
enum class EC_FRESULT{
    OK = 0,                 /* (0) Succeeded */
    DISK_ERR,               /* (1) A hard error occurred in the low level disk I/O layer */
    INT_ERR,                /* (2) Assertion failed */
    NOT_READY,              /* (3) The physical drive cannot work */
    NO_FILE,                /* (4) Could not find the file */
    NO_PATH,                /* (5) Could not find the path */
    INVALID_NAME,           /* (6) The path name format is invalid */
    DENIED,                 /* (7) Access denied due to prohibited access or directory full */
    EXIST,                  /* (8) Access denied due to prohibited access */
    INVALID_OBJECT,         /* (9) The file/directory object is invalid */
    WRITE_PROTECTED,        /* (10) The physical drive is write protected */
    INVALID_DRIVE,          /* (11) The logical drive number is invalid */
    NOT_ENABLED,            /* (12) The volume has no work area */
    NO_FILESYSTEM,          /* (13) There is no valid FAT volume */
    MKFS_ABORTED,           /* (14) The f_mkfs() aborted due to any parameter error */
    TIMEOUT,                /* (15) Could not get a grant to access the volume within defined period */
    LOCKED,                 /* (16) The operation is rejected according to the file sharing policy */
    NOT_ENOUGH_CORE,        /* (17) LFN working buffer could not be allocated */
    TOO_MANY_OPEN_FILES,    /* (18) Number of open files > _FS_LOCK */
    INVALID_PARAMETER       /* (19) Given parameter is invalid */
} ;

enum class STA {
    OK          = 0,
    NOINIT		= 0x01,	/* Drive not initialized */
    NODISK		= 0x02,	/* No medium in the drive */
    PROTECT		= 0x04	/* Write protected */
};

// Формат получаемого от micro-sd ответа.
enum class MICRO_SD_ANSWER_TYPE {
    R1 = 1,
    R3 = 3,
    R7 = 7
};


struct microsd_spi_cfg_t {
    const pin*                      const cs;             // Вывод CS, подключенный к microsd.
          uint32_t                  init_spi_baudrate;    // Скорость во время инициализации.
          uint32_t                  spi_baudrate_job;     // Скорость во время работы.
          spi_master_8bit_base*     const p_spi;
};

// Результат ожидания чего-либо.
enum class EC_RES_WAITING {
    OK          =   0,
    TIMEOUT     =   1
};

class microsd_spi {
public:
    microsd_spi ( const microsd_spi_cfg_t* const cfg );

    //**********************************************************************
    // Метод:
    // 1. Распознает тип карты.
    // 2. Инициализирует ее в соответсвии с ее типом.
    //**********************************************************************
    MICRO_SD_TYPE   initialize ( void ) const;

    // Считать сектор: структура карты, указатель на первый байт, куда будут помещены данные.
    // Адрес внутри microsd. В зависимости от карты: либо первого байта, откуда считать (выравнивание по 512 байт), либо адрес
    // сектора (каждый сектор 512 байт).
    EC_FRESULT read_sector ( uint8_t *dst, uint32_t address ) const;

    // Записать по адресу address массив src длинной 512 байт.
    EC_FRESULT write_sector ( uint32_t address, uint8_t *src ) const;

    // По номеру сектора и типу SD решаем, какого типа адресация и возвращаем  адрес, который нужно передать в microsd.
    uint32_t get_address_for_sd_card ( uint32_t sector ) const;

    STA             microsd_card_get_card_info ( void ) const;
    EC_SD_RESULT    get_CSD ( uint8_t *src ) const;

private:
    // Переключение CS.
    void    cs_low                          ( void ) const;       // CS = 0, GND.
    void    cs_high                         ( void ) const;       // CS = 1, VDD.

    // Передать count пустых байт (шлем 0xFF).
    void    send_empty_package              ( uint16_t count ) const;

    // Передача 1 пустого байта. Требуется после каждой команды для ОЧЕНЬ старых карт.
    void    send_wait_package               ( void ) const;

    // Пропускаем count приших байт.
    void    lose_package                    ( uint16_t count ) const;

    // Переводим micro-sd в режим SPI.
    void    init_spi_mode                   ( void ) const;

    // Просто передача команды.
    void    send_cmd                        ( uint8_t cmd, uint32_t arg, uint8_t crc ) const;
    // Отправить ACMD.
    EC_RES_WAITING  send_acmd ( uint8_t acmd, uint32_t arg, uint8_t crc ) const;


    // Ждать R1.
    EC_RES_WAITING    wait_r1               ( void ) const;
    EC_RES_WAITING    wait_r1               ( uint8_t* r1 ) const;

    // Принимаем R3 (регистр OCR).
    EC_RES_WAITING    wait_r3               ( uint32_t* r3 ) const;

    // Принимаем r7.
    EC_RES_WAITING    wait_r7               ( uint32_t* r7 ) const;
        const microsd_spi_cfg_t* const cfg;
    MICRO_SD_TYPE       type_microsd = MICRO_SD_TYPE::ERROR;           // Тип microSD.

    USER_OS_STATIC_MUTEX_BUFFER     mutex_buf = USER_OS_STATIC_MUTEX_BUFFER_INIT_VALUE;
    USER_OS_STATIC_MUTEX            mutex =     nullptr;
};
