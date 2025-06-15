//***************************************************************************
//
// File Name            : lcd_dog_AVR128_driver.c
// Title                : lcd_dog_AVR128_driver
// Date                 : 03/25/2024
// Version              : 1.0
// Target MCU           : AVR128DB48
// Target Hardware      ;
// Author               : Brian Kim, Andrew Nguyen
// DESCRIPTION
// Basic driver software that allows the AVR128DB48 to communicate with the
// DOGM163W-A LCD using a SPI interface.
//
// PA4	MOSI
// PA6	SCK
// PA7	/SS
// PC0	RS	(Register select for LCD)
//
//**************************************************************************

#include <avr/io.h>
#include <stdio.h>
//#define F_CPU 4000000
#include <util/delay.h>

// Display buffer for DOG LCD using sprintf()
char dsp_buff1[17];
char dsp_buff2[17];
char dsp_buff3[17];

/* Function Prototypes */
void clear_dsp(void);
void init_spi_lcd (void);
void init_lcd_dog (void);
void lcd_spi_transmit_CMD (unsigned char cmd);
void lcd_spi_transmit_DATA (unsigned char cmd);
void update_lcd_dog(void);

//******************************************************************************
// Function             : init_spi_lcd
// Date and version     : 03/26/24, version 1.0
// Target MCU           : AVR128DB48
// Author               : Brian Kim, Andrew Nguyen
// DESCRIPTION
// Initializes SPI0 pins for communication between AVR and ST7036.
//
// Modified
//******************************************************************************
void init_spi_lcd (void)
{
	PORTC.DIR |= PIN0_bm; /* Register select for LCD set to output*/
	PORTA.DIR |= PIN4_bm; /* Set MOSI pin direction to output */
	PORTA.DIR |= PIN6_bm; /* Set SCK pin direction to output */
	PORTA.DIR |= PIN7_bm; /* Set SS pin direction to output */
	//PORTA.DIR &= ~PIN5_bm; /* Set MISO pin direction to input */
	//PORTF.DIR |= PIN2_bm; /* Set as output to drive MAX5402 chip select */
	SPI0.CTRLA = SPI_ENABLE_bm | SPI_MASTER_bm; /* SPI Master mode select & Enable SPI */
	SPI0.CTRLB = 0x04; /* Slave select disable */
}

//******************************************************************************
// Function             : lcd_spi_transmit_CMD
// Date and version     : 03/26/24, version 1.0
// Target MCU           : AVR128DB48
// Author               : Brian Kim, Andrew Nguyen
// DESCRIPTION
// Function to send commands/instructions to ST7036 via SPI0. RS is set to 0 for 
// command.
//
// Modified
//******************************************************************************
void lcd_spi_transmit_CMD (unsigned char cmd)
{
	//while (!(SPI0.INTFLAGS & SPI_IF_bm)){}
	PORTC.OUT &= ~PIN0_bm; // RS = 0 for command
	PORTA.OUT &= ~PIN7_bm; // Select ST7036
	SPI0.DATA = cmd;
	while (!(SPI0.INTFLAGS & SPI_IF_bm)){} // Waits until data are exchanged
	PORTA.OUT |= PIN7_bm; // Deselect ST7036
}

//******************************************************************************
// Function             : lcd_spi_transmit_DATA
// Date and version     : 03/26/24, version 1.0
// Target MCU           : AVR128DB48
// Author               : Brian Kim, Andrew Nguyen
// DESCRIPTION
// Function to send data to ST7036 via SPI0. RS is set to 1 for data.
//
// Modified
//******************************************************************************
void lcd_spi_transmit_DATA (unsigned char cmd)
{
	//while (!(SPI0.INTFLAGS & SPI_IF_bm)) {} 
	PORTC.OUT |= PIN0_bm; // RS = 1 for data
	PORTA.OUT &= ~PIN7_bm; // Select ST7036
	SPI0.DATA = cmd;
	while (!(SPI0.INTFLAGS & SPI_IF_bm)) {} // Waits until data are exchanged
	PORTA.OUT |= PIN7_bm; // Deselect ST7036
}

//******************************************************************************
// Function             : init_lcd_dog
// Date and version     : 03/26/24, version 1.0
// Target MCU           : AVR128DB48
// Author               : Brian Kim, Andrew Nguyen
// DESCRIPTION
// Initializes DOG LCD by sending instructions via lcd_spi_transmit_CMD() 
// function.
//
// Modified
//******************************************************************************
void init_lcd_dog (void)
{
	init_spi_lcd();		//Initialize MCU for LCD SPI
	
	//start_dly_40ms:
	_delay_ms(40);   //startup delay.
	
	//func_set1:
	lcd_spi_transmit_CMD(0x39);   // send function set #1
	_delay_us(30);	//delay for command to be processed

	//func_set2:
	lcd_spi_transmit_CMD(0x39);	//send function set #2
	_delay_us(30);	//delay for command to be processed

	//bias_set:
	lcd_spi_transmit_CMD(0x1E);	//set bias value.
	_delay_us(30);	//delay for command to be processed

	//power_ctrl:
	lcd_spi_transmit_CMD(0x55);	//~ 0x50 nominal for 5V
	//~ 0x55 for 3.3V (delicate adjustment).
	_delay_us(30);	//delay for command to be processed

	//follower_ctrl:
	lcd_spi_transmit_CMD(0x6C);	//follower mode on...
	_delay_ms(40);	//delay for command to be processed

	//contrast_set:
	lcd_spi_transmit_CMD(0x7F);	//~ 77 for 5V, ~ 7F for 3.3V
	_delay_us(30);	//delay for command to be processed

	//display_on:
	lcd_spi_transmit_CMD(0x0c);	//display on, cursor off, blink off
	_delay_us(30);	//delay for command to be processed

	//clr_display:
	lcd_spi_transmit_CMD(0x01);	//clear display, cursor home
	_delay_us(30);	//delay for command to be processed

	//entry_mode:
	lcd_spi_transmit_CMD(0x06);	//clear display, cursor home
	_delay_us(30);	//delay for command to be processed
	
	
}

//******************************************************************************
// Function             : update_lcd_dog
// Date and version     : 03/26/24, version 1.0
// Target MCU           : AVR128DB48
// Author               : Brian Kim, Andrew Nguyen
// DESCRIPTION
// Updates the DOG LCD with data in the display buffers.
//
// NOTES
// This function must be called after putting new data into buffer.
//
// Modified
//******************************************************************************
void update_lcd_dog(void)
{
	init_spi_lcd();		//init SPI port for LCD.

	// send line 1 to the LCD module.
	lcd_spi_transmit_CMD(0x80);	//init DDRAM addr-ctr
	_delay_us(30);
	for (int i = 0; i < 16; i++) {
		lcd_spi_transmit_DATA(dsp_buff1[i]);
		_delay_us(30);
	}
	
	// send line 2 to the LCD module.
	lcd_spi_transmit_CMD(0x90);	//init DDRAM addr-ctr
	_delay_us(30);
	for (int i = 0; i < 16; i++) {
		lcd_spi_transmit_DATA(dsp_buff2[i]);
		_delay_us(30);
	}
	
	// send line 3 to the LCD module.
	lcd_spi_transmit_CMD(0xA0);	//init DDRAM addr-ctr
	_delay_us(30);
	for (int i = 0; i < 16; i++) {
		lcd_spi_transmit_DATA(dsp_buff3[i]);
		_delay_us(30);
	}
}
