/* Includes ------------------------------------------------------------------*/
#include "main.h"
//#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#define ARM_MATH_CM4
#include "stdlib.h"
#include "string.h"
#include "semphr.h"

// create task defines
TaskHandle_t HPThandler;
void HPT_TASK (void *pvParameters);

TaskHandle_t MPThandler;
void MPT_TASK (void *pvParameters);

TaskHandle_t LPThandler;
void LPT_TASK (void *pvParameters);

TaskHandle_t VLPThandler;
void VLPT_TASK (void *pvParameters);

// semaphore related
SemaphoreHandle_t CountingSem;

// resource related
int resource[3] = {111,222,333};
int indx = 0;

// uart related
uint8_t rx_data = 0;
UART_HandleTypeDef huart2;

void InitializeCountingSemaphore(void) {

    CountingSem = xSemaphoreCreateCounting(3, 0);
    if (CountingSem == NULL) {
        HAL_UART_Transmit(&huart2, (uint8_t *) "Unable to Create Semaphore\n\n", 28, 100);
    } else {
        HAL_UART_Transmit(&huart2, (uint8_t *) "Counting Semaphore created successfully\n\n", 41, 1000);
    }
}


// create TASKS
int main(void)
{
  xTaskCreate(HPT_TASK, "HPT", 128, NULL, 3, &HPThandler);
  xTaskCreate(MPT_TASK, "MPT", 128, NULL, 2, &MPThandler);
  xTaskCreate(LPT_TASK, "LPT", 128, NULL, 1, &LPThandler);
  xTaskCreate(VLPT_TASK, "VLPT", 128, NULL, 0, &VLPThandler);

  vTaskStartScheduler();
}
  void HPT_TASK (void *pvParameters)
  {
  	char sresource[3];
  	int semcount = 0;
  	char ssemcount[2];


  	// Give 3 semaphores at the beginning..
  	xSemaphoreGive(CountingSem);
  	xSemaphoreGive(CountingSem);
  	xSemaphoreGive(CountingSem);

  	while (1)
  	{
  		char str[150];
  		strcpy(str, "Entered HPT Task\n About to ACQUIRE the Semaphore\n ");
  		semcount = uxSemaphoreGetCount(CountingSem);
  		itoa (semcount, ssemcount, 10);
  		strcat (str, "Tokens available are: ");
  		strcat (str, ssemcount);
  		strcat (str, "\n\n");
  		HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen (str), HAL_MAX_DELAY);

  		xSemaphoreTake(CountingSem, portMAX_DELAY);

  		itoa (resource[indx], sresource, 10);
  		strcpy (str, "Leaving HPT Task\n Data ACCESSED is:: ");
  		strcat (str, sresource);
  		strcat (str, "\n Not releasing the Semaphore\n\n\n");
  		HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen (str), HAL_MAX_DELAY);

  		indx++;
  		if (indx>2) indx=0;

  		vTaskDelay(3000);
  //		vTaskDelete(NULL);
  	}
  }

  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
  {
  	HAL_UART_Receive_IT(huart, &rx_data, 1);
  	if (rx_data == 'r')
  	{
  		// release the semaphore here
  		 /* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE as
  		 it will get set to pdTRUE inside the interrupt safe API function if a
  		 context switch is required. */
  		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  		xSemaphoreGiveFromISR(CountingSem, &xHigherPriorityTaskWoken);  // ISR SAFE VERSION
  		xSemaphoreGiveFromISR(CountingSem, &xHigherPriorityTaskWoken);  // ISR SAFE VERSION
  		xSemaphoreGiveFromISR(CountingSem, &xHigherPriorityTaskWoken);  // ISR SAFE VERSION

  		/* Pass the xHigherPriorityTaskWoken value into portEND_SWITCHING_ISR(). If
  		 xHigherPriorityTaskWoken was set to pdTRUE inside xSemaphoreGiveFromISR()
  		 then calling portEND_SWITCHING_ISR() will request a context switch. If
  		 xHigherPriorityTaskWoken is still pdFALSE then calling
  		 portEND_SWITCHING_ISR() will have no effect */

  		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  	}
  }
