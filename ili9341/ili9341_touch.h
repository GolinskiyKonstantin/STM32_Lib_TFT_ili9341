/*
  ******************************************************************************
  * @file 			( фаил ):   ili9341_touch.h
  * @brief 		( описание ):  	
  ******************************************************************************
  * @attention 	( внимание ):
  ******************************************************************************
  
 */
 
#ifndef _ILI9341_TOUCH_H
#define _ILI9341_TOUCH_H


/* C++ detection */
#ifdef __cplusplus
extern C {
#endif

// Обязательно нужен #include "main.h" 
// чтоб отдельно не подключать файлы связанные с МК и стандартными библиотеками

#include "main.h"
#include "fonts.h"

#include "stdlib.h"
#include "string.h"


/*** Redefine if necessary ***/

// Warning! Use SPI bus with < 1.3 Mbit speed, better ~650 Kbit to be save.
#define ILI9341_TOUCH_SPI_PORT hspi1
extern SPI_HandleTypeDef ILI9341_TOUCH_SPI_PORT;

#define ILI9341_TOUCH_IRQ_Pin       GPIO_PIN_4 // Arduino D5
#define ILI9341_TOUCH_IRQ_GPIO_Port GPIOB
#define ILI9341_TOUCH_CS_Pin        GPIO_PIN_10 // Arduino D2
#define ILI9341_TOUCH_CS_GPIO_Port  GPIOA

// change depending on screen orientation
#define ILI9341_TOUCH_SCALE_X 240
#define ILI9341_TOUCH_SCALE_Y 320

// to calibrate uncomment UART_Printf line in ili9341_touch.c
#define ILI9341_TOUCH_MIN_RAW_X 1500
#define ILI9341_TOUCH_MAX_RAW_X 31000
#define ILI9341_TOUCH_MIN_RAW_Y 3276
#define ILI9341_TOUCH_MAX_RAW_Y 30110

// call before initializing any SPI devices
void ILI9341_TouchUnselect(void);

uint8_t ILI9341_TouchPressed(void);
uint8_t ILI9341_TouchGetCoordinates(uint16_t* x, uint16_t* y);


/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif	/*	_ILI9341_TOUCH_H */

/************************ (C) COPYRIGHT GKP *****END OF FILE****/
