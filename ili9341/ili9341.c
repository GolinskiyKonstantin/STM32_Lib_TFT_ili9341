/*

  ******************************************************************************
  * @file 			( фаил ):   ili9341.c
  * @brief 		( описание ):  	
  ******************************************************************************
  * @attention 	( внимание ):
  ******************************************************************************
  
*/

#include "ili9341.h"


static void ILI9341_Unselect(void);
static void ILI9341_Select(void);
static void ILI9341_SendCmd(uint8_t Cmd);
static void ILI9341_SendData(uint8_t Data );
static void SwapInt16Values(int16_t *pValue1, int16_t *pValue2);
static void ILI9341_DrawLine_Slow(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);


uint16_t ILI9341_Width, ILI9341_Height;

//   TFT 2.2" 240 x 320

//==============================================================================


//==============================================================================
// Процедура управления SPI
//==============================================================================
static void ILI9341_Select(void) {
	
    #ifdef CS_PORT
			//-- если захотим переделать под HAL ------------------	
			#ifdef ILI9341_SPI_HAL
				HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
			#endif
			//-----------------------------------------------------
			
			//-- если захотим переделать под CMSIS  ---------------
			#ifdef ILI9341_SPI_CMSIS
				CS_GPIO_Port->BSRR = ( CS_Pin << 16 );
			#endif
			//-----------------------------------------------------
	#endif
	
}
//==============================================================================


//==============================================================================
// Процедура управления SPI
//==============================================================================
static void ILI9341_Unselect(void) {
	
    #ifdef CS_PORT
			//-- если захотим переделать под HAL ------------------	
			#ifdef ILI9341_SPI_HAL
				HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
			#endif
			//-----------------------------------------------------
			
			//-- если захотим переделать под CMSIS  ---------------
			#ifdef ILI9341_SPI_CMSIS
					 CS_GPIO_Port->BSRR = CS_Pin;
			#endif
			//-----------------------------------------------------
	#endif
	
}
//==============================================================================


//==============================================================================
// Процедура аппаратного сброса дисплея (ножкой RESET)
//==============================================================================
static void ILI9341_Reset(void) {
	
    HAL_GPIO_WritePin(RESET_GPIO_Port, RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(RESET_GPIO_Port, RESET_Pin, GPIO_PIN_SET);
}
//==============================================================================


//==============================================================================
// Процедура отправки команды в дисплей
//==============================================================================
static void ILI9341_SendCmd(uint8_t cmd) {
	
	//-- если захотим переделать под HAL ------------------
	#ifdef ILI9341_SPI_HAL
		
		// pin DC LOW
		 HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_RESET);
		 
		HAL_SPI_Transmit(&ILI9341_SPI_HAL, &cmd, sizeof(cmd), HAL_MAX_DELAY);
		while(HAL_SPI_GetState(&ILI9341_SPI_HAL) != HAL_SPI_STATE_READY){};
		
		// pin DC HIGH
		 HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET);
		
	#endif
	//-----------------------------------------------------
			
	//-- если захотим переделать под CMSIS  ---------------------------------------------
	#ifdef ILI9341_SPI_CMSIS
		
		// pin DC LOW
		DC_GPIO_Port->BSRR = ( DC_Pin << 16 );
		
		//======  FOR F-SERIES ===========================================================
			
			// Disable SPI	
			//CLEAR_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);	// ILI9341_SPI_CMSIS->CR1 &= ~SPI_CR1_SPE;
			// Enable SPI
			if((ILI9341_SPI_CMSIS->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
				// If disabled, I enable it
				SET_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);	// ILI9341_SPI_CMSIS->CR1 |= SPI_CR1_SPE;
			}
			
			// Ждем, пока не освободится буфер передатчика
			// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
			while( (ILI9341_SPI_CMSIS->SR & SPI_SR_TXE) == RESET ){};
		
			// передаем 1 байт информации--------------
			*((__IO uint8_t *)&ILI9341_SPI_CMSIS->DR) = cmd;
			
			// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
			while( (ILI9341_SPI_CMSIS->SR & (SPI_SR_TXE | SPI_SR_BSY)) != SPI_SR_TXE ){};
			
			//Ждем, пока SPI освободится от предыдущей передачи
			//while((ILI9341_SPI_CMSIS->SR&SPI_SR_BSY)){};
			
			// Disable SPI	
			//CLEAR_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);
			
		//================================================================================
		
/*		//======  FOR H-SERIES ===========================================================

			// Disable SPI	
			//CLEAR_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);	// ILI9341_SPI_CMSIS->CR1 &= ~SPI_CR1_SPE;
			// Enable SPI
			if((ILI9341_SPI_CMSIS->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
				// If disabled, I enable it
				SET_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);	// ILI9341_SPI_CMSIS->CR1 |= SPI_CR1_SPE;
			}
			
			SET_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_CSTART);	// ILI9341_SPI_CMSIS->CR1 |= SPI_CR1_CSTART;
			
			// ждем пока SPI будет свободна------------
			//while (!(ILI9341_SPI_CMSIS->SR & SPI_SR_TXP)){};		
		
			// передаем 1 байт информации--------------
			*((__IO uint8_t *)&ILI9341_SPI_CMSIS->TXDR )  = cmd;
				
			// Ждать завершения передачи---------------
			while (!( ILI9341_SPI_CMSIS -> SR & SPI_SR_TXC )){};
			
			// Disable SPI	
			//CLEAR_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);
			
*/		//================================================================================
		
		// pin DC HIGH
		DC_GPIO_Port->BSRR = DC_Pin;
		
	#endif
	//-----------------------------------------------------------------------------------
	
}
//==============================================================================


//==============================================================================
// Процедура отправки данных (параметров) в дисплей 1 BYTE
//==============================================================================
static void ILI9341_SendData(uint8_t* buff, size_t buff_size) {
	
	//-- если захотим переделать под HAL ------------------
	#ifdef ILI9341_SPI_HAL

		if( buff_size <= 0xFFFF ){
			HAL_SPI_Transmit(&ILI9341_SPI_HAL, buff, buff_size, HAL_MAX_DELAY);
		}
		else{
			while( buff_size > 0xFFFF ){
				HAL_SPI_Transmit(&ILI9341_SPI_HAL, buff, 0xFFFF, HAL_MAX_DELAY);
				buff_size-=0xFFFF;
				buff+=0xFFFF;
			}
			HAL_SPI_Transmit(&ILI9341_SPI_HAL, buff, buff_size, HAL_MAX_DELAY);
		}
		
		while(HAL_SPI_GetState(&ILI9341_SPI_HAL) != HAL_SPI_STATE_READY){};
		
	#endif
	//-----------------------------------------------------
	
	//-- если захотим переделать под CMSIS  ---------------------------------------------
	#ifdef ILI9341_SPI_CMSIS	

		//======  FOR F-SERIES ===========================================================
			
			// Disable SPI	
			//CLEAR_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);	// ILI9341_SPI_CMSIS->CR1 &= ~SPI_CR1_SPE;
			// Enable SPI
			if((ILI9341_SPI_CMSIS->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
				// If disabled, I enable it
				SET_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);	// ILI9341_SPI_CMSIS->CR1 |= SPI_CR1_SPE;
			}
	
			while( buff_size ){
				
				// Ждем, пока не освободится буфер передатчика
				// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
				while( (ILI9341_SPI_CMSIS->SR & SPI_SR_TXE) == RESET ){};
				
				// передаем 1 байт информации--------------
				*((__IO uint8_t *)&ILI9341_SPI_CMSIS->DR) = *buff++;

				buff_size--;
			}
			
			// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
			while( (ILI9341_SPI_CMSIS->SR & (SPI_SR_TXE | SPI_SR_BSY)) != SPI_SR_TXE ){};
			
			// Ждем, пока не освободится буфер передатчика
			// while((ILI9341_SPI_CMSIS->SR&SPI_SR_BSY)){};
			
			// Disable SPI	
			//CLEAR_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);
			
		//================================================================================
		
/*		//======  FOR H-SERIES ===========================================================

			// Disable SPI	
			//CLEAR_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);	// ILI9341_SPI_CMSIS->CR1 &= ~SPI_CR1_SPE;
			// Enable SPI
			if((ILI9341_SPI_CMSIS->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
				// If disabled, I enable it
				SET_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);	// ILI9341_SPI_CMSIS->CR1 |= SPI_CR1_SPE;
			}
			
			SET_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_CSTART);	// ILI9341_SPI_CMSIS->CR1 |= SPI_CR1_CSTART;
			
			// ждем пока SPI будет свободна------------
			//while (!(ILI9341_SPI_CMSIS->SR & SPI_SR_TXP)){};		
			
			while( buff_size ){
		
				// передаем 1 байт информации--------------
				*((__IO uint8_t *)&ILI9341_SPI_CMSIS->TXDR )  = *buff++;
				
				// Ждать завершения передачи---------------
				while (!( ILI9341_SPI_CMSIS -> SR & SPI_SR_TXC )){};

				buff_size--;

			}
			
			// Disable SPI	
			//CLEAR_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);
			
*/		//================================================================================
		
	#endif
	//-----------------------------------------------------------------------------------
}
//==============================================================================


//==============================================================================
// Процедура установка границ экрана для заполнения
//==============================================================================
static void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	
    // column address set
    ILI9341_SendCmd(0x2A); // CASET
    {
        uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1 & 0xFF };
        ILI9341_SendData(data, sizeof(data));
    }

    // row address set
    ILI9341_SendCmd(0x2B); // RASET
    {
        uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1 & 0xFF };
        ILI9341_SendData(data, sizeof(data));
    }

    // write to RAM
    ILI9341_SendCmd(0x2C); // RAMWR
}
//==============================================================================
	  
	  
	  
//==============================================================================
// Процедура инициализации дисплея
//==============================================================================
void ILI9341_Init(void) {
	
	
	// Задержка после подачи питания
	// если при старте не всегда запускаеться дисплей увеличиваем время задержки
	HAL_Delay(200);	

	ILI9341_Width = ILI9341_WIDTH;		// 240
	ILI9341_Height = ILI9341_HEIGHT;	// 320
	
	
    ILI9341_Select();
    ILI9341_Reset();

    // command list is based on https://github.com/martnak/STM32-ILI9341

    // SOFTWARE RESET
    ILI9341_SendCmd(0x01);
    HAL_Delay(1000);
        
    // POWER CONTROL A
    ILI9341_SendCmd(0xCB);
    {
        uint8_t data[] = { 0x39, 0x2C, 0x00, 0x34, 0x02 };
        ILI9341_SendData(data, sizeof(data));
    }

    // POWER CONTROL B
    ILI9341_SendCmd(0xCF);
    {
        uint8_t data[] = { 0x00, 0xC1, 0x30 };
        ILI9341_SendData(data, sizeof(data));
    }

    // DRIVER TIMING CONTROL A
    ILI9341_SendCmd(0xE8);
    {
        uint8_t data[] = { 0x85, 0x00, 0x78 };
        ILI9341_SendData(data, sizeof(data));
    }

    // DRIVER TIMING CONTROL B
    ILI9341_SendCmd(0xEA);
    {
        uint8_t data[] = { 0x00, 0x00 };
        ILI9341_SendData(data, sizeof(data));
    }

    // POWER ON SEQUENCE CONTROL
    ILI9341_SendCmd(0xED);
    {
        uint8_t data[] = { 0x64, 0x03, 0x12, 0x81 };
        ILI9341_SendData(data, sizeof(data));
    }

    // PUMP RATIO CONTROL
    ILI9341_SendCmd(0xF7);
    {
        uint8_t data[] = { 0x20 };
        ILI9341_SendData(data, sizeof(data));
    }

    // POWER CONTROL,VRH[5:0]
    ILI9341_SendCmd(0xC0);
    {
        uint8_t data[] = { 0x23 };
        ILI9341_SendData(data, sizeof(data));
    }

    // POWER CONTROL,SAP[2:0];BT[3:0]
    ILI9341_SendCmd(0xC1);
    {
        uint8_t data[] = { 0x10 };
        ILI9341_SendData(data, sizeof(data));
    }

    // VCM CONTROL
    ILI9341_SendCmd(0xC5);
    {
        uint8_t data[] = { 0x3E, 0x28 };
        ILI9341_SendData(data, sizeof(data));
    }

    // VCM CONTROL 2
    ILI9341_SendCmd(0xC7);
    {
        uint8_t data[] = { 0x86 };
        ILI9341_SendData(data, sizeof(data));
    }

    // MEMORY ACCESS CONTROL
    ILI9341_SendCmd(0x36);
    {
        uint8_t data[] = { 0x48 };
        ILI9341_SendData(data, sizeof(data));
    }

    // PIXEL FORMAT
    ILI9341_SendCmd(0x3A);
    {
        uint8_t data[] = { 0x55 };
        ILI9341_SendData(data, sizeof(data));
    }

    // FRAME RATIO CONTROL, STANDARD RGB COLOR
    ILI9341_SendCmd(0xB1);
    {
        uint8_t data[] = { 0x00, 0x18 };
        ILI9341_SendData(data, sizeof(data));
    }

    // DISPLAY FUNCTION CONTROL
    ILI9341_SendCmd(0xB6);
    {
        uint8_t data[] = { 0x08, 0x82, 0x27 };
        ILI9341_SendData(data, sizeof(data));
    }

    // 3GAMMA FUNCTION DISABLE
    ILI9341_SendCmd(0xF2);
    {
        uint8_t data[] = { 0x00 };
        ILI9341_SendData(data, sizeof(data));
    }

    // GAMMA CURVE SELECTED
    ILI9341_SendCmd(0x26);
    {
        uint8_t data[] = { 0x01 };
        ILI9341_SendData(data, sizeof(data));
    }

    // POSITIVE GAMMA CORRECTION
    ILI9341_SendCmd(0xE0);
    {
        uint8_t data[] = { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                           0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 };
        ILI9341_SendData(data, sizeof(data));
    }

    // NEGATIVE GAMMA CORRECTION
    ILI9341_SendCmd(0xE1);
    {
        uint8_t data[] = { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                           0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F };
        ILI9341_SendData(data, sizeof(data));
    }

    // EXIT SLEEP
    ILI9341_SendCmd(0x11);
    HAL_Delay(120);

    // TURN ON DISPLAY
    ILI9341_SendCmd(0x29);

    // MADCTL
    ILI9341_SendCmd(0x36);
    {
        uint8_t data[] = { (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR) };
        ILI9341_SendData(data, sizeof(data));
    }

    ILI9341_Unselect();
}
//==============================================================================


//==============================================================================
// Процедура окрашивает 1 пиксель дисплея
//==============================================================================
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
	
    if((x >= ILI9341_Width) || (y >= ILI9341_Height)){
        return;
	}

    ILI9341_Select();

    ILI9341_SetAddressWindow(x, y, x+1, y+1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    ILI9341_SendData(data, sizeof(data));

    ILI9341_Unselect();
}
//==============================================================================


//==============================================================================
// Процедура рисования символа ( 1 буква или знак )
//==============================================================================
void ILI9341_DrawChar(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, FontDef_t* Font, uint8_t multiplier, unsigned char ch) {
	
	uint32_t i, b, j;
	
	uint32_t X = x, Y = y;
	
	uint8_t xx, yy;
	
	if( multiplier < 1 ){
		multiplier = 1;
	}

	
	ILI9341_SetAddressWindow(x, y, x+Font->FontWidth-1, y+Font->FontHeight-1);
	
	
	/* Check available space in LCD */
	if (ILI9341_Width >= ( x + Font->FontWidth) || ILI9341_Height >= ( y + Font->FontHeight)){
	
		/* Go through font */
		for (i = 0; i < Font->FontHeight; i++) {		
			
			if( ch < 127 ){			
				b = Font->data[(ch - 32) * Font->FontHeight + i];
			}
			
			else if( (uint8_t) ch > 191 ){
				// +96 это так как латинские символы и знаки в шрифтах занимают 96 позиций
				// и если в шрифте который содержит сперва латиницу и спец символы и потом 
				// только кирилицу то нужно добавлять 95 если шрифт 
				// содержит только кирилицу то +96 не нужно
				b = Font->data[((ch - 192) + 96) * Font->FontHeight + i];
			}
			
			else if( (uint8_t) ch == 168 ){	// 168 символ по ASCII - Ё
				// 160 эллемент ( символ Ё ) 
				b = Font->data[( 160 ) * Font->FontHeight + i];
			}
			
			else if( (uint8_t) ch == 184 ){	// 184 символ по ASCII - ё
				// 161 эллемент  ( символ ё ) 
				b = Font->data[( 161 ) * Font->FontHeight + i];
			}
			//-------------------------------------------------------------------
			
			//----  Украинская раскладка ----------------------------------------------------
			else if( (uint8_t) ch == 170 ){	// 168 символ по ASCII - Є
				// 162 эллемент ( символ Є )
				b = Font->data[( 162 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 175 ){	// 184 символ по ASCII - Ї
				// 163 эллемент  ( символ Ї )
				b = Font->data[( 163 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 178 ){	// 168 символ по ASCII - І
				// 164 эллемент ( символ І )
				b = Font->data[( 164 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 179 ){	// 184 символ по ASCII - і
				// 165 эллемент  ( символ і )
				b = Font->data[( 165 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 186 ){	// 184 символ по ASCII - є
				// 166 эллемент  ( символ є )
				b = Font->data[( 166 ) * Font->FontHeight + i];
			}
			else if( (uint8_t) ch == 191 ){	// 168 символ по ASCII - ї
				// 167 эллемент ( символ ї )
				b = Font->data[( 167 ) * Font->FontHeight + i];
			}
			//-----------------------------------------------------------------------------
			
			for (j = 0; j < Font->FontWidth; j++) {
				
				if ((b << j) & 0x8000) {
					
					for (yy = 0; yy < multiplier; yy++){
						for (xx = 0; xx < multiplier; xx++){
								ILI9341_DrawPixel(X+xx, Y+yy, TextColor);
						}
					}
					
				} 
				else if( TransparentBg ){
					
					for (yy = 0; yy < multiplier; yy++){
						for (xx = 0; xx < multiplier; xx++){
								ILI9341_DrawPixel(X+xx, Y+yy, BgColor);
						}
					}
					
				}
				X = X + multiplier;
			}
			X = x;
			Y = Y + multiplier;
		}
	
	}
}
//==============================================================================


//==============================================================================
// Процедура рисования строки
//==============================================================================
void ILI9341_print(uint16_t x, uint16_t y, uint16_t TextColor, uint16_t BgColor, uint8_t TransparentBg, FontDef_t* Font, uint8_t multiplier, char *str){	
	
	if( multiplier < 1 ){
		multiplier = 1;
	}
	
	unsigned char buff_char;
	
	uint16_t len = strlen(str);
	
	while (len--) {
		
		//---------------------------------------------------------------------
		// проверка на кириллицу UTF-8, если латиница то пропускаем if
		// Расширенные символы ASCII Win-1251 кириллица (код символа 128-255)
		// проверяем первый байт из двух ( так как UTF-8 ето два байта )
		// если он больше либо равен 0xC0 ( первый байт в кириллеце будет равен 0xD0 либо 0xD1 именно в алфавите )
		if ( (uint8_t)*str >= 0xC0 ){	// код 0xC0 соответствует символу кириллица 'A' по ASCII Win-1251
			
			// проверяем какой именно байт первый 0xD0 либо 0xD1---------------------------------------------
			switch ((uint8_t)*str) {
				case 0xD0: {
					// увеличиваем массив так как нам нужен второй байт
					str++;
					// проверяем второй байт там сам символ
					if ((uint8_t)*str >= 0x90 && (uint8_t)*str <= 0xBF){ buff_char = (*str) + 0x30; }	// байт символов А...Я а...п  делаем здвиг на +48
					else if ((uint8_t)*str == 0x81) { buff_char = 0xA8; break; }		// байт символа Ё ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x84) { buff_char = 0xAA; break; }		// байт символа Є ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x86) { buff_char = 0xB2; break; }		// байт символа І ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x87) { buff_char = 0xAF; break; }		// байт символа Ї ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					break;
				}
				case 0xD1: {
					// увеличиваем массив так как нам нужен второй байт
					str++;
					// проверяем второй байт там сам символ
					if ((uint8_t)*str >= 0x80 && (uint8_t)*str <= 0x8F){ buff_char = (*str) + 0x70; }	// байт символов п...я	елаем здвиг на +112
					else if ((uint8_t)*str == 0x91) { buff_char = 0xB8; break; }		// байт символа ё ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x94) { buff_char = 0xBA; break; }		// байт символа є ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x96) { buff_char = 0xB3; break; }		// байт символа і ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					else if ((uint8_t)*str == 0x97) { buff_char = 0xBF; break; }		// байт символа ї ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					break;
				}
			}
			//------------------------------------------------------------------------------------------------
			// уменьшаем еще переменную так как израсходывали 2 байта для кириллицы
			len--;
			
			ILI9341_DrawChar(x, y, TextColor, BgColor, TransparentBg, Font, multiplier, buff_char);
		}
		//---------------------------------------------------------------------
		else{
			ILI9341_DrawChar(x, y, TextColor, BgColor, TransparentBg, Font, multiplier, *str);
		}
			
		x = x + (Font->FontWidth * multiplier);
		/* Increase string pointer */
		str++;
	}
}
//==============================================================================


//==============================================================================
// Процедура заполнения прямоугольника цветом color
//==============================================================================
void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
	
    // clipping
    if((x >= ILI9341_Width) || (y >= ILI9341_Height)){
		return;
	}
	
    if((x + w - 1) >= ILI9341_Width){
		w = ILI9341_Width - x;
	}
	
    if((y + h - 1) >= ILI9341_Height){
		h = ILI9341_Height - y;
	}

    ILI9341_Select();
    ILI9341_SetAddressWindow(x, y, x+w-1, y+h-1);

    uint8_t data[2];
	data[0] = color >> 8;
	data[1] = color & 0xFF ;
		
	//-- если захотим переделать под HAL ------------------
	#ifdef ILI9341_SPI_HAL
			
		// pin DC HIGH
		HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET);
				
	#endif
	//-----------------------------------------------------

	//-- если захотим переделать под CMSIS  ---------------------------------------------
	#ifdef ILI9341_SPI_CMSIS
				
		// pin DC HIGH
		DC_GPIO_Port->BSRR = DC_Pin;
		
	#endif
	//-----------------------------------------------------
	
    for(y = h; y > 0; y--) {
        for(x = w; x > 0; x--) {
			
			//-- если захотим переделать под HAL ------------------
			#ifdef ILI9341_SPI_HAL
				 
				HAL_SPI_Transmit(&ILI9341_SPI_HAL, data, sizeof(data), HAL_MAX_DELAY);
				while(HAL_SPI_GetState(&ILI9341_SPI_HAL) != HAL_SPI_STATE_READY){};
				
			#endif
			//-----------------------------------------------------
					
			//-- если захотим переделать под CMSIS  ---------------------------------------------
			#ifdef ILI9341_SPI_CMSIS
				
				//======  FOR F-SERIES ===========================================================
					
					// Disable SPI	
					//CLEAR_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);	// ILI9341_SPI_CMSIS->CR1 &= ~SPI_CR1_SPE;
					// Enable SPI
					if((ILI9341_SPI_CMSIS->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
						// If disabled, I enable it
						SET_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);	// ILI9341_SPI_CMSIS->CR1 |= SPI_CR1_SPE;
					}
					
					// Ждем, пока не освободится буфер передатчика
					// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
					while( (ILI9341_SPI_CMSIS->SR & SPI_SR_TXE) == RESET ){};
				
					// передаем 1 байт информации--------------
					*((__IO uint8_t *)&ILI9341_SPI_CMSIS->DR) = data[0];
					
					// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
					while( (ILI9341_SPI_CMSIS->SR & (SPI_SR_TXE | SPI_SR_BSY)) != SPI_SR_TXE ){};
					
					// Ждем, пока не освободится буфер передатчика
					// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
					while( (ILI9341_SPI_CMSIS->SR & SPI_SR_TXE) == RESET ){};
						
					// передаем 1 байт информации--------------
					*((__IO uint8_t *)&ILI9341_SPI_CMSIS->DR) = data[1];
					
					// TXE(Transmit buffer empty) – устанавливается когда буфер передачи(регистр SPI_DR) пуст, очищается при загрузке данных
					while( (ILI9341_SPI_CMSIS->SR & (SPI_SR_TXE | SPI_SR_BSY)) != SPI_SR_TXE ){};
						
					//Ждем, пока SPI освободится от предыдущей передачи
					//while((ILI9341_SPI_CMSIS->SR&SPI_SR_BSY)){};
					
					// Disable SPI	
					//CLEAR_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);
					
				//================================================================================
				
		/*		//======  FOR H-SERIES ===========================================================

					// Disable SPI	
					//CLEAR_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);	// ILI9341_SPI_CMSIS->CR1 &= ~SPI_CR1_SPE;
					// Enable SPI
					if((ILI9341_SPI_CMSIS->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
						// If disabled, I enable it
						SET_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);	// ILI9341_SPI_CMSIS->CR1 |= SPI_CR1_SPE;
					}
					
					SET_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_CSTART);	// ILI9341_SPI_CMSIS->CR1 |= SPI_CR1_CSTART;
					
					// ждем пока SPI будет свободна------------
					//while (!(ILI9341_SPI_CMSIS->SR & SPI_SR_TXP)){};		
				
					// передаем 1 байт информации--------------
					*((__IO uint8_t *)&ILI9341_SPI_CMSIS->TXDR )  = data[0];
						
					// Ждать завершения передачи---------------
					while (!( ILI9341_SPI_CMSIS -> SR & SPI_SR_TXC )){};
					
					// ждем пока SPI будет свободна------------
					//while (!(ILI9341_SPI_CMSIS->SR & SPI_SR_TXP)){};
					
					// передаем 1 байт информации--------------
					*((__IO uint8_t *)&ILI9341_SPI_CMSIS->TXDR )  = data[1];
						
					// Ждать завершения передачи---------------
					while (!( ILI9341_SPI_CMSIS -> SR & SPI_SR_TXC )){};
					
					// Disable SPI	
					//CLEAR_BIT(ILI9341_SPI_CMSIS->CR1, SPI_CR1_SPE);
					
		*/		//================================================================================
								
			#endif
			//-----------------------------------------------------------------------------------        
        }
    }
	
	//-- если захотим переделать под HAL ------------------
	#ifdef ILI9341_SPI_HAL
				
		// pin DC LOW
		HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_RESET);
				
	#endif
	//-----------------------------------------------------
	
	//-- если захотим переделать под CMSIS  ---------------------------------------------
	#ifdef ILI9341_SPI_CMSIS
				
		// pin DC LOW
		DC_GPIO_Port->BSRR = ( DC_Pin << 16 );
		
	#endif
	//-----------------------------------------------------
	
    ILI9341_Unselect();
}
//==============================================================================


//==============================================================================
// Процедура закрашивает экран цветом color
//==============================================================================
void ILI9341_FillScreen(uint16_t color) {
	
    ILI9341_FillRectangle(0, 0, ILI9341_Width, ILI9341_Height, color);
}
//==============================================================================


//==============================================================================
// Процедура вывода цветного изображения на дисплей
//==============================================================================
void ILI9341_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data) {
	
    if((x >= ILI9341_Width) || (y >= ILI9341_Height)){
		return;
	}
	
    if((x + w - 1) >= ILI9341_Width){
		return;
	}
	
    if((y + h - 1) >= ILI9341_Height){
		return;
	}

    ILI9341_Select();
	
    ILI9341_SetAddressWindow(x, y, x+w-1, y+h-1);
    ILI9341_SendData((uint8_t*)data, sizeof(uint16_t)*w*h);
	
    ILI9341_Unselect();
}
//==============================================================================


//==============================================================================
// Процедура включения/отключения режима частичного заполнения экрана
//==============================================================================
void ILI9341_InvertColors(uint8_t invert) {
	
    ILI9341_Select();
    ILI9341_SendCmd(invert ? 0x21 /* INVON */ : 0x20 /* INVOFF */);
    ILI9341_Unselect();
	
}
//==============================================================================


//==============================================================================
// Процедура очистки экрана - закрашивает экран цветом черный
//==============================================================================
void ILI9341_Clear(void){
	
  ILI9341_FillRectangle(0, 0,  ILI9341_Width, ILI9341_Height, 0);
}
//==============================================================================


//==============================================================================
// Процедура включения режима сна
//==============================================================================
void ILI9341_SleepModeEnter( void ){
	
	ILI9341_Select(); 
	
	ILI9341_SendCmd( 0x10 ); // SLPIN
	
	ILI9341_Unselect();
	
	HAL_Delay(250);
}
//==============================================================================


//==============================================================================
// Процедура отключения режима сна
//==============================================================================
void ILI9341_SleepModeExit( void ){
	
	ILI9341_Select(); 
	
	ILI9341_SendCmd( 0x11 ); // SLPOUT
	
	ILI9341_Unselect();
	
	HAL_Delay(250);
}
//==============================================================================


//==============================================================================
// Процедура управления подсветкой (ШИМ)
//==============================================================================
void ILI9341_SetBL(uint8_t Value){
	
//  if (Value > 100)
//    Value = 100;

//	tmr2_PWM_set(ILI9341_PWM_TMR2_Chan, Value);

}
//==============================================================================


//==============================================================================
// Процедура включения/отключения питания дисплея
//==============================================================================
void ILI9341_DisplayPower(uint8_t On){
	
  ILI9341_Select(); 
	
  if (On){
    ILI9341_SendCmd( 0x29 ); // DISPON
  }
  else{
    ILI9341_SendCmd( 0x28 ); // DISPOFF
  }
  
  ILI9341_Unselect();
}
//==============================================================================


//==============================================================================
// Процедура рисования прямоугольника ( пустотелый )
//==============================================================================
void ILI9341_DrawRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
	
  ILI9341_DrawLine(x1, y1, x1, y2, color);
  ILI9341_DrawLine(x2, y1, x2, y2, color);
  ILI9341_DrawLine(x1, y1, x2, y1, color);
  ILI9341_DrawLine(x1, y2, x2, y2, color);
	
}
//==============================================================================


//==============================================================================
// Процедура вспомогательная для --- Процедура рисования прямоугольника ( заполненый )
//==============================================================================
static void SwapInt16Values(int16_t *pValue1, int16_t *pValue2){
	
  int16_t TempValue = *pValue1;
  *pValue1 = *pValue2;
  *pValue2 = TempValue;
}
//==============================================================================


//==============================================================================
// Процедура рисования прямоугольника ( заполненый )
//==============================================================================
void ILI9341_DrawRectangleFilled(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t fillcolor) {
	
  if (x1 > x2){
    SwapInt16Values(&x1, &x2);
  }
  
  if (y1 > y2){
    SwapInt16Values(&y1, &y2);
  }
  
  ILI9341_FillRectangle(x1, y1, x2 - x1, y2 - y1, fillcolor);
}
//==============================================================================


//==============================================================================
// Процедура вспомогательная для --- Процедура рисования линии
//==============================================================================
static void ILI9341_DrawLine_Slow(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
	
  const int16_t deltaX = abs(x2 - x1);
  const int16_t deltaY = abs(y2 - y1);
  const int16_t signX = x1 < x2 ? 1 : -1;
  const int16_t signY = y1 < y2 ? 1 : -1;

  int16_t error = deltaX - deltaY;

  ILI9341_DrawPixel(x2, y2, color);

  while (x1 != x2 || y1 != y2) {
	  
    ILI9341_DrawPixel(x1, y1, color);
    const int16_t error2 = error * 2;
 
    if (error2 > -deltaY) {
		
      error -= deltaY;
      x1 += signX;
    }
    if (error2 < deltaX){
		
      error += deltaX;
      y1 += signY;
    }
  }
}
//==============================================================================


//==============================================================================
// Процедура рисования линии
//==============================================================================
void ILI9341_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {

  if (x1 == x2){

    if (y1 > y2){
      ILI9341_FillRectangle(x1, y2, 1, y1 - y2 + 1, color);
	}
    else{
      ILI9341_FillRectangle(x1, y1, 1, y2 - y1 + 1, color);
	}
	
    return;
  }
  
  if (y1 == y2){
    
    if (x1 > x2){
      ILI9341_FillRectangle(x2, y1, x1 - x2 + 1, 1, color);
	}
    else{
      ILI9341_FillRectangle(x1, y1, x2 - x1 + 1, 1, color);
	}
	
    return;
  }
  
  ILI9341_DrawLine_Slow(x1, y1, x2, y2, color);
}
//==============================================================================


//==============================================================================
// Процедура рисования треугольника ( пустотелый )
//==============================================================================
void ILI9341_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color){
	/* Draw lines */
	ILI9341_DrawLine(x1, y1, x2, y2, color);
	ILI9341_DrawLine(x2, y2, x3, y3, color);
	ILI9341_DrawLine(x3, y3, x1, y1, color);
}
//==============================================================================


//==============================================================================
// Процедура рисования треугольника ( заполненый )
//==============================================================================
void ILI9341_DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color){
	
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
	curpixel = 0;
	
	deltax = abs(x2 - x1);
	deltay = abs(y2 - y1);
	x = x1;
	y = y1;

	if (x2 >= x1) {
		xinc1 = 1;
		xinc2 = 1;
	} 
	else {
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) {
		yinc1 = 1;
		yinc2 = 1;
	} 
	else {
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay){
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	} 
	else {
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++) {
		ILI9341_DrawLine(x, y, x3, y3, color);

		num += numadd;
		if (num >= den) {
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}
//==============================================================================


//==============================================================================
// Процедура рисования круг ( заполненый )
//==============================================================================
void ILI9341_DrawCircleFilled(int16_t x0, int16_t y0, int16_t radius, uint16_t fillcolor) {
	
  int x = 0;
  int y = radius;
  int delta = 1 - 2 * radius;
  int error = 0;

  while (y >= 0){
	  
    ILI9341_DrawLine(x0 + x, y0 - y, x0 + x, y0 + y, fillcolor);
    ILI9341_DrawLine(x0 - x, y0 - y, x0 - x, y0 + y, fillcolor);
    error = 2 * (delta + y) - 1;

    if (delta < 0 && error <= 0) {
		
      ++x;
      delta += 2 * x + 1;
      continue;
    }
	
    error = 2 * (delta - x) - 1;
		
    if (delta > 0 && error > 0) {
		
      --y;
      delta += 1 - 2 * y;
      continue;
    }
	
    ++x;
    delta += 2 * (x - y);
    --y;
  }
}
//==============================================================================


//==============================================================================
// Процедура рисования круг ( пустотелый )
//==============================================================================
void ILI9341_DrawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color) {
	
  int x = 0;
  int y = radius;
  int delta = 1 - 2 * radius;
  int error = 0;

  while (y >= 0){
	  
    ILI9341_DrawPixel(x0 + x, y0 + y, color);
    ILI9341_DrawPixel(x0 + x, y0 - y, color);
    ILI9341_DrawPixel(x0 - x, y0 + y, color);
    ILI9341_DrawPixel(x0 - x, y0 - y, color);
    error = 2 * (delta + y) - 1;

    if (delta < 0 && error <= 0) {
		
      ++x;
      delta += 2 * x + 1;
      continue;
    }
	
    error = 2 * (delta - x) - 1;
		
    if (delta > 0 && error > 0) {
		
      --y;
      delta += 1 - 2 * y;
      continue;
    }
	
    ++x;
    delta += 2 * (x - y);
    --y;
  }
}
//==============================================================================


//==============================================================================
// Процедура ротации ( положение ) дисплея
//==============================================================================
// па умолчанию 1 режим ( всего 1, 2, 3, 4 )
void ILI9341_rotation( uint8_t rotation ){
	
	ILI9341_Select();
	
	ILI9341_SendCmd( 0x36 ); // MADCTL

	// для подгона под любой другой нужно отнимать разницу пикселей

	  switch (rotation) {
		
		case 1:
			//== 2.2" 240 x 320 ILI9341 =================================================
				{
					uint8_t data[] = { (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR) };
					ILI9341_SendData(data, sizeof(data));
				}
				ILI9341_Width = 240;
				ILI9341_Height = 320;
				ILI9341_FillScreen(0);

			//==========================================================================
		 break;
		
		case 2:
			//== 2.2" 240 x 320 ILI9341 =================================================
				{
					uint8_t data[] = { (ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR) };
					ILI9341_SendData(data, sizeof(data));
				}
				ILI9341_Width = 320;
				ILI9341_Height = 240;		
				ILI9341_FillScreen(0);

			//==========================================================================
		 break;
		
	   case 3:
			//== 2.2" 240 x 320 ILI9341 =================================================
				{
					uint8_t data[] = { (ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR) };
					ILI9341_SendData(data, sizeof(data));
				}
				ILI9341_Width = 240;
				ILI9341_Height = 320;
				ILI9341_FillScreen(0);

			//==========================================================================
		 break;
	   
	   case 4:
			//== 2.2" 240 x 320 ILI9341 =================================================
				{
					uint8_t data[] = { (ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR) };
					ILI9341_SendData(data, sizeof(data));
				}
				ILI9341_Width = 320;
				ILI9341_Height = 240;
				ILI9341_FillScreen(0);

			//==========================================================================
		 break;
	   
	   default:
		 break;
	  }
	  
	  ILI9341_Unselect();
}
//==============================================================================


//==============================================================================
// Процедура рисования иконки монохромной
//==============================================================================
void ILI9341_DrawBitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, uint16_t color){

    int16_t byteWidth = (w + 7) / 8; 	// Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    for(int16_t j=0; j<h; j++, y++){
		
        for(int16_t i=0; i<w; i++){
			
            if(i & 7){
               byte <<= 1;
            }
            else{
               byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
            }
			
            if(byte & 0x80){
				ILI9341_DrawPixel(x+i, y, color);
			}
        }
    }
}
//==============================================================================


//==============================================================================
// Процедура рисования прямоугольник с закругленніми краями ( заполненый )
//==============================================================================
void ILI9341_DrawFillRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, int16_t cornerRadius, uint16_t color) {
	
	int16_t max_radius = ((width < height) ? width : height) / 2; // 1/2 minor axis
  if (cornerRadius > max_radius){
    cornerRadius = max_radius;
	}
	
  ILI9341_DrawRectangleFilled(x + cornerRadius, y, x + cornerRadius + width - 2 * cornerRadius, y + height, color);
  // draw four corners
  ILI9341_DrawFillCircleHelper(x + width - cornerRadius - 1, y + cornerRadius, cornerRadius, 1, height - 2 * cornerRadius - 1, color);
  ILI9341_DrawFillCircleHelper(x + cornerRadius, y + cornerRadius, cornerRadius, 2, height - 2 * cornerRadius - 1, color);
}
//==============================================================================

//==============================================================================
// Процедура рисования половины окружности ( правая или левая ) ( заполненый )
//==============================================================================
void ILI9341_DrawFillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color) {

  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;
  int16_t px = x;
  int16_t py = y;

  delta++; // Avoid some +1's in the loop

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    if (x < (y + 1)) {
      if (corners & 1){
        ILI9341_DrawLine(x0 + x, y0 - y, x0 + x, y0 - y - 1 + 2 * y + delta, color);
			}
      if (corners & 2){
        ILI9341_DrawLine(x0 - x, y0 - y, x0 - x, y0 - y - 1 + 2 * y + delta, color);
			}
    }
    if (y != py) {
      if (corners & 1){
        ILI9341_DrawLine(x0 + py, y0 - px, x0 + py, y0 - px - 1 + 2 * px + delta, color);
			}
      if (corners & 2){
        ILI9341_DrawLine(x0 - py, y0 - px, x0 - py, y0 - px - 1 + 2 * px + delta, color);
			}
			py = y;
    }
    px = x;
  }
}
//==============================================================================																		

//==============================================================================
// Процедура рисования четверти окружности (закругление, дуга) ( ширина 1 пиксель)
//==============================================================================
void ILI9341_DrawCircleHelper(int16_t x0, int16_t y0, int16_t radius, int8_t quadrantMask, uint16_t color)
{
    int16_t f = 1 - radius ;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * radius;
    int16_t x = 0;
    int16_t y = radius;

    while (x <= y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
				
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (quadrantMask & 0x4) {
            ILI9341_DrawPixel(x0 + x, y0 + y, color);
            ILI9341_DrawPixel(x0 + y, y0 + x, color);;
        }
        if (quadrantMask & 0x2) {
			ILI9341_DrawPixel(x0 + x, y0 - y, color);
            ILI9341_DrawPixel(x0 + y, y0 - x, color);
        }
        if (quadrantMask & 0x8) {
			ILI9341_DrawPixel(x0 - y, y0 + x, color);
            ILI9341_DrawPixel(x0 - x, y0 + y, color);
        }
        if (quadrantMask & 0x1) {
            ILI9341_DrawPixel(x0 - y, y0 - x, color);
            ILI9341_DrawPixel(x0 - x, y0 - y, color);
        }
    }
}
//==============================================================================		

//==============================================================================
// Процедура рисования прямоугольник с закругленніми краями ( пустотелый )
//==============================================================================
void ILI9341_DrawRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, int16_t cornerRadius, uint16_t color) {
	
	int16_t max_radius = ((width < height) ? width : height) / 2; // 1/2 minor axis
  if (cornerRadius > max_radius){
    cornerRadius = max_radius;
	}
	
  ILI9341_DrawLine(x + cornerRadius, y, x + cornerRadius + width -1 - 2 * cornerRadius, y, color);         // Top
  ILI9341_DrawLine(x + cornerRadius, y + height - 1, x + cornerRadius + width - 1 - 2 * cornerRadius, y + height - 1, color); // Bottom
  ILI9341_DrawLine(x, y + cornerRadius, x, y + cornerRadius + height - 1 - 2 * cornerRadius, color);         // Left
  ILI9341_DrawLine(x + width - 1, y + cornerRadius, x + width - 1, y + cornerRadius + height - 1 - 2 * cornerRadius, color); // Right
	
  // draw four corners
	ILI9341_DrawCircleHelper(x + cornerRadius, y + cornerRadius, cornerRadius, 1, color);
  ILI9341_DrawCircleHelper(x + width - cornerRadius - 1, y + cornerRadius, cornerRadius, 2, color);
	ILI9341_DrawCircleHelper(x + width - cornerRadius - 1, y + height - cornerRadius - 1, cornerRadius, 4, color);
  ILI9341_DrawCircleHelper(x + cornerRadius, y + height - cornerRadius - 1, cornerRadius, 8, color);
}
//==============================================================================

//==============================================================================
// Процедура рисования линия толстая ( последний параметр толщина )
//==============================================================================
void ILI9341_DrawLineThick(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, uint8_t thick) {
	const int16_t deltaX = abs(x2 - x1);
	const int16_t deltaY = abs(y2 - y1);
	const int16_t signX = x1 < x2 ? 1 : -1;
	const int16_t signY = y1 < y2 ? 1 : -1;

	int16_t error = deltaX - deltaY;

	if (thick > 1){
		ILI9341_DrawCircleFilled(x2, y2, thick >> 1, color);
	}
	else{
		ILI9341_DrawPixel(x2, y2, color);
	}

	while (x1 != x2 || y1 != y2) {
		if (thick > 1){
			ILI9341_DrawCircleFilled(x1, y1, thick >> 1, color);
		}
		else{
			ILI9341_DrawPixel(x1, y1, color);
		}

		const int16_t error2 = error * 2;
		if (error2 > -deltaY) {
			error -= deltaY;
			x1 += signX;
		}
		if (error2 < deltaX) {
			error += deltaX;
			y1 += signY;
		}
	}
}
//==============================================================================		

//==============================================================================
// Процедура рисования дуга толстая ( часть круга )
//==============================================================================
void ILI9341_DrawArc(int16_t x0, int16_t y0, int16_t radius, int16_t startAngle, int16_t endAngle, uint16_t color, uint8_t thick) {
	
	int16_t xLast = -1, yLast = -1;
	startAngle -= 90;
	endAngle -= 90;

	for (int16_t angle = startAngle; angle <= endAngle; angle += 2) {
		float angleRad = (float) angle * PI / 180;
		int x = cos(angleRad) * radius + x0;
		int y = sin(angleRad) * radius + y0;

		if (xLast == -1 || yLast == -1) {
			xLast = x;
			yLast = y;
			continue;
		}

		if (thick > 1){
			ILI9341_DrawLineThick(xLast, yLast, x, y, color, thick);
		}
		else{
			ILI9341_DrawLine(xLast, yLast, x, y, color);
		}

		xLast = x;
		yLast = y;
	}
}
//==============================================================================



//#########################################################################################################################
//#########################################################################################################################

/*

//==============================================================================


//==============================================================================
// Тест поочерёдно выводит на дисплей картинки с SD-флешки
//==============================================================================
void Test_displayImage(const char* fname)
{
  FRESULT res;
  
  FIL file;
  res = f_open(&file, fname, FA_READ);
  if (res != FR_OK)
    return;

  unsigned int bytesRead;
  uint8_t header[34];
  res = f_read(&file, header, sizeof(header), &bytesRead);
  if (res != FR_OK) 
  {
    f_close(&file);
    return;
  }

  if ((header[0] != 0x42) || (header[1] != 0x4D))
  {
    f_close(&file);
    return;
  }

  uint32_t imageOffset = header[10] | (header[11] << 8) | (header[12] << 16) | (header[13] << 24);
  uint32_t imageWidth  = header[18] | (header[19] << 8) | (header[20] << 16) | (header[21] << 24);
  uint32_t imageHeight = header[22] | (header[23] << 8) | (header[24] << 16) | (header[25] << 24);
  uint16_t imagePlanes = header[26] | (header[27] << 8);

  uint16_t imageBitsPerPixel = header[28] | (header[29] << 8);
  uint32_t imageCompression  = header[30] | (header[31] << 8) | (header[32] << 16) | (header[33] << 24);

  if((imagePlanes != 1) || (imageBitsPerPixel != 24) || (imageCompression != 0))
  {
    f_close(&file);
    return;
  }

  res = f_lseek(&file, imageOffset);
  if(res != FR_OK)
  {
    f_close(&file);
    return;
  }

  // Подготавливаем буфер строки картинки для вывода
  uint8_t imageRow[(240 * 3 + 3) & ~3];
  uint16_t PixBuff[240];

  for (uint32_t y = 0; y < imageHeight; y++)
  {
    res = f_read(&file, imageRow, (imageWidth * 3 + 3) & ~3, &bytesRead);
    if (res != FR_OK)
    {
      f_close(&file);
      return;
    }
      
    uint32_t rowIdx = 0;
    for (uint32_t x = 0; x < imageWidth; x++)
    {
      uint8_t b = imageRow[rowIdx++];
      uint8_t g = imageRow[rowIdx++];
      uint8_t r = imageRow[rowIdx++];
      PixBuff[x] = RGB565(r, g, b);
    }

    dispcolor_DrawPartXY(0, imageHeight - y - 1, imageWidth, 1, PixBuff);
  }

  f_close(&file);
}
//==============================================================================


//==============================================================================
// Тест вывода картинок на дисплей
//==============================================================================
void Test240x240_Images(void)
{
  FATFS fatfs;
  DIR DirInfo;
  FILINFO FileInfo;
  FRESULT res;
  
  res = f_mount(&fatfs, "0", 1);
  if (res != FR_OK)
    return;
  
  res = f_chdir("/240x240");
  if (res != FR_OK)
    return;

  res = f_opendir(&DirInfo, "");
  if (res != FR_OK)
    return;
  
  while (1)
  {
    res = f_readdir(&DirInfo, &FileInfo);
    if (res != FR_OK)
      break;
      
    if (FileInfo.fname[0] == 0)
      break;
      
    char *pExt = strstr(FileInfo.fname, ".BMP");
    if (pExt)
    {
      Test_displayImage(FileInfo.fname);
      delay_ms(2000);
    }
  }
}
//==============================================================================


//==============================================================================
// Процедура заполнения прямоугольной области из буфера. Порядок заполнения экрана Y - X
//==============================================================================
void ST77xx_DrawPartYX(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pBuff)
{
  if ((x >= ST77xx_Width) || (y >= ST77xx_Height))
    return;
  
  if ((x + w - 1) >= ST77xx_Width)
    w = ST77xx_Width - x;
  
  if ((y + h - 1) >= ST77xx_Height)
    h = ST77xx_Height - y;

  ST77xx_SetWindow(x, y, x + w - 1, y + h - 1);

  for (uint32_t i = 0; i < (h * w); i++)
    ST77xx_RamWrite(pBuff++, 1);
}
//==============================================================================


//==============================================================================
// Процедура заполнения прямоугольной области из буфера. Порядок заполнения экрана X - Y
//==============================================================================
void ST77xx_DrawPartXY(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pBuff)
{
  if ((x >= ST77xx_Width) || (y >= ST77xx_Height))
    return;
  
  if ((x + w - 1) >= ST77xx_Width)
    w = ST77xx_Width - x;
  
  if ((y + h - 1) >= ST77xx_Height)
    h = ST77xx_Height - y;

  for (uint16_t iy = y; iy < y + h; iy++)
  {
    ST77xx_SetWindow(x, iy, x + w - 1, iy + 1);
    for (x = w; x > 0; x--)
      ST77xx_RamWrite(pBuff++, 1);
  }
}
//==============================================================================

//########################################################################################################

*/




/************************ (C) COPYRIGHT GKP *****END OF FILE****/
