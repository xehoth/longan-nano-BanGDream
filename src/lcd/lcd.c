#include "lcd/lcd.h"
#include "lcd/oledfont.h"
#include "lcd/bmp.h"
u16 BACK_COLOR;  // Background color

/******************************************************************************
       Function description: LCD serial data write function
       Entry data: serial data to be written to dat
       Return value: None
******************************************************************************/
// ------------------------------------------------------------------------

#define spi_wait_idle()                     \
  do {                                      \
    while (SPI_STAT(SPI0) & SPI_STAT_TRANS) \
      ;                                     \
  } while (0)
#define spi_wait_tbe()                       \
  do {                                       \
    while (!(SPI_STAT(SPI0) & SPI_STAT_TBE)) \
      ;                                      \
  } while (0)
#define lcd_mode_cmd()                 \
  do {                                 \
    gpio_bit_reset(GPIOB, GPIO_PIN_0); \
  } while (0)
#define lcd_mode_data()              \
  do {                               \
    gpio_bit_set(GPIOB, GPIO_PIN_0); \
  } while (0)
#define lcd_cs_enable()                \
  do {                                 \
    gpio_bit_reset(GPIOB, GPIO_PIN_2); \
  } while (0)
#define lcd_cs_disable()             \
  do {                               \
    gpio_bit_set(GPIOB, GPIO_PIN_2); \
  } while (0)

// ------------------------------------------------------------------------

typedef enum {
  WAIT_NONE = 0,
  WAIT_READ_U24 = 1,
  WAIT_WRITE_U24 = 2,
} WaitStatus;

static WaitStatus g_waitStatus = WAIT_NONE;
static uint32_t g_fbAddress = 0;
static int g_fbEnabled = 0;

// ------------------------------------------------------------------------
// Internal functions.
// ------------------------------------------------------------------------

void spiSet8bit() {
  if (SPI_CTL0(SPI0) & (uint32_t)(SPI_CTL0_FF16)) {
    SPI_CTL0(SPI0) &= ~(uint32_t)(SPI_CTL0_SPIEN);
    SPI_CTL0(SPI0) &= ~(uint32_t)(SPI_CTL0_FF16);
    SPI_CTL0(SPI0) |= (uint32_t)(SPI_CTL0_SPIEN);
  }
}

void spiSet16bit() {
  if (!(SPI_CTL0(SPI0) & (uint32_t)(SPI_CTL0_FF16))) {
    SPI_CTL0(SPI0) &= ~(uint32_t)(SPI_CTL0_SPIEN);
    SPI_CTL0(SPI0) |= (uint32_t)(SPI_CTL0_FF16);
    SPI_CTL0(SPI0) |= (uint32_t)(SPI_CTL0_SPIEN);
  }
}

void dmaSendU16(const void *src, uint32_t count) {
  spi_wait_idle();
  lcd_mode_data();
  spiSet16bit();
  dma_channel_disable(DMA0, DMA_CH2);
  dma_memory_width_config(DMA0, DMA_CH2, DMA_MEMORY_WIDTH_16BIT);
  dma_periph_width_config(DMA0, DMA_CH2, DMA_PERIPHERAL_WIDTH_16BIT);
  dma_memory_address_config(DMA0, DMA_CH2, (uint32_t)src);
  dma_memory_increase_enable(DMA0, DMA_CH2);
  dma_transfer_number_config(DMA0, DMA_CH2, count);
  dma_channel_enable(DMA0, DMA_CH2);
}

uint32_t g_dma_const_value = 0;

void dmaSendConstU16(uint16_t data, uint32_t count) {
  spi_wait_idle();
  g_dma_const_value = data;
  lcd_mode_data();
  spiSet16bit();
  dma_channel_disable(DMA0, DMA_CH2);
  dma_memory_width_config(DMA0, DMA_CH2, DMA_MEMORY_WIDTH_16BIT);
  dma_periph_width_config(DMA0, DMA_CH2, DMA_PERIPHERAL_WIDTH_16BIT);
  dma_memory_address_config(DMA0, DMA_CH2, (uint32_t)(&g_dma_const_value));
  dma_memory_increase_disable(DMA0, DMA_CH2);
  dma_transfer_number_config(DMA0, DMA_CH2, count);
  dma_channel_enable(DMA0, DMA_CH2);
}

void lcdReg(uint8_t x) {
  spi_wait_idle();
  spiSet8bit();
  lcd_mode_cmd();
  spi_i2s_data_transmit(SPI0, x);
}

void lcdU8(uint8_t x) {
  spi_wait_idle();
  spiSet8bit();
  lcd_mode_data();
  spi_i2s_data_transmit(SPI0, x);
}

void lcdU8c(uint8_t x) {
  spi_wait_tbe();
  spi_i2s_data_transmit(SPI0, x);
}

void lcdU16(uint16_t x) {
  spi_wait_idle();
  spiSet16bit();
  lcd_mode_data();
  spi_i2s_data_transmit(SPI0, x);
}

void lcdU16c(uint16_t x) {
  spi_wait_tbe();
  spi_i2s_data_transmit(SPI0, x);
}

void lcdSetAddr(int x, int y, int w, int h) {
  lcdReg(0x2a);
  lcdU16(x + 1);
  lcdU16c(x + w);
  lcdReg(0x2b);
  lcdU16(y + 26);
  lcdU16c(y + h + 25);
  lcdReg(0x2c);
}

void lcd_wait(void) {
  if (g_fbEnabled) return;

  if (g_waitStatus == WAIT_NONE) return;

  if (g_waitStatus == WAIT_READ_U24) {
    // Poll until reception is complete.
    while (dma_transfer_number_get(DMA0, DMA_CH1))
      ;

    // Reception is complete, reconfigure SPI for sending and toggle LCD CS to
    // stop transmission.
    dma_channel_disable(DMA0, DMA_CH1);
    spi_disable(SPI0);
    lcd_cs_disable();
    SPI_CTL0(SPI0) =
        (uint32_t)(SPI_MASTER | SPI_TRANSMODE_FULLDUPLEX | SPI_FRAMESIZE_8BIT |
                   SPI_NSS_SOFT | SPI_ENDIAN_MSB | SPI_CK_PL_LOW_PH_1EDGE |
                   SPI_PSC_8);
    SPI_CTL1(SPI0) = (uint32_t)(SPI_CTL1_DMATEN);
    lcd_cs_enable();
    spi_enable(SPI0);

    // Return to normal color mode.
    lcdReg(0x3a);  // COLMOD
    lcdU8(0x55);   // RGB565 (transferred as 16b)

    // Clear wait status and return.
    g_waitStatus = WAIT_NONE;
    return;
  }

  if (g_waitStatus == WAIT_WRITE_U24) {
    // Wait until send is complete, then restore normal color mode.
    spi_wait_idle();
    lcdReg(0x3a);  // COLMOD
    lcdU8(0x55);   // RGB565 (transferred as 16b)

    // Clear wait status and return.
    g_waitStatus = WAIT_NONE;
    return;
  }
}

/******************************************************************************
       Function description: LCD write data
       Entry data: data written by dat
       Return value: None
******************************************************************************/
void LCD_WR_DATA8(u8 dat) { lcdU8(dat); }

/******************************************************************************
           Function description: LCD write data
       Entry data: data written by dat
       Return value: None
******************************************************************************/
void LCD_WR_DATA(u16 dat) { lcdU16(dat); }

/******************************************************************************
           Function description: LCD write command
       Entry data: command written by dat
       Return value: None
******************************************************************************/
void LCD_WR_REG(u8 dat) { lcdReg(dat); }

/******************************************************************************
           Function description: Set start and end addresses
       Entry data: x1, x2 set the start and end addresses of the column
                   y1, y2 set the start and end addresses of the line
       Return value: None
******************************************************************************/
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2) {
  lcdSetAddr(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}

/******************************************************************************
           Function description: LCD initialization function
       Entry data: None
       Return value: None
******************************************************************************/
void Lcd_Init(void) {
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOB);
  rcu_periph_clock_enable(RCU_AF);
  rcu_periph_clock_enable(RCU_DMA0);
  rcu_periph_clock_enable(RCU_SPI0);

  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_7);
  gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,
            GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);
  gpio_bit_reset(GPIOB, GPIO_PIN_0 | GPIO_PIN_1);  // DC=0, RST=0
  lcd_cs_disable();

  delay_1ms(1);
  gpio_bit_set(GPIOB, GPIO_PIN_1);  // RST=1
  delay_1ms(5);

  // Deinit SPI and DMA.
  spi_i2s_deinit(SPI0);
  dma_deinit(DMA0, DMA_CH1);
  dma_deinit(DMA0, DMA_CH2);

  // Configure DMA, do not enable.
  DMA_CHCTL(DMA0, DMA_CH1) =
      (uint32_t)(DMA_PRIORITY_ULTRA_HIGH | DMA_CHXCTL_MNAGA);  // Receive.
  DMA_CHCTL(DMA0, DMA_CH2) =
      (uint32_t)(DMA_PRIORITY_ULTRA_HIGH | DMA_CHXCTL_DIR);  // Transmit.
  DMA_CHPADDR(DMA0, DMA_CH1) = (uint32_t)&SPI_DATA(SPI0);
  DMA_CHPADDR(DMA0, DMA_CH2) = (uint32_t)&SPI_DATA(SPI0);

  // Configure and enable SPI.
  SPI_CTL0(SPI0) =
      (uint32_t)(SPI_MASTER | SPI_TRANSMODE_FULLDUPLEX | SPI_FRAMESIZE_8BIT |
                 SPI_NSS_SOFT | SPI_ENDIAN_MSB | SPI_CK_PL_LOW_PH_1EDGE |
                 SPI_PSC_8);
  SPI_CTL1(SPI0) = (uint32_t)(SPI_CTL1_DMATEN);
  spi_enable(SPI0);

  // Enable lcd controller.
  lcd_cs_enable();

  // Initialization settings. Based on lcd.c in gd32v_lcd example.
  static const uint8_t init_sequence[] = {
      0x21, 0xff, 0xb1, 0x05, 0x3a, 0x3a, 0xff, 0xb2, 0x05, 0x3a, 0x3a, 0xff,
      0xb3, 0x05, 0x3a, 0x3a, 0x05, 0x3a, 0x3a, 0xff, 0xb4, 0x03, 0xff, 0xc0,
      0x62, 0x02, 0x04, 0xff, 0xc1, 0xc0, 0xff, 0xc2, 0x0d, 0x00, 0xff, 0xc3,
      0x8d, 0x6a, 0xff, 0xc4, 0x8d, 0xee, 0xff, 0xc5, 0x0e, 0xff, 0xe0, 0x10,
      0x0e, 0x02, 0x03, 0x0e, 0x07, 0x02, 0x07, 0x0a, 0x12, 0x27, 0x37, 0x00,
      0x0d, 0x0e, 0x10, 0xff, 0xe1, 0x10, 0x0e, 0x03, 0x03, 0x0f, 0x06, 0x02,
      0x08, 0x0a, 0x13, 0x26, 0x36, 0x00, 0x0d, 0x0e, 0x10, 0xff, 0x3a, 0x55,
      0xff, 0x36, 0x78, 0xff, 0x29, 0xff, 0x11, 0xff, 0xff};

  // Initialize the display.
  for (const uint8_t *p = init_sequence; *p != 0xff; p++) {
    lcdReg(*p++);
    if (*p == 0xff) continue;
    spi_wait_idle();
    lcd_mode_data();
    while (*p != 0xff) lcdU8c(*p++);
  }

  // Clear display.
  LCD_Clear(0);

  // Init internal state.
  g_waitStatus = WAIT_NONE;
  g_fbAddress = 0;
  g_fbEnabled = 0;
}

/******************************************************************************
           Function description: LCD clear screen function
       Entry data: None
       Return value: None
******************************************************************************/
void LCD_Clear(u16 Color) {
  if (g_fbEnabled) return;
  lcd_wait();
  lcdSetAddr(0, 0, 160, 80);
  dmaSendConstU16(Color, 160 * 80);
}

/******************************************************************************
           Function description: LCD display Chinese characters
       Entry data: x, y starting coordinates
                   index Chinese character number
                   size font size
       Return value: None
******************************************************************************/
void LCD_ShowChinese(u16 x, u16 y, u8 index, u8 size, u16 color) {
  u8 i, j;
  u8 *temp, size1;
  if (size == 16) {
    temp = Hzk16;
  }  //选择字号
  if (size == 32) {
    temp = Hzk32;
  }
  LCD_Address_Set(x, y, x + size - 1, y + size - 1);  //设置一个汉字的区域
  size1 = size * size / 8;  //一个汉字所占的字节
  temp += index * size1;    //写入的起始位置
  for (j = 0; j < size1; j++) {
    for (i = 0; i < 8; i++) {
      if ((*temp & (1 << i)) != 0)  //从数据的低位开始读
      {
        LCD_WR_DATA(color);  //点亮
      } else {
        LCD_WR_DATA(BACK_COLOR);  //不点亮
      }
    }
    temp++;
  }
}

/******************************************************************************
           Function description: LCD draws point
       Entry data: x, y starting coordinates
       Return value: None
******************************************************************************/
void LCD_DrawPoint(u16 x, u16 y, u16 color) {
  LCD_Address_Set(x, y, x, y);  //设置光标位置
  LCD_WR_DATA(color);
}

/******************************************************************************
           Function description: LCD draws a large dot
       Entry data: x, y starting coordinates
       Return value: None
******************************************************************************/
void LCD_DrawPoint_big(u16 x, u16 y, u16 color) {
  LCD_Fill(x - 1, y - 1, x + 1, y + 1, color);
}

/******************************************************************************
           Function description: fill color in the specified area
       Entry data: xsta, ysta starting coordinates
                   xend, yend termination coordinates
       Return value: None
******************************************************************************/
void LCD_Fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color) {
  u16 i, j;
  LCD_Address_Set(xsta, ysta, xend, yend);  //设置光标位置
  for (i = ysta; i <= yend; i++) {
    for (j = xsta; j <= xend; j++) LCD_WR_DATA(color);  //设置光标位置
  }
}

/******************************************************************************
           Function description: draw a line
       Entry data: x1, y1 starting coordinates
                   x2, y2 terminating coordinates
       Return value: None
******************************************************************************/
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color) {
  u16 t;
  int xerr = 0, yerr = 0, delta_x, delta_y, distance;
  int incx, incy, uRow, uCol;
  delta_x = x2 - x1;  //计算坐标增量
  delta_y = y2 - y1;
  uRow = x1;  //画线起点坐标
  uCol = y1;
  if (delta_x > 0)
    incx = 1;  //设置单步方向
  else if (delta_x == 0)
    incx = 0;  //垂直线
  else {
    incx = -1;
    delta_x = -delta_x;
  }
  if (delta_y > 0)
    incy = 1;
  else if (delta_y == 0)
    incy = 0;  //水平线
  else {
    incy = -1;
    delta_y = -delta_x;
  }
  if (delta_x > delta_y)
    distance = delta_x;  //选取基本增量坐标轴
  else
    distance = delta_y;
  for (t = 0; t < distance + 1; t++) {
    LCD_DrawPoint(uRow, uCol, color);  //画点
    xerr += delta_x;
    yerr += delta_y;
    if (xerr > distance) {
      xerr -= distance;
      uRow += incx;
    }
    if (yerr > distance) {
      yerr -= distance;
      uCol += incy;
    }
  }
}

/******************************************************************************
           Function description: draw a rectangle
       Entry data: x1, y1 starting coordinates
                   x2, y2 terminating coordinates
       Return value: None
******************************************************************************/
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color) {
  LCD_DrawLine(x1, y1, x2, y1, color);
  LCD_DrawLine(x1, y1, x1, y2, color);
  LCD_DrawLine(x1, y2, x2, y2, color);
  LCD_DrawLine(x2, y1, x2, y2, color);
}

/******************************************************************************
           Function description: draw circle
       Entry data: x0, y0 center coordinates
                   r radius
       Return value: None
******************************************************************************/
void LCD_DrawCircle(u16 x0, u16 y0, u8 r, u16 color) {
  int a, b;
  // int di;
  a = 0;
  b = r;
  while (a <= b) {
    LCD_DrawPoint(x0 - b, y0 - a, color);  // 3
    LCD_DrawPoint(x0 + b, y0 - a, color);  // 0
    LCD_DrawPoint(x0 - a, y0 + b, color);  // 1
    LCD_DrawPoint(x0 - a, y0 - b, color);  // 2
    LCD_DrawPoint(x0 + b, y0 + a, color);  // 4
    LCD_DrawPoint(x0 + a, y0 - b, color);  // 5
    LCD_DrawPoint(x0 + a, y0 + b, color);  // 6
    LCD_DrawPoint(x0 - b, y0 + a, color);  // 7
    a++;
    if ((a * a + b * b) >
        (r * r))  // Determine whether the points to be drawn are too far away
    {
      b--;
    }
  }
}

/******************************************************************************
           Function description: display characters
       Entry data: x, y starting point coordinates
                   num characters to display
                   mode 1 superimposed mode 0 non-superimposed mode
       Return value: None
******************************************************************************/
void LCD_ShowChar(u16 x, u16 y, u8 num, u8 mode, u16 color) {
  u8 temp;
  u8 pos, t;
  u16 x0 = x;
  if (x > LCD_W - 16 || y > LCD_H - 16) return;  // Settings window
  num = num - ' ';                               // Get offset value
  LCD_Address_Set(x, y, x + 8 - 1, y + 16 - 1);  // Set cursor position
  if (!mode)                                     // Non-overlapping
  {
    for (pos = 0; pos < 16; pos++) {
      temp = asc2_1608[(u16)num * 16 + pos];  // Call 1608 font
      for (t = 0; t < 8; t++) {
        if (temp & 0x01)
          LCD_WR_DATA(color);
        else
          LCD_WR_DATA(BACK_COLOR);
        temp >>= 1;
        x++;
      }
      x = x0;
      y++;
    }
  } else  // overlapping mode
  {
    for (pos = 0; pos < 16; pos++) {
      temp = asc2_1608[(u16)num * 16 + pos];  // Call 1608 font
      for (t = 0; t < 8; t++) {
        if (temp & 0x01) LCD_DrawPoint(x + t, y + pos, color);  // Draw a dot
        temp >>= 1;
      }
    }
  }
}

/******************************************************************************
           Function description: display string
       Entry data: x, y starting point coordinates
                   *p string start address
       Return value: None
******************************************************************************/
void LCD_ShowString(u16 x, u16 y, const u8 *p, u16 color) {
  while (*p != '\0') {
    if (x > LCD_W - 16) {
      x = 0;
      y += 16;
    }
    if (y > LCD_H - 16) {
      y = x = 0;
      LCD_Clear(RED);
    }
    LCD_ShowChar(x, y, *p, 0, color);
    x += 8;
    p++;
  }
}

/******************************************************************************
           Function description: display numbers
       Entry data: base m, n exponent
       Return value: None
******************************************************************************/
u32 mypow(u8 m, u8 n) {
  u32 result = 1;
  while (n--) result *= m;
  return result;
}

/******************************************************************************
           Function description: display numbers
       Entry data: x, y starting point coordinates
                   num number to display
                   len number of digits to display
       Return value: None
******************************************************************************/
void LCD_ShowNum(u16 x, u16 y, u16 num, u8 len, u16 color) {
  u8 t, temp;
  u8 enshow = 0;
  for (t = 0; t < len; t++) {
    temp = (num / mypow(10, len - t - 1)) % 10;
    if (enshow == 0 && t < (len - 1)) {
      if (temp == 0) {
        LCD_ShowChar(x + 8 * t, y, ' ', 0, color);
        continue;
      } else
        enshow = 1;
    }
    LCD_ShowChar(x + 8 * t, y, temp + 48, 0, color);
  }
}

/******************************************************************************
           Function description: display decimal
       Entry data: x, y starting point coordinates
                   num decimal to display
                   len number of digits to display
       Return value: None
******************************************************************************/
void LCD_ShowNum1(u16 x, u16 y, float num, u8 len, u16 color) {
  u8 t, temp;
  // u8 enshow=0;
  u16 num1;
  num1 = num * 100;
  for (t = 0; t < len; t++) {
    temp = (num1 / mypow(10, len - t - 1)) % 10;
    if (t == (len - 2)) {
      LCD_ShowChar(x + 8 * (len - 2), y, '.', 0, color);
      t++;
      len += 1;
    }
    LCD_ShowChar(x + 8 * t, y, temp + 48, 0, color);
  }
}

/******************************************************************************
           Function description: display 160x40 16bit (RGB565) picture
       Entry data: x, y starting point coordinates
       Return value: None
******************************************************************************/
void LCD_ShowPicture(u16 x1, u16 y1, u16 x2, u16 y2) {
  int i;
  LCD_Address_Set(x1, y1, x2, y2);
  for (i = 0; i < 12800; i++) {
    // LCD_WR_DATA8(image[i*2+1]);
    LCD_WR_DATA8(image[i]);
  }
}

void LCD_ShowPicture16(u16 x1, u16 y1, u16 x2, u16 y2) {
  int i;
  LCD_Address_Set(x1, y1, x2, y2);
  for (i = 0; i < 6400; i++) {
    // LCD_WR_DATA8(image[i*2+1]);
    LCD_WR_DATA8(image[i * 2 + 1]);
    LCD_WR_DATA8(image[i * 2 + 0]);
  }
}

// void LCD_ShowLogo(void) {
//   int i;
//   LCD_Address_Set(0, 0, 159, 75);
//   for (i = 0; i < 25600; i++) {
//     LCD_WR_DATA8(logo_bmp[i]);
//   }
// }

void renderFrameBuffer(int side) {
	// if (g_fbEnabled) return;
  // lcd_wait();
  // LCD_Address_Set(0, 0, 159, 79);
	// dma_send_u16(image, 6400);
  if (!side) {
    lcdWriteU16(0, 0, 160, 40, image);
  } else {
    lcdWriteU16(0, 40, 160, 40, image);
  }
}

void lcdWriteU16(int x, int y, int w, int h, const void *buffer) {
  if (g_fbEnabled) return;
  lcd_wait();
  lcdSetAddr(x, y, w, h);
  dmaSendU16(buffer, w * h);
}