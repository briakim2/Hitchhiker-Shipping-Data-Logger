//The header "i2c.h" has to be implemented for your own platform to  conform the following protocol :

#include <avr/io.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif
	
enum i2c_transfer_direction {
	I2C_TRANSFER_WRITE = 0,
	I2C_TRANSFER_READ  = 1,
};

enum status_code {
	STATUS_OK           = 0x00,
	STATUS_ERR_OVERFLOW	= 0x01,
	STATUS_ERR_TIMEOUT  = 0x02,
};

struct i2c_master_packet {
	// Address to slave device
	uint16_t address;
	// Length of data array
	uint16_t data_length;
	// Data array containing all data to be transferred
	uint8_t *data;
};

void i2c_master_init(void);
enum status_code i2c_master_read_packet_wait(struct i2c_master_packet *const packet);
enum status_code i2c_master_write_packet_wait(struct i2c_master_packet *const packet);
enum status_code i2c_master_write_packet_wait_no_stop(struct i2c_master_packet *const packet);

void i2c_master_init(void)
{
	TWI0.MBAUD = 0x01;		//Want 400kHz, but to get it BAUD value would be negative. 4MHz main clock -> ~400KHz I2C clock
	TWI0.MCTRLA = 0x01;		//Enable TWI master bit0
	TWI0.DBGCTRL = 0x01;	//Enable debug while TWI enabled
	TWI0.MSTATUS = 0x01;	//Force bus state to idle
}

// return????????????????????????
enum status_code i2c_master_read_packet_wait(struct i2c_master_packet *const packet)
{
	while((TWI0.MSTATUS & 0x03) != 0x01) ;	// wait until idle
		
	TWI0.MADDR = ((packet->address) << 1) | I2C_TRANSFER_READ; // send slave address and read command
		
	while((TWI0.MSTATUS & 0x80) == 0);		// RIF flag, wait until byte is received
		
	if ((packet->data_length) != 0 && (packet->data) != NULL)
	{
		for (uint8_t i = 0; i < (packet->data_length); i++)		//for count amount of bytes
		{
			(packet->data)[i] = TWI0.MDATA;			//read data
			TWI0.MCTRLB = 0x02;					//MCMD - issue ack followed by a byte read operation
			while((TWI0.MSTATUS & 0x80) == 0);		// RIF flag, wait until byte is received
		}
			
		TWI0.MCTRLB = 0x07;		//MCMD issue nack followed by a stop
		
		return STATUS_OK;
	}
	else
	{
		return STATUS_OK;
	}
};

enum status_code i2c_master_write_packet_wait(struct i2c_master_packet *const packet)
{
	while((TWI0.MSTATUS & 0x03) != 0x01) ; /* wait until idle */
		
	TWI0.MADDR = ((packet->address) << 1) | I2C_TRANSFER_WRITE; // send slave address and write command

	while((TWI0.MSTATUS & 0x40) == 0); /* WIF flag, wait until slave sent */
		
	if ((packet->data_length) != 0 && (packet->data) != NULL)
	{
		for (uint8_t i = 0; i < (packet->data_length); i++)		//for count amount of bytes
		{
			TWI0.MDATA = (packet->data)[i];			//write data
			while((TWI0.MSTATUS & 0x40) == 0); /* WIF flag, wait until reg_RTC sent */
		}
			
		TWI0.MCTRLB |= 0x03;		/* issue a stop */
			
		return STATUS_OK;
	}
	else
	{
		return STATUS_OK;
	}
};


enum status_code i2c_master_write_packet_wait_no_stop(struct i2c_master_packet *const packet)
{
	while((TWI0.MSTATUS & 0x03) != 0x01) ; /* wait until idle */
		
	TWI0.MADDR = ((packet->address) << 1) | I2C_TRANSFER_WRITE; // send slave address and write command

	while((TWI0.MSTATUS & 0x40) == 0); /* WIF flag, wait until slave sent */
		
	if ((packet->data_length) != 0 && (packet->data) != NULL)
	{
		for (uint8_t i = 0; i < (packet->data_length); i++)		//for count amount of bytes
		{
			TWI0.MDATA = (packet->data)[i];			//write data
			while((TWI0.MSTATUS & 0x40) == 0); /* WIF flag, wait until reg_RTC sent */
		}
			
		TWI0.MCTRLB |= 0x00;		/* issue a nack, no stop */
			
		return STATUS_OK;
	}
	else
	{
		return STATUS_OK;
	}
};