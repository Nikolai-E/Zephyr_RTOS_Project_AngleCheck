/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * By Nikolai Eidheim
 * 
 * Based on code from docs.zephyrproject.org and Steven Bos's github code: 
 * https://github.com/aiunderstand/Real-TimeOS-with-Microbit-nRF52833
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "lsm303_ll.h"
#include "matrix.h"
#include <stdbool.h>
#include <nrfx_timer.h>
#include <zephyr/timing/timing.h>

// thread initialization
#define STACKSIZE 1024
#define THREAD0_PRIORITY 5 // higest priority
#define THREAD1_PRIORITY 6

// global timer initialization
#define TIME_TO_WAIT_MS 5000UL // 5 seconds
#define TIMER_INST_IDX 0
nrfx_timer_t timer_inst = NRFX_TIMER_INSTANCE(TIMER_INST_IDX);

// message queue initialization
int msgQueue[3] = {0};
K_MSGQ_DEFINE(my_msgq, sizeof(msgQueue), 1, 4); 

// toggle counter for display patterns
int counter = 0;

// Status variables used for logic
int HasTilted = 0;
int Tiltstatus = 0;
int CriticalThresholdReached = 0;
int makeCheck = 0;

// interupt function for when the timer is done
static void timer_handler(nrf_timer_event_t event_type, void * p_context)
{
	makeCheck = 1;
}

// initialize the program
int main(void)
{
	int ret;
	ret = lsm303_ll_begin();        
	if (ret < 0)
	{
		printk("\nError initializing lsm303.  Error code = %d\n",ret);	
		while(1);
	}
	int ret2;
	ret2 = matrix_begin();
	if (ret2 < 0)
	{
		printk("\nError initializing LED matrix.  Error code = %d\n",ret2);	
		while(1);
	}

	// initialize the timer

	#if defined(__ZEPHYR__)
    	IRQ_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_TIMER_INST_GET(TIMER_INST_IDX)), IRQ_PRIO_LOWEST,
                NRFX_TIMER_INST_HANDLER_GET(TIMER_INST_IDX), 0, 0);
	#endif
	
 	uint32_t base_frequency = NRF_TIMER_BASE_FREQUENCY_GET(timer_inst.p_reg);
    nrfx_timer_config_t config = NRFX_TIMER_DEFAULT_CONFIG(base_frequency);
    config.bit_width = NRF_TIMER_BIT_WIDTH_32;
    config.p_context = "My context";

    nrfx_err_t status = nrfx_timer_init(&timer_inst, &config, timer_handler);
    NRFX_ASSERT(status == NRFX_SUCCESS);
	

	nrfx_timer_clear(&timer_inst);

    /* Creating variable desired_ticks to store the output of nrfx_timer_ms_to_ticks function */
    uint32_t desired_ticks = nrfx_timer_ms_to_ticks(&timer_inst, TIME_TO_WAIT_MS);
    
    /*
     * Setting the timer channel NRF_TIMER_CC_CHANNEL0 in the extended compare mode to stop the timer and
     * trigger an interrupt if internal counter register is equal to desired_ticks.
     */
    nrfx_timer_extended_compare(&timer_inst, NRF_TIMER_CC_CHANNEL0, desired_ticks,
                                NRF_TIMER_SHORT_COMPARE0_STOP_MASK, true);

}

void thread0(void)
{
	while (1) {
		// if Critical Threshold Reached, do nothing
		if (CriticalThresholdReached == 1) {
			return;
		}

		// read data from accelerometer
		struct All_Axes_Data data = lsm303_ll_readAccel();

		// put the data in the message queue array
		msgQueue[0] = data.X;
		msgQueue[1] = data.Y;
		msgQueue[2] = data.Z;


		// Put a message in the message queue. If the message queue is full, purge messages.
        while (k_msgq_put(&my_msgq, &msgQueue, K_NO_WAIT) != 0) {
            /* message queue is full: purge old data & try again */
            k_msgq_purge(&my_msgq);
        }


		// sleep for 3ms, which is the period
		k_msleep(9);
	}
}

void thread1(void)
{
	while (1) {

		// to check time of execution
		/*
		timing_t start_time, end_time;
		uint64_t total_cycles;
		uint64_t total_ns;

		timing_init();
		timing_start();

		start_time = timing_counter_get();
		*/


		// array to store the data from the message queue
		int ReceivedData[3] = {0};

		// checks state after 5 seconds
		if (makeCheck == 1) {
			makeCheck = 0;
			if (Tiltstatus == 1) {
				CriticalThresholdReached = 1;
			}
			else {
				HasTilted = 0;
				nrfx_timer_clear(&timer_inst);
			}
		}

		// if Critical Threshold Reached, do nothing
		if (CriticalThresholdReached == 1) {
				// critical warning leds, all leds on
				matrix_put_pattern(31, 0);
				return;
		}

		// gets data from message queue
		k_msgq_get(&my_msgq, &ReceivedData, K_FOREVER);


		// print the data to the monitor, debugging purpose
		printk("X %d\n", ReceivedData[0]);
		printk("Y = %d\n", ReceivedData[1]);
		printk("Z = %d\n", ReceivedData[2]);	  

		// Check if msgQueue[0] or msgQueue[1], X or Y, is above or below +-500
		if (ReceivedData[0] > 500 || ReceivedData[0] < -500 || ReceivedData[1] > 500 || ReceivedData[1] < -500) {
			Tiltstatus = 1;
			// led for current tilt status
			if (counter == 2)
				matrix_put_pattern(4, 27);		
			if (HasTilted == 0) {
				nrfx_timer_enable(&timer_inst);
				HasTilted = 1;
			}
		}
		else {
			Tiltstatus = 0;
			matrix_all_off();
		}
		// led for warning state
		if (counter == 0 && HasTilted == 1) {
			matrix_put_pattern(17, 0);
		} else if (counter == 1 && HasTilted == 1) {
			matrix_put_pattern(14, 14);
		}

		// counter for toggling leds to allow more leds settings at the same time
		counter++;
		counter = counter%3;






		// to check time of execution		
		/*
		end_time = timing_counter_get();
    	total_cycles = timing_cycles_get(&start_time, &end_time);
    	total_ns = timing_cycles_to_ns(total_cycles);
    	timing_stop();
		printf("ns: %llu\n", total_ns);
		*/

		// sleep for 3ms, which is the period
		k_msleep(10);

	}
}

// define the threads
K_THREAD_DEFINE(thread0_id, STACKSIZE, thread0, NULL, NULL, NULL,
		THREAD0_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread1_id, STACKSIZE, thread1, NULL, NULL, NULL,
		THREAD1_PRIORITY, 0, 0);
