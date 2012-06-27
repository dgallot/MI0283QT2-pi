#ifndef MI0283QT2_h
#define MI0283QT2_h


#ifdef __cplusplus
extern "C" {
#endif
  #include "fonts.h"
  #include <stdint.h>
  
#ifdef __cplusplus
}
#endif

#include <string>

#define RGB(r,g,b)   (((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3)) //5 red | 6 green | 5 blue

#define COLOR_WHITE  RGB(255,255,255)
#define COLOR_BLACK  RGB(  0,  0,  0)
#define COLOR_RED    RGB(255,  0,  0)
#define COLOR_GREEN  RGB(  0,255,  0)
#define COLOR_BLUE   RGB(  0,  0,255)
#define COLOR_YELLOW RGB(255,255,  0)

#ifndef DEC
# define DEC (10)
#endif
#ifndef HEX
# define HEX (16)
#endif
#ifndef OCT
# define OCT (8)
#endif
#ifndef BIN
# define BIN (2)
#endif


class MI0283QT2
{
  public:
    uint16_t lcd_orientation;
    uint16_t lcd_width, lcd_height;

    MI0283QT2(const std::string &dev);
    void init();
    void led(uint8_t power); //0-100

    void setOrientation(uint16_t o); //0 90 180 270
    uint16_t getWidth(void);
    uint16_t getHeight(void);
    void setArea(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void setCursor(uint16_t x, uint16_t y);

    void clear(uint16_t color);
    void drawStart(void);
    void draw(uint16_t color);
    void drawStop(void);
    void drawPixel(uint16_t x0, uint16_t y0, uint16_t color);
    void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
    void drawRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
    void fillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
    void drawCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color);
    void fillCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color);

    uint16_t drawChar(uint16_t x, uint16_t y, char c, uint8_t size, uint16_t color, uint16_t bg_color);
    uint16_t drawText(uint16_t x, uint16_t y, const char *s, uint8_t size, uint16_t color, uint16_t bg_color);
    uint16_t drawText(uint16_t x, uint16_t y, int i, uint8_t size, uint16_t color, uint16_t bg_color);
    uint16_t drawText(uint16_t x, uint16_t y, unsigned int i, uint8_t size, uint16_t color, uint16_t bg_color);
    uint16_t drawText(uint16_t x, uint16_t y, long l, uint8_t size, uint16_t color, uint16_t bg_color);
    uint16_t drawText(uint16_t x, uint16_t y, unsigned long l, uint8_t size, uint16_t color, uint16_t bg_color);
    uint16_t drawText(uint16_t x, uint16_t y, const std::string &s, uint8_t size, uint16_t color, uint16_t bg_color);

    uint16_t drawMLText(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const char *s, uint8_t size, uint16_t color, uint16_t bg_color);
    uint16_t drawMLText(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const std::string &s, uint8_t size, uint16_t color, uint16_t bg_color);

    uint16_t drawInteger(uint16_t x, uint16_t y, char val, uint8_t base, uint8_t size, uint16_t color, uint16_t bg_color); //base = DEC, HEX, OCT, BIN
    uint16_t drawInteger(uint16_t x, uint16_t y, unsigned char val, uint8_t base, uint8_t size, uint16_t color, uint16_t bg_color);
    uint16_t drawInteger(uint16_t x, uint16_t y, int val, uint8_t base, uint8_t size, uint16_t color, uint16_t bg_color);
    uint16_t drawInteger(uint16_t x, uint16_t y, long val, uint8_t base, uint8_t size, uint16_t color, uint16_t bg_color);

    void printOptions(uint8_t size, uint16_t color, uint16_t bg_color);
    void printClear(void);
    void printXY(uint16_t x, uint16_t y);
    uint16_t printGetX(void);
    uint16_t printGetY(void);
    virtual size_t write(uint8_t c);
    virtual size_t write(const char *s);
    virtual size_t write(const uint8_t *s, size_t size);

  private:
    uint8_t p_size;
    uint16_t p_fg, p_bg;
    uint16_t p_x, p_y;
    int32_t txc; 
    uint8_t tx[1024*1024];
    int fd;
    const std::string device;
    uint8_t mode;
    uint8_t bits;
    uint32_t speed;
    uint16_t delay;


    void reset(void);
    void wr_cmd(uint8_t reg, uint8_t param);
    void wr_data(uint16_t data);
    void wr_spi(uint8_t data);
    void transfer(uint16_t delay);
    void CS_ENABLE();
    void CS_DISABLE();
 
};


#endif //MI0283QT2_h
