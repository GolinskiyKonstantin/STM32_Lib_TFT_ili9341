/*
  ******************************************************************************
  * @file 			( фаил ):   ili9341.h
  * @brief 		( описание ):  	
  ******************************************************************************
  * @attention 	( внимание ):
  ******************************************************************************
  
 */
 
#ifndef _ILI9341_H
#define _ILI9341_H


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
#include "math.h"




//#######  SETUP  ##############################################################################################
		
		//==== выбераем через что будем отправлять через HAL или CMSIS(быстрее) ==================
		//-- нужное оставляем другое коментируем ( важно должно быть только один выбран )---------
		
			// указываем порт SPI для CMSIS ( быстро )-------
			// так как у разных МК разные регистры то в функциях корректируем под свой МК
			// на данный момент есть реализация на серию F1 F4 H7 для выбора серии в функциях
			//	void ILI9341_SendCmd(uint8_t Cmd);
			//	void ILI9341_SendData(uint8_t Data );
			//  void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
			// комментируем и раскомментируем то что нам нужно, также там же редактируем под свой МК если не работает
			#define 	ILI9341_SPI_CMSIS 		SPI1
			//-----------------------------------------------
			
			// указываем порт SPI для HAL ( медлено )--------
			//#define 	ILI9341_SPI_HAL 		hspi1
			//-----------------------------------------------

		//============================================================================
		
		//=== указываем порты ( если в кубе назвали их DC RESET CS то тогда нечего указывать не нужно )
		#if defined (DC_GPIO_Port)	// RES
		#else
			#define DC_GPIO_Port	GPIOA
			#define DC_Pin			GPIO_PIN_10
		#endif
		
		#if defined (RESET_GPIO_Port)
		#else
			#define RESET_GPIO_Port   GPIOA
			#define RESET_Pin			GPIO_PIN_12
		#endif

		//--  Cесли используем порт CS для выбора устройства тогда раскомментировать ------------
		// если у нас одно устройство лучше пин CS притянуть к земле( или на порту подать GND )
		
		#define CS_PORT
		
		//----------------------------------------------------------------------------------------
		#ifdef CS_PORT
			#if defined (CS_GPIO_Port)
			#else
				#define CS_GPIO_Port    GPIOA
				#define CS_Pin			GPIO_PIN_14
			#endif
		#endif
		
		//=============================================================================
		
	
//##############################################################################################################



#ifdef ILI9341_SPI_HAL
	extern SPI_HandleTypeDef ILI9341_SPI_HAL;
#endif		
	
extern uint16_t ILI9341_Width, ILI9341_Height;
	
// default orientation
#define ILI9341_WIDTH  		240
#define ILI9341_HEIGHT 		320

#define RGB565(r, g, b)    (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

#define PI 	3.14159265

//--- готовые цвета ------------------------------
	#define	ILI9341_BLACK   0x0000
	#define	ILI9341_BLUE    0x001F
	#define	ILI9341_RED     0xF800
	#define	ILI9341_GREEN   0x07E0
	#define ILI9341_CYAN    0x07FF
	#define ILI9341_MAGENTA 0xF81F
	#define ILI9341_YELLOW  0xFFE0
	#define ILI9341_WHITE   0xFFFF
//------------------------------------------------


#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_ML  0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH  0x04
//------------------------------------------------







//   TFT 2.2" 240 x 320

//==============================================================================


void ILI9341_Init(void);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9341_DrawChar(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, FontDef_t* Font, uint8_t multiplier, unsigned char ch);
void ILI9341_print(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, FontDef_t* Font, uint8_t multiplier, char *str);
void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
void ILI9341_InvertColors(uint8_t invert);
void ILI9341_Clear(void);
void ILI9341_SleepModeEnter( void );
void ILI9341_SleepModeExit( void );
void ILI9341_SetBL(uint8_t Value);
void ILI9341_DisplayPower(uint8_t On);
void ILI9341_DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void ILI9341_DrawRectangleFilled(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t fillcolor);
void ILI9341_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) ;
void ILI9341_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color);
void ILI9341_DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color);
void ILI9341_DrawCircleFilled(int16_t x0, int16_t y0, int16_t radius, uint16_t fillcolor);
void ILI9341_DrawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color);
void ILI9341_rotation( uint8_t rotation );
void ILI9341_DrawBitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, uint16_t color);
void ILI9341_DrawCircleHelper(int16_t x0, int16_t y0, int16_t radius, int8_t quadrantMask, uint16_t color);
void ILI9341_DrawFillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color);
void ILI9341_DrawFillRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, int16_t cornerRadius, uint16_t color);
void ILI9341_DrawRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, int16_t cornerRadius, uint16_t color);
void ILI9341_DrawArc(int16_t x0, int16_t y0, int16_t radius, int16_t startAngle, int16_t endAngle, uint16_t color, uint8_t thick);
void ILI9341_DrawLineThick(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, uint8_t thick);


/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif	/*	_ILI9341_H */

/************************ (C) COPYRIGHT GKP *****END OF FILE****/
