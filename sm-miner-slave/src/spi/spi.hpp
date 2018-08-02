/*******************************************************************************
*  file    : spi.hpp
*  created : 23.05.2013
*  author  : Slyshyk Oleksiy (alexSlyshyk@gmail.com)
*******************************************************************************/

#ifndef SPI_HPP
#define SPI_HPP
#include "spi_base.hpp"


typedef spi_base<spi_1_config> Spi1;
typedef spi_base<spi_3_config> Spi3;


//-----------------------------------------------
#if defined(USE_BRD_MB_V1)
//-----------------------------------------------

extern Spi1 spi1;
#define spiToMaster spi1

//-----------------------------------------------
#elif defined(USE_BRD_F4_DISCOVERY)
//-----------------------------------------------

extern Spi3 spi3;
#define spiToMaster spi3

//-----------------------------------------------
#else
#error "Must define board revision !"
#endif
//-----------------------------------------------

#endif // SPI_HPP
