#include <stdint.h>

#ifndef __SSD1306_FONTS_H__
#define __SSD1306_FONTS_H__

typedef struct {
	const uint8_t FontWidth;    /*!< Font width in pixels */
	uint8_t FontHeight;   /*!< Font height in pixels */
	const uint16_t *data; /*!< Pointer to data font data array */
	const uint8_t *data8bit;
} FontDef;



extern FontDef Font_8x9;
extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;

#define FONT8X9_SPS '~'+1
#define FONT8X9_S '~'+2
#define FONT8X9_mS '~'+3
#define FONT8X9_uS '~'+4
#define FONT8X9_DIV '~'+5
#define FONT8X9_DAHAI '~'+6
#define FONT8X9_RUNNINGMAN '~'+7
#define FONT8X9_STOPMAN '~'+8
#define FONT8X9_RUN '~'+9
#define FONT8X9_STOP '~'+10
#define FONT8X9_0p1 '~'+11
#define FONT8X9_0p2 '~'+12
#define FONT8X9_0p5 '~'+13
#define FONT8X9_1 '~'+14
#define FONT8X9_2 '~'+15
#define FONT8X9_5 '~'+16
#define FONT8X9_10 '~'+17
#define FONT8X9_20 '~'+18
#define FONT8X9_50 '~'+19
#define FONT8X9_100 '~'+20
#define FONT8X9_200 '~'+21
#define FONT8X9_500 '~'+22
#define FONT8X9_0p5V '~'+23
#define FONT8X9_1V '~'+24
#define FONT8X9_2V '~'+25
#define FONT8X9_5V '~'+26

#endif // __SSD1306_FONTS_H__
