/*
  BMPDemo (Display + SD-Card)
 */
#include <stdint.h>
#include <stdio.h>

#include "MI0283QT2.h"
#include "BMPheader.h"

MI0283QT2 lcd("/dev/spidev0.1");

uint32_t swap(uint32_t i) {
    char *c = (char *) &i;
    return * (uint32_t *) (char[]) {c[2], c[3], c[0], c[1] };
}


void OpenBMPFile(const char *file, int16_t x, int16_t y)
{
  uint8_t buf[40]; //read buf (min. size = sizeof(BMP_DIPHeader))
  BMP_Header *bmp_hd;
  BMP_DIPHeader *bmp_dip;
  int16_t width, height, w, h;
  FILE * fd;
  fd = fopen(file, "rb");
  if(fd == NULL)
  {
    lcd.drawText(x, y,"File not found!", 1, RGB(0,0,0), RGB(255,255,255));
    return;
  }

  //BMP Header
  bmp_hd = (BMP_Header*)&buf[0];
  int rd = fread(&buf, sizeof(BMP_Header), 1, fd);
  uint32_t offset = bmp_hd->offset;
  if(
     (bmp_hd->magic[0] == 'B') && (bmp_hd->magic[1] == 'M') && (offset == 54))
  {
    //BMP DIP-Header
    bmp_dip = (BMP_DIPHeader*)&buf[0];
    fread(&buf, sizeof(BMP_DIPHeader), 1, fd);
    if (
       (bmp_dip->size == sizeof(BMP_DIPHeader)) && (bmp_dip->bitspp == 24) && (bmp_dip->compress == 0))
    {
      //BMP data (1. pixel = bottom left)
      width  = bmp_dip->width;
      height = bmp_dip->height;
      int pad    = width % 4; //padding (line is multiply of 4)

      if((x+width) <= lcd.getWidth() && (y+height) <= lcd.getHeight())
      {
        lcd.setArea(x, y, x+width-1, y+height-1);
        for(h=(y+height-1); h >= y; h--) //for every line
        {
          for(w=x; w < (x+width); w++) //for every pixel in line
          {
            fread( &buf, 3, 1, fd);
            lcd.drawPixel(w, h, RGB(buf[2],buf[1],buf[0]));
          }
          if(pad)
          {
            fread(&buf, pad, 1, fd);
          }
        }
      }
      else
      {
        lcd.drawText(x, y, "Pic out of screen!", 1, RGB(0,0,0), RGB(255,255,255));
      }
    }
  } else 
        {
        lcd.drawText(x, y, "IS not a bmp file!", 1, RGB(0,0,0), RGB(255,255,255));
      }


  fclose(fd);
}

/*
void OpenBMPFile_opt(char *file, int16_t x, int16_t y)
{
  FIL fsrc;        //file objects
  FRESULT res;     //result code
  UINT rd;         //file R/W count
  uint8_t pad;
  uint8_t buf[MAX_BMP_WIDTH*2], pix[3]; //read buf (min. size = sizeof(BMP_DIPHeader))
  uint16_t *ptr;
  BMP_Header *bmp_hd;
  BMP_DIPHeader *bmp_dip;
  int16_t width, height, w, h;

  res = f_open(&fsrc, file, FA_OPEN_EXISTING | FA_READ);
  if(res != FR_OK)
  {
    lcd.drawTextPGM(x, y, PSTR("File not found!"), 1, RGB(0,0,0), RGB(255,255,255));
    return;
  }

  //BMP Header
  bmp_hd = (BMP_Header*)&buf[0];
  res = f_read(&fsrc, &buf, sizeof(BMP_Header), &rd);
  if((res == FR_OK) &&
     (bmp_hd->magic[0] == 'B') && (bmp_hd->magic[1] == 'M') && (bmp_hd->offset == 54))
  {
    //BMP DIP-Header
    bmp_dip = (BMP_DIPHeader*)&buf[0];
    res = f_read(&fsrc, &buf, sizeof(BMP_DIPHeader), &rd);
    if((res == FR_OK) &&
       (bmp_dip->size == sizeof(BMP_DIPHeader)) && (bmp_dip->bitspp == 24) && (bmp_dip->compress == 0))
    {
      //BMP data (1. pixel = bottom left)
      width  = bmp_dip->width;
      height = bmp_dip->height;
      pad    = width % 4; //padding (line is multiply of 4)

      if((x+width) <= lcd.getWidth() && (y+height) <= lcd.getHeight())
      {
        for(h=(y+height-1); h >= y; h--) //for every line
        {
          //read complete line
          ptr = (uint16_t*)&buf[0];
          for(w=width; w != 0; w--) //for every pixel in line
          {
            f_read(&fsrc, &pix, 3, &rd);
            *ptr++ = RGB(pix[2],pix[1],pix[0]);
          }
          if(pad)
          {
            f_read(&fsrc, &pix, pad, &rd);
          }

          //draw complete line
          ptr = (uint16_t*)&buf[0];
          lcd.setArea(x, h, x+width-1, h);
          lcd.drawStart();
          for(w=width; w != 0; w--) //for every pixel in line
          {
            lcd.draw(*ptr++);
          }
          lcd.drawStop();
        }
      }
      else
      {
        lcd.drawTextPGM(x, y, PSTR("Pic out of screen!"), 1, RGB(0,0,0), RGB(255,255,255));
      }
    }
  }

  f_close(&fsrc);
}

*/


int main()
{
  //init Display
  lcd.init(); //spi-clk = Fcpu/4

  //clear screen
  lcd.clear(RGB(255,255,255));

  lcd.clear(RGB(0,255,0));
  lcd.drawText(5, 5,"Load file...", 1, RGB(255,255,255), RGB(0,255,0));

  //windows bmp file (24bit RGB):  examples/mSDShield/BMPDemo/image.bmp
  OpenBMPFile("image.bmp", 20, 20);
 // OpenBMPFile_opt("image.bmp", 160, 20); //optimized version
  return 0;
}


