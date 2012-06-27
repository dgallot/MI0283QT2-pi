#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
}
#endif
#include "fonts.h"
#include "MI0283QT2.h"
#include <string>

#define read_byte(x) (*(uint8_t *)x)
#define read_word(x) ( (*(uint8_t *)x) |  ((*((uint8_t)x)+1) << 8))
#define read_dword(x) ( (*(uint8_t *)x) | ((*((uint8_t)x)+1) << 8))| ((*((uint8_t)x)+2) << 16))| ((*((uint8_t)x)+3) << 24))

#define PRINT_STARTX    (2)
#define PRINT_STARTY    (2)

#define LCD_ID          (0)
#define LCD_DATA        ((0x72)|(LCD_ID<<2))
#define LCD_REGISTER    ((0x70)|(LCD_ID<<2))

#define RST_DISABLE()   printf("RST_DISABLE\n");
#define RST_ENABLE()    printf("RST_ENABLE\n");


static void pabort(const char *s)
{
        perror(s);
        abort();
}

static void _delay_ms( int delay )
{
  usleep( delay );
}

static void delay_10ms(uint8_t ms) //delay of 10ms * x
{
  for(; ms!=0; ms--)
  {
    _delay_ms(10);
  }

  return;
}


//-------------------- Constructor --------------------


MI0283QT2::MI0283QT2(const std::string & dev) : device(dev.c_str())
{
printf("device : %s\n", dev.c_str() );
printf("device : %s\n", device.c_str() );

	txc = 0;
	mode = 3;
	bits = 8;
	speed = 60000000;
	delay = 0;
  return;
}


//-------------------- Public --------------------


void MI0283QT2::init()
{
   int ret = 0;
   printf("device : %s\n", device.c_str() );
        fd = open(device.c_str(), O_RDWR);
        if (fd < 0)
                pabort("Can't open device ... ");

        /*
         * spi mode
         */
        ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
        if (ret == -1)
                pabort("can't set spi mode");

        ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
        if (ret == -1)
                pabort("can't get spi mode");

        /*
         * bits per word
         */
        ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
        if (ret == -1)
                pabort("can't set bits per word");

        ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
        if (ret == -1)
                pabort("can't get bits per word");

        /*
         * max speed hz
         */
        ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        if (ret == -1)
                pabort("can't set max speed hz");

        ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
        if (ret == -1)
                pabort("can't get max speed hz");

        printf("spi mode: %d\n", mode);
        printf("bits per word: %d\n", bits);
        printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

  //reset display
  reset();
  p_size = 0;
  p_fg   = COLOR_BLACK;
  p_bg   = COLOR_WHITE;

  return;
}

void MI0283QT2::transfer(uint16_t delay)
{
        int ret;
          //printf("Writing %d \n", txc);

          struct spi_ioc_transfer tr;
	  tr.tx_buf = (unsigned long)tx;
          tr.rx_buf = 0;
          tr.len = txc;
          tr.delay_usecs = 0;
          tr.speed_hz = 0;
          tr.bits_per_word = 0;
          ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
          if (ret == 1)
                  pabort("can't send spi message");
}


void MI0283QT2::wr_spi( uint8_t data )
{
 tx[txc] = data;
 txc++;
}

void MI0283QT2::CS_ENABLE()
{
  txc = 0;
}

void MI0283QT2::CS_DISABLE()
{
  transfer( 0 );
}


void MI0283QT2::led(uint8_t power)
{
  printf( "led not implemented \n");
  return;
}


void MI0283QT2::setOrientation(uint16_t o)
{
  switch(o)
  {
    case 0:
      lcd_orientation = 0;
      lcd_width  = 320;
      lcd_height = 240;
      wr_cmd(0x16, 0x00A8); //MY=1 MX=0 MV=1 ML=0 BGR=1
      break;

    case 90:
      lcd_orientation = 90;
      lcd_width  = 240;
      lcd_height = 320;
      wr_cmd(0x16, 0x0008); //MY=0 MX=0 MV=0 ML=0 BGR=1
      break;

    case 180:
      lcd_orientation = 180;
      lcd_width  = 320;
      lcd_height = 240;
      wr_cmd(0x16, 0x0068); //MY=0 MX=1 MV=1 ML=0 BGR=1
      break;

    case 270:
      lcd_orientation = 270;
      lcd_width  = 240;
      lcd_height = 320;
      wr_cmd(0x16, 0x00C8); //MY=1 MX=0 MV=1 ML=0 BGR=1
      break;
  }
  
  setArea(0, 0, lcd_width-1, lcd_height-1);

  p_x = PRINT_STARTX;
  p_y = PRINT_STARTY;

  return;
}


uint16_t MI0283QT2::getWidth(void)
{
  return lcd_width;
}


uint16_t MI0283QT2::getHeight(void)
{
  return lcd_height;
}


void MI0283QT2::setArea(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  if((x1 >= lcd_width) ||
     (y1 >= lcd_height))
  {
    return;
  }

  wr_cmd(0x03, (x0>>0)); //set x0
  wr_cmd(0x02, (x0>>8)); //set x0
  wr_cmd(0x05, (x1>>0)); //set x1
  wr_cmd(0x04, (x1>>8)); //set x1
  wr_cmd(0x07, (y0>>0)); //set y0
  wr_cmd(0x06, (y0>>8)); //set y0
  wr_cmd(0x09, (y1>>0)); //set y1
  wr_cmd(0x08, (y1>>8)); //set y1

  return;
}


void MI0283QT2::setCursor(uint16_t x, uint16_t y)
{
  setArea(x, y, x, y);

  return;
}


void MI0283QT2::clear(uint16_t color)
{
  uint16_t size;

  setArea(0, 0, lcd_width-1, lcd_height-1);

  drawStart();
  for(size=(320UL*240UL/8UL); size!=0; size--)
  {
    draw(color); //1
    draw(color); //2
    draw(color); //3
    draw(color); //4
    draw(color); //5
    draw(color); //6
    draw(color); //7
    draw(color); //8
  }
  drawStop();

  return;
}


void MI0283QT2::drawStart(void)
{
  CS_ENABLE();
  wr_spi(LCD_REGISTER);
  wr_spi(0x22);
  CS_DISABLE();

  CS_ENABLE();
  wr_spi(LCD_DATA);

  return;
}


void MI0283QT2::draw(uint16_t color)
{
  wr_spi(color>>8);
  wr_spi(color);

  return;
}


void MI0283QT2::drawStop(void)
{
  CS_DISABLE();

  return;
}


void MI0283QT2::drawPixel(uint16_t x0, uint16_t y0, uint16_t color)
{
  if((x0 >= lcd_width) ||
     (y0 >= lcd_height))
  {
    return;
  }

  setCursor(x0, y0);

  drawStart();
  draw(color);
  drawStop();

  return;
}


void MI0283QT2::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	int16_t dx, dy, dx2, dy2, err, stepx, stepy;

  if(x0 >= lcd_width)
  {
    x0 = lcd_width-1;
  }
  if(x1 >= lcd_width)
  {
    x1 = lcd_width-1;
  }
  if(y0 >= lcd_height)
  {
    y0 = lcd_height-1;
  }
  if(y1 >= lcd_height)
  {
    y1 = lcd_height-1;
  }

  if((x0 == x1) ||
     (y0 == y1)) //horizontal or vertical line
  {
    fillRect(x0, y0, x1, y1, color);
  }
  else
  {
    //calculate direction
    dx = x1 - x0;
    dy = y1 - y0;
    if(dx < 0) { dx = -dx; stepx = -1; } else { stepx = +1; }
    if(dy < 0) { dy = -dy; stepy = -1; } else { stepy = +1; }
    dx2 = dx << 1;
    dy2 = dy << 1;
    //draw line
    setArea(0, 0, lcd_width-1, lcd_height-1);
    drawPixel(x0, y0, color);
    if(dx > dy)
    {
      err = dy2 - dx;
      while(x0 != x1)
      {
        if(err >= 0)
        {
          y0  += stepy;
          err -= dx2;
        }
        x0  += stepx;
        err += dy2;
        drawPixel(x0, y0, color);
      }
    }
    else
    {
      err = dx2 - dy;
      while(y0 != y1)
      {
        if(err >= 0)
        {
          x0  += stepx;
          err -= dy2;
        }
        y0  += stepy;
        err += dx2;
        drawPixel(x0, y0, color);
      }
    }
  }

  return;
}


void MI0283QT2::drawRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
  fillRect(x0, y0, x0, y1, color);
  fillRect(x0, y1, x1, y1, color);
  fillRect(x1, y0, x1, y1, color);
  fillRect(x0, y0, x1, y0, color);

  return;
}


void MI0283QT2::fillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
  uint32_t size;
  uint16_t tmp, i;

  if(x0 > x1)
  {
    tmp = x0;
    x0  = x1;
    x1  = tmp;
  }
  if(y0 > y1)
  {
    tmp = y0;
    y0  = y1;
    y1  = tmp;
  }

  if(x1 >= lcd_width)
  {
    x1 = lcd_width-1;
  }
  if(y1 >= lcd_height)
  {
    y1 = lcd_height-1;
  }

  setArea(x0, y0, x1, y1);

  drawStart();
  size = (uint32_t)(1+(x1-x0)) * (uint32_t)(1+(y1-y0));
  tmp  = size/8;
  if(tmp != 0)
  {
    for(i=tmp; i!=0; i--)
    {
      draw(color); //1
      draw(color); //2
      draw(color); //3
      draw(color); //4
      draw(color); //5
      draw(color); //6
      draw(color); //7
      draw(color); //8
    }
    for(i=size%8; i!=0; i--)
    {
      draw(color);
    }
  }
  else
  {
    for(i=size; i!=0; i--)
    {
      draw(color);
    }
  }
  drawStop();

  return;
}


void MI0283QT2::drawCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color)
{
  int16_t err, x, y;
  
  err = -radius;
  x   = radius;
  y   = 0;

  setArea(0, 0, lcd_width-1, lcd_height-1);

  while(x >= y)
  {
    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);

    err += y;
    y++;
    err += y;
    if(err >= 0)
    {
      x--;
      err -= x;
      err -= x;
    }
  }

  return;
}


void MI0283QT2::fillCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color)
{
  int16_t err, x, y;
  
  err = -radius;
  x   = radius;
  y   = 0;

  setArea(0, 0, lcd_width-1, lcd_height-1);

  while(x >= y)
  {
    drawLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
    drawLine(x0 - x, y0 - y, x0 + x, y0 - y, color);
    drawLine(x0 - y, y0 + x, x0 + y, y0 + x, color);
    drawLine(x0 - y, y0 - x, x0 + y, y0 - x, color);

    err += y;
    y++;
    err += y;
    if(err >= 0)
    {
      x--;
      err -= x;
      err -= x;
    }
  }

  return;
}


uint16_t MI0283QT2::drawChar(uint16_t x, uint16_t y, char c, uint8_t size, uint16_t color, uint16_t bg_color)
{
  uint16_t ret;
#if FONT_WIDTH <= 8
  uint8_t data, mask;
#elif FONT_WIDTH <= 16
  uint16_t data, mask;
#elif FONT_WIDTH <= 32
  uint32_t data, mask;
#endif
  uint8_t i, j, width, height;
  const uint8_t *ptr;

  i      = (uint8_t)c;
#if FONT_WIDTH <= 8
  ptr    = &font_PGM[(i-FONT_START)*(8*FONT_HEIGHT/8)];
#elif FONT_WIDTH <= 16
  ptr    = &font_PGM[(i-FONT_START)*(16*FONT_HEIGHT/8)];
#elif FONT_WIDTH <= 32
  ptr    = &font_PGM[(i-FONT_START)*(32*FONT_HEIGHT/8)];
#endif
  width  = FONT_WIDTH;
  height = FONT_HEIGHT;

  if(size <= 1)
  {
    ret = x+width;
    if((ret-1) >= lcd_width)
    {
      return lcd_width+1;
    }
    else if((y+height-1) >= lcd_height)
    {
      return lcd_width+1;
    }

    setArea(x, y, (x+width-1), (y+height-1));

    drawStart();
    for(; height!=0; height--)
    {
#if FONT_WIDTH <= 8
      data = read_byte(ptr); ptr+=1;
#elif FONT_WIDTH <= 16
      data = read_word(ptr); ptr+=2;
#elif FONT_WIDTH <= 32
      data = read_dword(ptr); ptr+=4;
#endif
      for(mask=(1<<(width-1)); mask!=0; mask>>=1)
      {
        if(data & mask)
        {
          draw(color);
        }
        else
        {
          draw(bg_color);
        }
      }
    }
    drawStop();
  }
  else
  {
    ret = x+(width*size);
    if((ret-1) >= lcd_width)
    {
      return lcd_width+1;
    }
    else if((y+(height*size)-1) >= lcd_height)
    {
      return lcd_width+1;
    }
    
    setArea(x, y, (x+(width*size)-1), (y+(height*size)-1));

    drawStart();
    for(; height!=0; height--)
    {
#if FONT_WIDTH <= 8
      data = read_byte(ptr); ptr+=1;
#elif FONT_WIDTH <= 16
      data = read_word(ptr); ptr+=2;
#elif FONT_WIDTH <= 32
      data = read_dword(ptr); ptr+=4;
#endif
      for(i=size; i!=0; i--)
      {
        for(mask=(1<<(width-1)); mask!=0; mask>>=1)
        {
          if(data & mask)
          {
            for(j=size; j!=0; j--)
            {
              draw(color);
            }
          }
          else
          {
            for(j=size; j!=0; j--)
            {
              draw(bg_color);
            }
          }
        }
      }
    }
    drawStop();
  }

  return ret;
}


uint16_t MI0283QT2::drawText(uint16_t x, uint16_t y, char *s, uint8_t size, uint16_t color, uint16_t bg_color)
{
  while(*s != 0)
  {
    x = drawChar(x, y, (char)*s++, size, color, bg_color);
    if(x > lcd_width)
    {
      break;
    }
  }

  return x;
}

uint16_t MI0283QT2::drawText(uint16_t x, uint16_t y, int i, uint8_t size, uint16_t color, uint16_t bg_color)
{
  char tmp[16];
  sprintf( tmp,  "%i", i );
  return drawText(x, y, tmp, size, color, bg_color);
}


uint16_t MI0283QT2::drawText(uint16_t x, uint16_t y, unsigned int i, uint8_t size, uint16_t color, uint16_t bg_color)
{
  char tmp[16];
  sprintf( tmp, "%u", i );

  return drawText(x, y, tmp, size, color, bg_color);
}


uint16_t MI0283QT2::drawText(uint16_t x, uint16_t y, long l, uint8_t size, uint16_t color, uint16_t bg_color)
{
  char tmp[32];
  sprintf( tmp, "%il", l );
  return drawText(x, y, tmp, size, color, bg_color);
}


uint16_t MI0283QT2::drawText(uint16_t x, uint16_t y, unsigned long l, uint8_t size, uint16_t color, uint16_t bg_color)
{
  char tmp[32];

  sprintf( tmp, "%ul", l );

  return drawText(x, y, tmp, size, color, bg_color);
}


uint16_t MI0283QT2::drawText(uint16_t x, uint16_t y, const std::string &s, uint8_t size, uint16_t color, uint16_t bg_color)
{
  uint16_t i;

  for(i=0; i < s.length(); i++) 
  {
    x = drawChar(x, y, (char)s[i], size, color, bg_color);
    if(x > lcd_width)
    {
      break;
    }
  }

  return x;
}


uint16_t MI0283QT2::drawMLText(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, char *s, uint8_t size, uint16_t color, uint16_t bg_color)
{
  uint16_t x=x0, y=y0, wlen, llen;
  char c;
  char *wstart;

  fillRect(x0, y0, x1, y1, bg_color);

  llen   = (x1-x0)/(FONT_WIDTH*size); //line len in chars
  wstart = s;
  while(*s && (y < y1))
  {
    c = *s++;
    if(c == '\n') //new line
    {
      x  = x0;
      y += (FONT_HEIGHT*size)+1;
      continue;
    }
    else if(c == '\r') //skip
    {
      continue;
    }

    if(c == ' ') //start of a new word
    {
      wstart = s;
      if(x == x0) //do nothing
      {
        continue;
      }
    }

    if(c)
    {
      if((x+(FONT_WIDTH*size)) > x1) //new line
      {
        if(c == ' ') //do not start with space
        {
          x  = x0;
          y += (FONT_HEIGHT*size)+1;
        }
        else
        {
          wlen = (s-wstart);
          if(wlen > llen) //word too long
          {
            x  = x0;
            y += (FONT_HEIGHT*size)+1;
            if(y < y1)
            {
              x = drawChar(x, y, c, size, color, bg_color);
            }
          }
          else
          {
            fillRect(x-(wlen*FONT_WIDTH*size), y, x1, (y+(FONT_HEIGHT*size)), bg_color); //clear word
            x  = x0;
            y += (FONT_HEIGHT*size)+1;
            s  = wstart;
          }
        }
      }
      else
      {
        x = drawChar(x, y, c, size, color, bg_color);
      }
    }
  }

  return x;
}


uint16_t MI0283QT2::drawMLText(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const std::string &s, uint8_t size, uint16_t color, uint16_t bg_color)
{
  uint16_t x=x0, y=y0, wstart, wlen, llen, p;
  char c;

  fillRect(x0, y0, x1, y1, bg_color);

  llen   = (x1-x0)/(FONT_WIDTH*size); //line len in chars
  wstart = 0;
  for(p=0; (p < s.length()) && (y < y1); p++) 
  {
    c = (char)s[p];
    if(c == '\n') //new line
    {
      x  = x0;
      y += (FONT_HEIGHT*size)+1;
      continue;
    }
    else if(c == '\r') //skip
    {
      continue;
    }

    if(c == ' ') //start of a new word
    {
      wstart = p;
      if(x == x0) //do nothing
      {
        continue;
      }
    }

    if(c)
    {
      if((x+(FONT_WIDTH*size)) > x1) //new line
      {
        if(c == ' ') //do not start with space
        {
          x  = x0;
          y += (FONT_HEIGHT*size)+1;
        }
        else
        {
          wlen = (p-wstart);
          if(wlen > llen) //word too long
          {
            x  = x0;
            y += (FONT_HEIGHT*size)+1;
            if(y < y1)
            {
              x = drawChar(x, y, c, size, color, bg_color);
            }
          }
          else
          {
            fillRect(x-(wlen*FONT_WIDTH*size), y, x1, (y+(FONT_HEIGHT*size)), bg_color); //clear word
            x  = x0;
            y += (FONT_HEIGHT*size)+1;
            p  = wstart;
          }
        }
      }
      else
      {
        x = drawChar(x, y, c, size, color, bg_color);
      }
    }
  }

  return x;
}


uint16_t MI0283QT2::drawInteger(uint16_t x, uint16_t y, char val, uint8_t base, uint8_t size, uint16_t color, uint16_t bg_color)
{
  char tmp[16+1];

  sprintf( tmp, "%i",(int)val );
  return drawText(x, y, tmp, size, color, bg_color);
}


uint16_t MI0283QT2::drawInteger(uint16_t x, uint16_t y, unsigned char val, uint8_t base, uint8_t size, uint16_t color, uint16_t bg_color)
{
  char tmp[16+1];

  sprintf( tmp, "%i", (uint)val );

  return drawText(x, y, tmp, size, color, bg_color);
}


uint16_t MI0283QT2::drawInteger(uint16_t x, uint16_t y, int val, uint8_t base, uint8_t size, uint16_t color, uint16_t bg_color)
{
  char tmp[16+1];
  switch ( base ) {
    case 8:
      sprintf( tmp, "%o", (uint)val );
    break;
    case 10:
      sprintf( tmp, "%i", (uint)val );
    break;
    case 16:
      sprintf( tmp, "%x", (uint)val );
    break;
  }

  return drawText(x, y, tmp, size, color, bg_color);
}


uint16_t MI0283QT2::drawInteger(uint16_t x, uint16_t y, long val, uint8_t base, uint8_t size, uint16_t color, uint16_t bg_color)
{
  char tmp[32+1];
  switch ( base ) {
    case 8:
      sprintf( tmp, "%ol", (uint)val );
    break;
    case 10:
      sprintf( tmp, "%il", (uint)val );
    break;
    case 16:
      sprintf( tmp, "%xl", (uint)val );
    break;
  }

  return drawText(x, y, tmp, size, color, bg_color);
}


void MI0283QT2::printOptions(uint8_t size, uint16_t color, uint16_t bg_color)
{
  p_size = size;
  p_fg   = color;
  p_bg   = bg_color;

  return;
}


void MI0283QT2::printClear(void)
{
  clear(p_bg);

  p_x = PRINT_STARTX;
  p_y = PRINT_STARTY;

  return;
}


void MI0283QT2::printXY(uint16_t x, uint16_t y)
{
  p_x = x;
  p_y = y;

  return;
}


uint16_t MI0283QT2::printGetX(void)
{
  return p_x;
}


uint16_t MI0283QT2::printGetY(void)
{
  return p_y;
}


size_t MI0283QT2::write(uint8_t c)
{
  uint16_t x=p_x, y=p_y;

  if(c == '\n')
  {
    x  = PRINT_STARTX;
    y += (FONT_HEIGHT*p_size)+1;
    if(y >= lcd_height)
    {
      y = PRINT_STARTY;
    }
  }
  else if (c == '\r') //skip
  {
    //do nothing
  }
  else
  {
    x = drawChar(x, y, c, p_size, p_fg, p_bg);
    if(x > lcd_width)
    {
      fillRect(p_x, y, lcd_width-1, y+(FONT_HEIGHT*p_size), p_bg);
      x  = PRINT_STARTX;
      y += (FONT_HEIGHT*p_size)+1;
      if(y >= lcd_height)
      {
        y = PRINT_STARTY;
      }
      x = drawChar(x, y, c, p_size, p_fg, p_bg);
    }
  }

  p_x = x;
  p_y = y;

  return 1;
}


size_t MI0283QT2::write(const char *s)
{
  size_t len=0;

  while(*s)
  {
    write((uint8_t)*s++);
    len++;
  }

  return len;
}


size_t MI0283QT2::write(const uint8_t *s, size_t size)
{
  size_t len=0;

  while(size != 0)
  {
    write((uint8_t)*s++);
    size--;
    len++;
  }

  return len;
}


//-------------------- Private --------------------


void MI0283QT2::reset(void)
{
  //reset
  CS_DISABLE();
  RST_ENABLE();
  delay_10ms(5);
  RST_DISABLE();
  delay_10ms(5);

  //driving ability
  wr_cmd(0xEA, 0x0000);
  wr_cmd(0xEB, 0x0020);
  wr_cmd(0xEC, 0x000C);
  wr_cmd(0xED, 0x00C4);
  wr_cmd(0xE8, 0x0040);
  wr_cmd(0xE9, 0x0038);
  wr_cmd(0xF1, 0x0001);
  wr_cmd(0xF2, 0x0010);
  wr_cmd(0x27, 0x00A3);

  //power voltage
  wr_cmd(0x1B, 0x001B);
  wr_cmd(0x1A, 0x0001);
  wr_cmd(0x24, 0x002F);
  wr_cmd(0x25, 0x0057);

  //VCOM offset
  wr_cmd(0x23, 0x008D); //for flicker adjust

  //power on
  wr_cmd(0x18, 0x0036);
  wr_cmd(0x19, 0x0001); //start osc
  wr_cmd(0x01, 0x0000); //wakeup
  wr_cmd(0x1F, 0x0088);
  _delay_ms(5);
  wr_cmd(0x1F, 0x0080);
  _delay_ms(5);
  wr_cmd(0x1F, 0x0090);
  _delay_ms(5);
  wr_cmd(0x1F, 0x00D0);
  _delay_ms(5);

  //color selection
  wr_cmd(0x17, 0x0005); //0x0005=65k, 0x0006=262k

  //panel characteristic
  wr_cmd(0x36, 0x0000);

  //display on
  wr_cmd(0x28, 0x0038);
  delay_10ms(4);
  wr_cmd(0x28, 0x003C);

  //display options
  setOrientation(0);

  return;
}


void MI0283QT2::wr_cmd(uint8_t reg, uint8_t param)
{
  CS_ENABLE();
  wr_spi(LCD_REGISTER);
  wr_spi(reg);
  CS_DISABLE();

  CS_ENABLE();
  wr_spi(LCD_DATA);
  wr_spi(param);
  CS_DISABLE();

  return;
}


void MI0283QT2::wr_data(uint16_t data)
{
  CS_ENABLE();
  wr_spi(LCD_DATA);
  wr_spi(data>>8);
  wr_spi(data);
  CS_DISABLE();

  return;
}


