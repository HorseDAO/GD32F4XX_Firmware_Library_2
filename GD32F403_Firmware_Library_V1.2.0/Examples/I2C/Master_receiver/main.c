/*!
    \file  main.c
    \brief master receiver
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-02-10, V1.0.1, firmware for GD32F403
*/

#include "gd32f403.h"

#define I2C_10BIT_ADDRESS      0

#define I2C0_OWN_ADDRESS7      0x72
#define I2C0_SLAVE_ADDRESS7    0x82
#define I2C0_SLAVE_ADDRESS10   0x0322

uint8_t i2c_receiver[16];

void rcu_config(void);
void gpio_config(void);
void i2c_config(void);

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    uint8_t i;
    uint8_t slave10_first_byte,slave10_second_byte;
    
    /* RCU config */
    rcu_config();
    /* GPIO config */
    gpio_config();
    /* I2C config */
    i2c_config();

    /* wait until I2C bus is idle */
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    
#if I2C_10BIT_ADDRESS
    slave10_first_byte = (0xF0) | (uint8_t)((I2C0_SLAVE_ADDRESS10 & 0x0300)>>7);
    /* send slave address first byte to I2C bus */
    i2c_master_addressing(I2C0, slave10_first_byte, I2C_TRANSMITTER);
    /* wait until ADD10SEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADD10SEND));
    /* the second byte contains the remaining 8 bits of the 10-bit address */
    slave10_second_byte = (uint8_t)(I2C0_SLAVE_ADDRESS10 & 0x00FF);
    /* send slave address 2nd byte to I2C bus */
    i2c_master_addressing(I2C0, slave10_second_byte, I2C_TRANSMITTER);
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    /* send slave address first byte to I2C bus */
    i2c_master_addressing(I2C0, slave10_first_byte, I2C_RECEIVER);
#else
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, I2C0_SLAVE_ADDRESS7, I2C_RECEIVER);
#endif
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    
    for(i=0; i<16; i++){
        if(13 == i){
            /* wait until the second last data byte is received into the shift register */
            while(!i2c_flag_get(I2C0, I2C_FLAG_BTC));
            /* disable acknowledge */
            i2c_ack_config(I2C0, I2C_ACK_DISABLE);
        }
        /* wait until the RBNE bit is set */
        while(!i2c_flag_get(I2C0, I2C_FLAG_RBNE));
        /* read a data from I2C_DATA */
        i2c_receiver[i] = i2c_data_receive(I2C0);
    }
    /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C0);
    while(I2C_CTL0(I2C0)&0x0200);
    /* enable acknowledge */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);

    while(1){
    }
}

/*!
    \brief      enable the peripheral clock
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    /* enable GPIOB clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    /* enable I2C0 clock */
    rcu_periph_clock_enable(RCU_I2C0);
}

/*!
    \brief      cofigure the GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none
*/
void gpio_config(void)
{
    /* connect PB6 to I2C0_SCL */
    /* connect PB7 to I2C0_SDA */
    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);
}

/*!
    \brief      cofigure the I2C0 and I2C1 interfaces
    \param[in]  none
    \param[out] none
    \retval     none
*/
void i2c_config(void)
{
    /* I2C clock configure */
    i2c_clock_config(I2C0, 400000, I2C_DTCY_2);
    /* I2C address configure */
    i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, I2C0_OWN_ADDRESS7);
    /* enable I2C0 */
    i2c_enable(I2C0);
    /* enable acknowledge */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);
}
