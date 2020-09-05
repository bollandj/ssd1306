
#ifndef GRAPHICSFONT_H_
#define GRAPHICSFONT_H_

#include <avr/pgmspace.h>

#define CHAR_WIDTH 6 // including character spacing i.e. actual char width + 1

/* ASCII characters 0-127 */
extern const uint8_t font[128*5] PROGMEM;

#endif /* GRAPHICSFONT_H_ */