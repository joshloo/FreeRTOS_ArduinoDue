/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include <uart.h>

#define MY_LED_1 IOPORT_CREATE_PIN(PIOC, 21)
#define MY_LED_2 IOPORT_CREATE_PIN(PIOB, 27)
#define MY_LED_3 IOPORT_CREATE_PIN(PIOC, 22)

uint8_t *received_byte;

/* Queue handler declaration*/
xQueueHandle Queue_id;

/* Semaphore handler declaration*/
xSemaphoreHandle semaphore_uart_tx;
xSemaphoreHandle semaphore_uart_rx;
xSemaphoreHandle semaphore_uart_tx_a;
xSemaphoreHandle semaphore_uart_tx_b;
xSemaphoreHandle notification_semaphore;
xSemaphoreHandle semaphore_1;
xSemaphoreHandle semaphore_2;
xSemaphoreHandle semaphore_3;
xSemaphoreHandle semaphore_task_done;

/* Task handler declaration*/
xTaskHandle worker1_id;
xTaskHandle worker2_id;
xTaskHandle worker3_id;
xTaskHandle worker_rx_id;
xTaskHandle worker_tx_id;
xTaskHandle worker_tx_a_id;
xTaskHandle worker_tx_b_id;
xTaskHandle manager_id;
xTaskHandle worker_sample_id;

/* Timer handler declaration*/
xTimerHandle Timer_id;

void vTask_sampleInput(void *pvParameters) {
	char c;
	while (1){
		// if task is started or other task done,
		// take all semaphore make sure no task is running
		if (xSemaphoreTake(semaphore_task_done, 100)) {
			xSemaphoreTake(semaphore_1, 0);
			xSemaphoreTake(semaphore_2, 0);
			xSemaphoreTake(semaphore_3, 0);
			xSemaphoreTake(semaphore_uart_tx, 0);
			xSemaphoreTake(semaphore_uart_rx, 0);
			xSemaphoreTake(semaphore_uart_tx_a, 0);
			xSemaphoreTake(semaphore_uart_tx_b, 0);

			char getchar ;
			usart_serial_getchar(UART, &getchar);
			switch (getchar){
				case ('1'):
					// LED blink
					xSemaphoreGive(semaphore_1);
					break;
				case ('2'):
					// Echo server
					xSemaphoreGive(semaphore_uart_rx);
					break;
				case ('4'):
					// Print ABC abc
					xSemaphoreGive(semaphore_uart_tx_a);
					break;
				default:
					xSemaphoreGive(semaphore_task_done);
					break;
			}			
		}
	}
}

void vTaskRX(void *pvParameters) {
	char getchar;
	while (1){
		if (xSemaphoreTake(semaphore_uart_rx, 100)){
			usart_serial_getchar(UART, &getchar);
			// '3' indicates exit condition for echo server
			if (getchar == '3') xSemaphoreGive(semaphore_task_done);
			else {
				if (Queue_id != 0) xQueueSend(Queue_id, (void *)&getchar, 1000);
				//ioport_toggle_pin_level(MY_LED_1);
				//usart_serial_putchar(UART, getchar);
				xSemaphoreGive(semaphore_uart_tx);
			}
		}
	}	
}

void vTaskTX(void *pvParameters) {
	char c;
	while (1){
		if (xSemaphoreTake(semaphore_uart_tx, 100)){
			uint8_t *putchar ;
			if (Queue_id != 0) xQueueReceive(Queue_id,&putchar,1000);

			usart_serial_putchar(UART, putchar);
			//ioport_toggle_pin_level(MY_LED_2);
			xSemaphoreGive(semaphore_uart_rx);
		}
	}
}

// Swap if and for lines for different sequencing. bytes or chunk
// Put the semaphore give in or outside the for loop
void vTaskTX_A(void *pvParameters) {
	char c;
	while (1){
		if (xSemaphoreTake(semaphore_uart_tx_a, 100)){
			for (c = 'a'; c <= 'z'; c++){
				usart_serial_putchar(UART, c);
			}
			xSemaphoreGive(semaphore_uart_tx_b);
		}
	}
}

void vTaskTX_B(void *pvParameters) {
	char c;
	while (1){
		if (xSemaphoreTake(semaphore_uart_tx_b, 100)){
			for (c = 'A'; c <= 'Z'; c++){
				usart_serial_putchar(UART, c);
			}
			//xSemaphoreGive(semaphore_uart_tx_a);
			xSemaphoreGive(semaphore_task_done);
		}
	}
}

static usart_serial_options_t usart_options = {
	.baudrate = 9600,
	.charlength = US_MR_CHRL_8_BIT,
	.paritytype = US_MR_PAR_NO,
	.stopbits = US_MR_NBSTOP_1_BIT
};

static void my_led_task_1(void *pvParameters){
	while(1){
		if (xSemaphoreTake(semaphore_1, 100)){
			ioport_toggle_pin_level(MY_LED_1);
			delay_ms(200);
			xSemaphoreGive(semaphore_2);
		}
	}
}

static void my_led_task_2(void *pvParameters){
	while(1){
		if (xSemaphoreTake(semaphore_2, 100)){
			ioport_toggle_pin_level(MY_LED_2);
			delay_ms(200);
			xSemaphoreGive(semaphore_3);
		}
	}
}

static void my_led_task_3(void *pvParameters){
	while(1){
		if (xSemaphoreTake(semaphore_3, 100)){
			ioport_toggle_pin_level(MY_LED_3);
			delay_ms(200);
			xSemaphoreGive(semaphore_task_done);
			//xSemaphoreGive(semaphore_1);
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

	/* Insert application code here, after the board has been initialized. */
	ioport_set_pin_dir(MY_LED_1, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(MY_LED_2, IOPORT_DIR_OUTPUT);

	usart_serial_init(UART, &usart_options);

	char *s = "Welcome to my FreeRTOS app.\r\nPress key \"1\" to run LED apps.\r\nPress key \"2\" to run echo server (key 3 to quit).\r\nPress key \"4\" to run abcABC prints.\r\n";
	while (*s != '\0' ){
		usart_putchar(UART, *(s));
		s++;
	}

	/* Task lists */
	xTaskCreate(my_led_task_1, "LED 1", configMINIMAL_STACK_SIZE+1000 , NULL, 2, &worker1_id);
	xTaskCreate(my_led_task_2, "LED 2", configMINIMAL_STACK_SIZE+1000 , NULL, 2, &worker2_id);
	xTaskCreate(my_led_task_3, "LED 3", configMINIMAL_STACK_SIZE+1000 , NULL, 2, &worker3_id);
	
	xTaskCreate(vTaskRX, "RX", configMINIMAL_STACK_SIZE+1000 , NULL, 2, &worker_rx_id);
	xTaskCreate(vTaskTX, "TX", configMINIMAL_STACK_SIZE+1000 , NULL, 2, &worker_tx_id);

	xTaskCreate(vTaskTX_A, "TX_A", configMINIMAL_STACK_SIZE+1000 , NULL, 2, &worker_tx_a_id);
	xTaskCreate(vTaskTX_B, "TX_B", configMINIMAL_STACK_SIZE+1000 , NULL, 2, &worker_tx_b_id);

	/* Semaphore lists */
	// LED semaphores
	vSemaphoreCreateBinary(semaphore_1);
	vQueueAddToRegistry(semaphore_1, "Semaphore 1");
	xSemaphoreTake(semaphore_1, 0);

	vSemaphoreCreateBinary(semaphore_2);
	vQueueAddToRegistry(semaphore_2, "Semaphore 2");
	xSemaphoreTake(semaphore_2, 0);

	vSemaphoreCreateBinary(semaphore_3);
	vQueueAddToRegistry(semaphore_3, "Semaphore 3");
	xSemaphoreTake(semaphore_3, 0);

	// UART TX RX echoserver semaphore
	vSemaphoreCreateBinary(semaphore_uart_rx);
	vQueueAddToRegistry(semaphore_uart_rx, "Semaphore rx");
	xSemaphoreTake(semaphore_uart_rx, 0);

	vSemaphoreCreateBinary(semaphore_uart_tx);
	vQueueAddToRegistry(semaphore_uart_tx, "Semaphore tx");
	xSemaphoreTake(semaphore_uart_tx, 0);

	// UART ABC abc printing semaphore
	vSemaphoreCreateBinary(semaphore_uart_tx_a);
	vQueueAddToRegistry(semaphore_uart_tx_a, "Semaphore tx a");
	xSemaphoreTake(semaphore_uart_tx_a, 0);

	vSemaphoreCreateBinary(semaphore_uart_tx_b);
	vQueueAddToRegistry(semaphore_uart_tx_b, "Semaphore tx b");
	xSemaphoreTake(semaphore_uart_tx_b, 0);

	vSemaphoreCreateBinary(semaphore_task_done);
	vQueueAddToRegistry(semaphore_task_done, "Semaphore Task done");
	xSemaphoreTake(semaphore_task_done, 0);

	// Deciding which semaphore to run
	xSemaphoreGive(semaphore_task_done);
	//xSemaphoreGive(semaphore_1);
	//xSemaphoreGive(semaphore_uart_rx);
	//xSemaphoreGive(semaphore_uart_tx_a);

	xTaskCreate(vTask_sampleInput, "Sample", configMINIMAL_STACK_SIZE+1000 , NULL, 2, &worker_sample_id);

	/* Create a queue*/
	Queue_id = xQueueCreate(1,sizeof(uint8_t *));

	/*Start Scheduler*/
	vTaskStartScheduler();

	/* Test codes go here */
	/*
	// Test LED
	while(1){
		ioport_toggle_pin_level(MY_LED_1);
		delay_ms(100);		
	}*/

	/*
	// Test UART
	while(1){
		usart_serial_getchar(UART, &received_byte);
		usart_serial_putchar(UART, received_byte);		
	}*/

	/*
	// Test LED toggle
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

	/* Create one Software Timer.*/
	//Timer_id = xTimerCreate("Timer",5000, pdTRUE, 0, TimerCallback);
	/* Start Timer.*/
	//xTimerStart( Timer_id, 0);
}
