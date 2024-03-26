/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>

/* Task handler declaration*/
xTaskHandle worker1_id;
xTaskHandle worker2_id;
xTaskHandle worker3_id;
xTimerHandle Timer_id;

xSemaphoreHandle notification_semaphore;
xSemaphoreHandle semaphore_1;
xSemaphoreHandle semaphore_2;
xSemaphoreHandle semaphore_3;
xTaskHandle manager_id;

xQueueHandle Queue_id;

#define MY_LED_1 IOPORT_CREATE_PIN(PIOC, 21)
#define MY_LED_2 IOPORT_CREATE_PIN(PIOB, 27)
#define MY_LED_3 IOPORT_CREATE_PIN(PIOC, 22)

static void my_led_task_1(void *pvParameters){
	while(1){
		if (xSemaphoreTake(semaphore_1, 100)){
			ioport_toggle_pin_level(MY_LED_1);
			delay_ms(100);
			xSemaphoreGive(semaphore_2);
		}
	}
}

static void my_led_task_2(void *pvParameters){
	while(1){
		if (xSemaphoreTake(semaphore_2, 100)){
			ioport_toggle_pin_level(MY_LED_2);
			delay_ms(100);
			xSemaphoreGive(semaphore_3);
		}
	}
}

static void my_led_task_3(void *pvParameters){
	while(1){
		if (xSemaphoreTake(semaphore_3, 100)){
			ioport_toggle_pin_level(MY_LED_3);
			delay_ms(100);
			xSemaphoreGive(semaphore_1);
		}
	}
}

/*
void TimerCallback( xTimerHandle pxtimer )
{
	//ioport_toggle_pin_level(MY_LED_2);
	//xSemaphoreGive(notification_semaphore);
	
	//xSemaphoreTake(semaphore_1, 0);
	//xSemaphoreTake(semaphore_2, 0);
	xSemaphoreGive(semaphore_1);
}
*/

/*
static void manager_task(void *pvParameters)
{
	// Create the notification semaphore and set the initial state.
	vSemaphoreCreateBinary(notification_semaphore);
	vQueueAddToRegistry(notification_semaphore, "Notification Semaphore");
	xSemaphoreTake(notification_semaphore, 0);
	while(1)
	{
		// Try to get the notification semaphore.
		// The notification semaphore is only released in the SW Timer callback
		if (xSemaphoreTake(notification_semaphore, 100))
		{
			vTaskResume(worker1_id);
			vTaskResume(worker2_id);
		}
	}
}
*/

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	sysclk_init();
	board_init();
	ioport_init();

	ioport_set_pin_dir(MY_LED_1, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(MY_LED_2, IOPORT_DIR_OUTPUT);
	
	/*
	while(1){
		ioport_toggle_pin_level(MY_LED);
		delay_ms(100);		
	}*/

	/* Insert application code here, after the board has been initialized. */
	xTaskCreate(my_led_task_1, "LED 1", configMINIMAL_STACK_SIZE+1000 , NULL, 2, &worker1_id);
	xTaskCreate(my_led_task_2, "LED 2", configMINIMAL_STACK_SIZE+1000 , NULL, 2, &worker2_id);
	xTaskCreate(my_led_task_3, "LED 3", configMINIMAL_STACK_SIZE+1000 , NULL, 2, &worker3_id);
	
	/* Create one Software Timer.*/
	//Timer_id = xTimerCreate("Timer",5000, pdTRUE, 0, TimerCallback);
	/* Start Timer.*/
	//xTimerStart( Timer_id, 0);

	vSemaphoreCreateBinary(semaphore_1);
	vQueueAddToRegistry(semaphore_1, "Semaphore 1");
	xSemaphoreTake(semaphore_1, 0);

	vSemaphoreCreateBinary(semaphore_2);
	vQueueAddToRegistry(semaphore_2, "Semaphore 2");
	xSemaphoreTake(semaphore_2, 0);

	vSemaphoreCreateBinary(semaphore_3);
	vQueueAddToRegistry(semaphore_3, "Semaphore 3");
	xSemaphoreTake(semaphore_3, 0);

	xSemaphoreGive(semaphore_1);
/*
	ioport_set_pin_level(MY_LED_1, false);
	ioport_set_pin_level(MY_LED_2, false);
	ioport_set_pin_level(MY_LED_3, false);
	delay_ms(100);

	ioport_set_pin_level(MY_LED_1, true);
	ioport_set_pin_level(MY_LED_2, true);
	ioport_set_pin_level(MY_LED_3, true);
	delay_ms(100);

	ioport_set_pin_level(MY_LED_1, false);
	ioport_set_pin_level(MY_LED_2, false);
	ioport_set_pin_level(MY_LED_3, false);
	delay_ms(100);
*/

	/* Create one manager task.*/
	//xTaskCreate(manager_task,"manager",configMINIMAL_STACK_SIZE+1000,NULL,tskIDLE_PRIORITY+3, &manager_id);

	/*Start Scheduler*/
	vTaskStartScheduler();
}
