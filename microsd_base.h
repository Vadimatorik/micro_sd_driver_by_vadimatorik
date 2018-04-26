#pragma once

#include <stdint.h>

enum class EC_SD_RESULT {
	OK		= 0,		// 0: Successful
	ERROR	= 1,		// 1: R/W Error
	WRPRT	= 2,		// 2: Write Protected
	NOTRDY	= 3,		// 3: Not Ready
	PARERR	= 4			// 4: Invalid Parameter
};

enum class EC_SD_STATUS {
	OK			= 0,
	NOINIT		= 0x01,	/* Drive not initialized */
	NODISK		= 0x02,	/* No medium in the drive */
	PROTECT		= 0x04	/* Write protected */
};

enum class EC_MICRO_SD_TYPE {
	ERROR			= 0,
	SDSC			= 1,
	SDSD			= 2,
	SDHC_OR_SDXC	= 3
};

enum class EC_SD_RES {
	OK							= 0,
	TIMEOUT						= 1,
	IO_ERROR					= 2,					// Если низкоуровневый интерфейс (SDIO/SPI) не отработал.
	R1_ILLEGAL_COMMAND			= 3,					// Команда не поддерживается.
};

class MicrosdBase {
public:
	//**********************************************************************
	// Метод:
	// 1. Распознает тип карты.
	// 2. Инициализирует ее в соответсвии с ее типом.
	//**********************************************************************
	virtual EC_MICRO_SD_TYPE	initialize			( void )						= 0;


	/*!
	 * Возвращает тип карты, определенный initialize.
	 * Если карта не была ранее инициализирована возвращает:
	 * EC_MICRO_SD_TYPE::ERROR.
	 */
	virtual EC_MICRO_SD_TYPE	getType				( void )						= 0;

	// Считать сектор: структура карты, указатель на первый байт, куда будут помещены данные.
	// Адрес внутри microsd. В зависимости от карты: либо первого байта, откуда считать (выравнивание по 512 байт), либо адрес
	// сектора (каждый сектор 512 байт).
	virtual EC_SD_RESULT		readSector			( uint32_t sector,
													  uint8_t *target_array,
													  uint32_t cout_sector,
													  uint32_t timeout_ms	)		= 0;

	// Записать по адресу address массив src длинной 512 байт.
	virtual EC_SD_RESULT		writeSector			( const uint8_t* const source_array,
													  uint32_t sector,
													  uint32_t cout_sector,
													  uint32_t timeout_ms	)		= 0;

	virtual EC_SD_STATUS		getStatus			( void )						= 0;
};
