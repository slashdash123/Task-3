/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include "stm32f4xx.h"
#include "printf.h"

int main(void)
{
	// konfiguriramo SPI - nastavimo pine
	RCC_TypeDef* p_RCC = RCC;												// ustvarimo kazalec in ga postavimo na RCC

	SET_BIT(p_RCC -> AHB1ENR, RCC_AHB1ENR_GPIOAEN ); 						// omogočimo GPIOA ((REG) |= (BIT))
	SET_BIT(p_RCC -> APB2ENR, RCC_APB2ENR_SPI1EN  ); 						// omogočimo SPI1  ((REG) |= (BIT))

	// nastavimo PA4 - PA6 na alternate function (PA7 ni potreben, ker MOSI ni potreben)
	GPIO_TypeDef* p_GPIOA = GPIOA;

	//PA4 - NSS
	CLEAR_BIT(p_GPIOA ->MODER, GPIO_MODER_MODER4_0);
	SET_BIT(p_GPIOA ->MODER, GPIO_MODER_MODER4_1);

	//PA5 - SCK
	CLEAR_BIT(p_GPIOA ->MODER, GPIO_MODER_MODER5_0);
	SET_BIT(p_GPIOA ->MODER, GPIO_MODER_MODER5_1);

	//PA6 - MISO
	CLEAR_BIT(p_GPIOA ->MODER, GPIO_MODER_MODER6_0);
	SET_BIT(p_GPIOA ->MODER, GPIO_MODER_MODER6_1);

	//preveri zakaj rabimo AF4 in AF6
	SET_BIT(p_GPIOA -> AFR[0], 5 << 16);									// če vpišemo 5 na 16. bit v AFRL1, nam vrne AF4
	SET_BIT(p_GPIOA -> AFR[0], 5 << 20);									// če vpišemo 5 na 20. bit v AFRL1, nam vrne AF5
	SET_BIT(p_GPIOA -> AFR[0], 5 << 24);									// če vpišemo 5 na 24. bit v AFRL1, nam vrne AF6

	//SPI - baud rate
	SPI_TypeDef* p_SPI1 = SPI1;

	SET_BIT(p_SPI1->CR1, SPI_CR1_MSTR);										// izberemo master kofuguracijo
	CLEAR_BIT(p_SPI1->CR1, SPI_CR1_LSBFIRST); 								// izberemo da se prvo pošlje MSB
	CLEAR_BIT(p_SPI1->CR2, 1<<4);											// nastavimo frame format 0: motorola mode, 1- TI mode
	SET_BIT( p_SPI1-> CR2, SPI_CR2_SSOE); 									/* 0: SS output is disabled in master mode and the cell can work in multimaster configuration,
																			1: SS output is enabled in master mode and when the cell is enabled.
																			The cell cannot work in a multimaster environment */
	CLEAR_BIT(p_SPI1->CR1, SPI_CR1_SSM); 									// onemogočimo slave kofiguracijo???

	SET_BIT(p_SPI1-> CR1, 3<<3);											// Nastavimo uro 16Mhz

	// če nimam TI mode morm zbrat še CPOL in CPHA
	CLEAR_BIT(p_SPI1->CR1, SPI_CR1_CPOL); 									// CLK to 0 when idle
	CLEAR_BIT(p_SPI1->CR1, SPI_CR1_CPHA); 									// 0: The first clock transition is the first data capture edge

	// nastavirev data frame formata na 16 bit
	SET_BIT(p_SPI1->CR1, SPI_CR1_DFF);
	SET_BIT(p_SPI1->CR1, SPI_CR1_RXONLY);									// nastavimo da samo bere podatke

	SET_BIT(SPI1->CR1, SPI_CR1_BR_2);										// ura nastavljena na 16 mhz/2

	// An interrupt is generated if the RXNEIE bit is set in the SPI_CR2 register ,
	// when this bit is set, the DMA request is made whenever the RXNE flag is set.
	// SET_BIT (p_SPI1->CR2,SPI_CR2_RXNEIE);

	// tu spremeni data type če bo potrebno (uint32_t) in potem še spodaj v printf
	int16_t temp;
	int16_t vrednost;

	 while (1)
	 {
		 SET_BIT(p_SPI1->CR1, SPI_CR1_SPE); 								// Enablamo SPI

		 // enable RXNE
		 while (READ_BIT(SPI1->SR,SPI_SR_RXNE) == 0); 						// ostane v tej zanki dokler ne prejema nič

		vrednost = SPI1->DR; 												// 16 bitno vrednost, ki jo je treba šiftat za 3 mesta v desno
		CLEAR_BIT(p_SPI1->CR1, SPI_CR1_SPE); 								// disable SPI
		vrednost = vrednost >>3; 											// prepišemo šiftano vrednost v novo spremenljivko

		// 2^13 = 8192 /2=4096 ... celotno merilno območje razdelimo na pol ker je lahk polovica negativno

		if (vrednost >= 4096)												// do te vrednosti daje senzor pozitivna števila, pol so pa že z 1 spredi in so negativna
		{
				// if ( vrednost & (1<<12) != 0)  							// lahko še enkrat preverimo če je negativno
				vrednost = ~(vrednost) + 1; 								// pretvorimo negativno v pozitivno
				temp = vrednost * 625; 										// pretvorba bit v °C , delimo s 10000 (celoštevilska aritmetika) zaradi decimalne vejice da se zamakne
				printf(" temp= - %d.%d\r\n", temp/10000, temp % 10000);
				// printf (temp);
		}
		else
		{
				temp = vrednost *625;
				printf(" temp=  %d.%d\r\n", temp/10000, temp % 10000);
				// printf (temp);
		 }
	 }
	 for (int i=0; i<10000; i++);											//delay
}
