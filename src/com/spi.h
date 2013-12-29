//
//  spi.h
//  Ethernet Shield
//
//  Created by EFCM van der Werf on 12/28/13.
//  Copyright (c) 2013 EFCM van der Werf. All rights reserved.
//

#ifndef COM_SPI_H
#define COM_SPI_H

#include "../config.h"

// Do we want SPI?
#ifdef COM_SPI

#include <inttypes.h>

/**
 * SPI config
 */
typedef struct spi_config {
    
} spi_config_t;

/**
 * @brief Initialize SPI channel
 * @param config Configuration for spi channel
 */
extern void spi_init(spi_config_t *config);

#define SPI_ACTIVE(port, pin)  (port) &= ~(1 << (pin))
#define SPI_PASSIVE(port, pin) (port) |=  (1 << (pin))
#define SPI_WAIT() while(!(SPSR&(1<<SPIF)))

#endif // COM_SPI
#endif // COM_SPI_H
