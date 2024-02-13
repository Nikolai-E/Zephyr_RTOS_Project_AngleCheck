//Adapted from: 
//https://github.com/fduignan/zephyr_bbc_microbit_v2/tree/main/zephyr_3.1.0/led_matrix/src

//Note:
//This driver is rather simple. Some LED patterns cannot be displayed without the concept of 
//persistance of vision. Since we have 5 rows and 5 columns at least 5 cycles are needed to //display all possible LED patterns. 

//More advanced display drivers would have more cycles to control the brightness of individual pixels with PWM signals, see
//https://docs.broadcom.com/doc/AV02-3697EN or the implementation in Zephyr in
//https://github.com/zephyrproject-rtos/zephyr/blob/main/samples/boards/nrf/nrf_led_matrix/src/main.c
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include "matrix.h"
#define ROW1_PORT_BIT 21
#define ROW2_PORT_BIT 22
#define ROW3_PORT_BIT 15
#define ROW4_PORT_BIT 24
#define ROW5_PORT_BIT 19

#define COL1_PORT_BIT 28
#define COL2_PORT_BIT 11
#define COL3_PORT_BIT 31
#define COL4_PORT_BIT 5
#define COL5_PORT_BIT 30

static const struct device *gpio0, *gpio1;		
void set_row1(int state)
{
	gpio_pin_set(gpio0,ROW1_PORT_BIT,state);
}
void set_row2(int state)
{
	gpio_pin_set(gpio0,ROW2_PORT_BIT,state);
}
void set_row3(int state)
{
	gpio_pin_set(gpio0,ROW3_PORT_BIT,state);
}
void set_row4(int state)
{
	gpio_pin_set(gpio0,ROW4_PORT_BIT,state);
}
void set_row5(int state)
{
	gpio_pin_set(gpio0,ROW5_PORT_BIT,state);
}
void set_col1(int state)
{
	gpio_pin_set(gpio0,COL1_PORT_BIT,state);
}
void set_col2(int state)
{
	gpio_pin_set(gpio0,COL2_PORT_BIT,state);
}
void set_col3(int state)
{
	gpio_pin_set(gpio0,COL3_PORT_BIT,state);
}
void set_col4(int state)
{
	gpio_pin_set(gpio1,COL4_PORT_BIT,state);
}
void set_col5(int state)
{
	gpio_pin_set(gpio0,COL5_PORT_BIT,state);
}
int matrix_begin()
{
	int ret;
	// Configure the GPIO's 	
	gpio0=DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (gpio0 == NULL)
	{
		printf("Error acquiring GPIO 0 interface\n");
		return -1;
	}
	gpio1=DEVICE_DT_GET(DT_NODELABEL(gpio1));
	if (gpio0 == NULL)
	{
		printf("Error acquiring GPIO 1 interface\n");
 		return -2;
	}
	ret = gpio_pin_configure(gpio0,ROW1_PORT_BIT,GPIO_OUTPUT);
	ret = gpio_pin_configure(gpio0,ROW2_PORT_BIT,GPIO_OUTPUT);
	ret = gpio_pin_configure(gpio0,ROW3_PORT_BIT,GPIO_OUTPUT);
	ret = gpio_pin_configure(gpio0,ROW4_PORT_BIT,GPIO_OUTPUT);
	ret = gpio_pin_configure(gpio0,ROW5_PORT_BIT,GPIO_OUTPUT);
	
	ret = gpio_pin_configure(gpio0,COL1_PORT_BIT,GPIO_OUTPUT);
	ret = gpio_pin_configure(gpio0,COL2_PORT_BIT,GPIO_OUTPUT);
	ret = gpio_pin_configure(gpio0,COL3_PORT_BIT,GPIO_OUTPUT);
	ret = gpio_pin_configure(gpio1,COL4_PORT_BIT,GPIO_OUTPUT);
	ret = gpio_pin_configure(gpio0,COL5_PORT_BIT,GPIO_OUTPUT);
	
	matrix_all_off();	
	return 0;	
}
void matrix_put_pattern(uint8_t rows, uint8_t cols)
{
	set_row1(rows & 1);
	rows = rows >> 1;
	set_row2(rows & 1);
	rows = rows >> 1;
	set_row3(rows & 1);
	rows = rows >> 1;
	set_row4(rows & 1);
	rows = rows >> 1;
	set_row5(rows & 1);
	
	set_col1(cols & 1);
	cols = cols >> 1;
	set_col2(cols & 1);
	cols = cols >> 1;
	set_col3(cols & 1);
	cols = cols >> 1;
	set_col4(cols & 1);
	cols = cols >> 1;
	set_col5(cols & 1);
}
void matrix_all_off()
{
	set_row1(0);
	set_row2(0);
	set_row3(0);
	set_row4(0);
	set_row5(0);
	
	set_col1(1);
	set_col2(1);
	set_col3(1);
	set_col4(1);
	set_col5(1);
	
}