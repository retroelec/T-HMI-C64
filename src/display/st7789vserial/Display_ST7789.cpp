#include "../../Config.h"
#ifdef USE_ST7789VSERIAL
#include "Display_ST7789.h"

SPIClass LCDspi(FSPI);
void SPI_Init() { LCDspi.begin(Config::SCLK, Config::MISO, Config::MOSI); }

void LCD_WriteCommand(uint8_t Cmd) {
  LCDspi.beginTransaction(SPISettings(SPIFreq, MSBFIRST, SPI_MODE0));
  digitalWrite(Config::LCD_CS, LOW);
  digitalWrite(Config::LCD_DC, LOW);
  LCDspi.transfer(Cmd);
  digitalWrite(Config::LCD_CS, HIGH);
  LCDspi.endTransaction();
}
void LCD_WriteData(uint8_t Data) {
  LCDspi.beginTransaction(SPISettings(SPIFreq, MSBFIRST, SPI_MODE0));
  digitalWrite(Config::LCD_CS, LOW);
  digitalWrite(Config::LCD_DC, HIGH);
  LCDspi.transfer(Data);
  digitalWrite(Config::LCD_CS, HIGH);
  LCDspi.endTransaction();
}
void LCD_WriteData_Word(uint16_t Data) {
  LCDspi.beginTransaction(SPISettings(SPIFreq, MSBFIRST, SPI_MODE0));
  digitalWrite(Config::LCD_CS, LOW);
  digitalWrite(Config::LCD_DC, HIGH);
  LCDspi.transfer16(Data);
  digitalWrite(Config::LCD_CS, HIGH);
  LCDspi.endTransaction();
}
void LCD_WriteData_nbyte(uint8_t *SetData, uint8_t *ReadData, uint32_t Size) {
  LCDspi.beginTransaction(SPISettings(SPIFreq, MSBFIRST, SPI_MODE0));
  digitalWrite(Config::LCD_CS, LOW);
  digitalWrite(Config::LCD_DC, HIGH);
  LCDspi.transferBytes(SetData, ReadData, Size);
  digitalWrite(Config::LCD_CS, HIGH);
  LCDspi.endTransaction();
}

void LCD_Reset(void) {
  digitalWrite(Config::LCD_CS, LOW);
  delay(50);
  digitalWrite(Config::LCD_RST, LOW);
  delay(50);
  digitalWrite(Config::LCD_RST, HIGH);
  delay(50);
}
void LCD_Init(void) {
  pinMode(Config::LCD_CS, OUTPUT);
  pinMode(Config::LCD_DC, OUTPUT);
  pinMode(Config::LCD_RST, OUTPUT);
  SPI_Init();

  LCD_Reset();
  //************* Start Initial Sequence **********//

  delay(120);
  LCD_WriteCommand(0x29); // Display on
  delay(120);
  LCD_WriteCommand(0x11);
  delay(120); // ms
  LCD_WriteCommand(0x36);
  LCD_WriteData(160);

  LCD_WriteCommand(0x3A);
  LCD_WriteData(0x05);

  LCD_WriteCommand(0xB0);
  LCD_WriteData(0x00);
  LCD_WriteData(0xE8); // 5 to 6-bit conversion: r0 = r5, b0 = b5

  LCD_WriteCommand(0xB2);
  LCD_WriteData(0x0C);
  LCD_WriteData(0x0C);
  LCD_WriteData(0x00);
  LCD_WriteData(0x33);
  LCD_WriteData(0x33);

  LCD_WriteCommand(0xB7);
  LCD_WriteData(0x75); // VGH=14.97V,VGL=-7.67V

  LCD_WriteCommand(0xBB);
  LCD_WriteData(0x1A);

  LCD_WriteCommand(0xC0);
  LCD_WriteData(0x2C);

  LCD_WriteCommand(0xC2);
  LCD_WriteData(0x01);
  LCD_WriteData(0xFF);

  LCD_WriteCommand(0xC3);
  LCD_WriteData(0x13);

  LCD_WriteCommand(0xC4);
  LCD_WriteData(0x20);

  LCD_WriteCommand(0xC6);
  LCD_WriteData(0x0F);

  LCD_WriteCommand(0xD0);
  LCD_WriteData(0xA4);
  LCD_WriteData(0xA1);

  LCD_WriteCommand(0xD6);
  LCD_WriteData(0xA1);

  LCD_WriteCommand(0xE0);
  LCD_WriteData(0xD0);
  LCD_WriteData(0x0D);
  LCD_WriteData(0x14);
  LCD_WriteData(0x0D);
  LCD_WriteData(0x0D);
  LCD_WriteData(0x09);
  LCD_WriteData(0x38);
  LCD_WriteData(0x44);
  LCD_WriteData(0x4E);
  LCD_WriteData(0x3A);
  LCD_WriteData(0x17);
  LCD_WriteData(0x18);
  LCD_WriteData(0x2F);
  LCD_WriteData(0x30);

  LCD_WriteCommand(0xE1);
  LCD_WriteData(0xD0);
  LCD_WriteData(0x09);
  LCD_WriteData(0x0F);
  LCD_WriteData(0x08);
  LCD_WriteData(0x07);
  LCD_WriteData(0x14);
  LCD_WriteData(0x37);
  LCD_WriteData(0x44);
  LCD_WriteData(0x4D);
  LCD_WriteData(0x38);
  LCD_WriteData(0x15);
  LCD_WriteData(0x16);
  LCD_WriteData(0x2C);
  LCD_WriteData(0x2E);

  LCD_WriteCommand(0x21);
  LCD_WriteCommand(0x29);

  //  LCD_WriteCommand(0x2C);

  //  Touch_Init();
}
/******************************************************************************
function: Set the cursor position
parameter :
    Xstart:   Start uint16_t x coordinate
    Ystart:   Start uint16_t y coordinate
    Xend  :   End uint16_t coordinates
    Yend  :   End uint16_t coordinatesen
******************************************************************************/
void LCD_SetCursor(uint16_t Xstart, uint16_t Ystart, uint16_t Xend,
                   uint16_t Yend) {
  if (HORIZONTAL) {
    // set the X coordinates
    LCD_WriteCommand(0x2A);
    LCD_WriteData(Xstart >> 8);
    LCD_WriteData(Xstart + Offset_X);
    LCD_WriteData(Xend >> 8);
    LCD_WriteData(Xend + Offset_X);

    // set the Y coordinates
    LCD_WriteCommand(0x2B);
    LCD_WriteData(Ystart >> 8);
    LCD_WriteData(Ystart + Offset_Y);
    LCD_WriteData(Yend >> 8);
    LCD_WriteData(Yend + Offset_Y);
  } else {
    // set the X coordinates
    LCD_WriteCommand(0x2A);
    LCD_WriteData(Ystart >> 8);
    LCD_WriteData(Ystart + Offset_Y);
    LCD_WriteData(Yend >> 8);
    LCD_WriteData(Yend + Offset_Y);
    // set the Y coordinates
    LCD_WriteCommand(0x2B);
    LCD_WriteData(Xstart >> 8);
    LCD_WriteData(Xstart + Offset_X);
    LCD_WriteData(Xend >> 8);
    LCD_WriteData(Xend + Offset_X);
  }
  LCD_WriteCommand(0x2C);
}
/******************************************************************************
function: Refresh the image in an area
parameter :
    Xstart:   Start uint16_t x coordinate
    Ystart:   Start uint16_t y coordinate
    Xend  :   End uint16_t coordinates
    Yend  :   End uint16_t coordinates
    color :   Set the color
******************************************************************************/
void LCD_addWindow(uint16_t Xstart, uint16_t Ystart, uint16_t Xend,
                   uint16_t Yend, uint16_t *color) {
  uint16_t Show_Width = Xend - Xstart + 1;
  uint16_t Show_Height = Yend - Ystart + 1;
  uint32_t numBytes = Show_Width * Show_Height * sizeof(uint16_t);
  LCD_SetCursor(Xstart, Ystart, Xend, Yend);
  LCD_WriteData_nbyte((uint8_t *)color, NULL, numBytes);
}

// backlight
uint8_t LCD_Backlight = 100;
void Backlight_Init() {
  ledcAttach(Config::LCD_BACKLIGHT_PIN, Config::LCD_BACKLIGHT_FREQUENCY,
             Config::LCD_BACKLIGHT_RESOLUTION);
  ledcWrite(Config::LCD_BACKLIGHT_PIN, Config::LCD_BACKLIGHT_DUTYFACTOR);
  Set_Backlight(LCD_Backlight); // 0~100
}

void Set_Backlight(uint8_t Light) {
  if (Light > Backlight_MAX)
    printf("Set Backlight parameters in the range of 0 to 100 \r\n");
  else {
    uint32_t Backlight = Light * 10;
    if (Backlight == 1000)
      Backlight = 1024;
    ledcWrite(Config::LCD_BACKLIGHT_PIN, Backlight);
  }
}
#endif
