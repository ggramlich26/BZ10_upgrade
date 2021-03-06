
/*******************************************************************************
* generated by lcd-image-converter rev.90f620c from 2017-10-04 21:09:44 +0500
* image
* filename: unsaved
* name: timer_small
*
* preset name: Monochrome
* data block size: 8 bit(s), uint8_t
* RLE compression enabled: no
* conversion type: Monochrome, Diffuse Dither 128
* split to rows: yes
* bits per pixel: 1
*
* preprocess:
*  main scan direction: top_to_bottom
*  line scan direction: forward
*  inverse: no
*******************************************************************************/

/*
 typedef struct {
     const uint8_t *data;
     uint16_t width;
     uint16_t height;
     uint8_t dataSize;
     } tImage;
*/
#include <stdint.h>


#ifndef _TIMER_SMALL_
#define _TIMER_SMALL_

static const uint8_t image_data_timer_small[215] = {
    // ∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙
    // ∙∙∙∙∙∙∙∙∙∙∙∙∙∙█████∙∙∙∙∙∙∙∙∙∙∙∙∙∙
    // ∙∙∙∙∙∙∙∙∙∙∙∙∙███████∙∙∙∙∙∙∙∙∙∙∙∙∙
    // ∙∙∙∙∙∙∙∙∙∙∙∙██∙∙∙∙∙██∙∙∙∙∙∙∙∙∙∙∙∙
    // ∙∙∙∙∙∙∙∙∙∙∙∙█∙∙∙∙∙∙∙█∙∙∙∙∙∙∙∙∙∙∙∙
    // ∙∙∙∙∙∙∙∙∙∙∙██∙∙∙∙∙∙∙██∙∙∙∙∙∙∙∙∙∙∙
    // ∙∙∙∙∙∙∙∙∙∙∙██∙█████∙██∙∙∙∙∙∙∙∙∙∙∙
    // ∙∙∙∙∙∙∙∙∙∙∙∙█∙█████∙█∙∙∙∙∙∙∙∙∙∙∙∙
    // ∙∙∙∙∙∙∙∙∙∙∙∙██∙███∙██∙∙∙∙∙∙∙∙∙∙∙∙
    // ∙∙∙∙█∙∙∙∙∙∙∙∙███████∙∙∙∙∙∙∙██∙∙∙∙
    // ∙∙∙███∙∙∙∙∙∙∙██████∙∙∙∙∙∙∙████∙∙∙
    // ∙∙█████∙∙∙∙∙∙∙∙███∙∙∙∙∙∙∙██████∙∙
    // ∙∙██████∙∙∙███████████∙∙∙█████∙∙∙
    // ∙∙∙█████∙███████████████∙█████∙∙∙
    // ∙∙∙∙████████∙∙∙∙∙∙∙∙∙████████∙∙∙∙
    // ∙∙∙∙∙∙████∙∙∙∙∙∙█∙∙∙∙∙█████∙∙∙∙∙∙
    // ∙∙∙∙∙████∙█∙∙∙∙∙█∙∙∙∙∙█∙████∙∙∙∙∙
    // ∙∙∙∙████∙∙∙█∙∙∙∙█∙∙∙∙█∙∙∙████∙∙∙∙
    // ∙∙∙████∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙███∙∙∙∙
    // ∙∙∙███∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙███∙∙∙
    // ∙∙∙██∙█∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙█∙∙∙████∙∙∙
    // ∙∙██∙∙∙█∙∙∙∙∙∙∙∙∙∙∙∙∙██∙∙█∙∙███∙∙
    // ∙∙██∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙██∙∙∙∙∙∙███∙∙
    // ∙∙██∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙██∙∙∙∙∙∙∙███∙∙
    // ∙███∙∙∙∙∙∙∙∙∙∙∙∙∙███∙∙∙∙∙∙∙∙∙██∙∙
    // ∙██∙∙∙∙∙∙∙∙∙∙∙∙∙███∙∙∙∙∙∙∙∙∙∙███∙
    // ∙██∙∙███∙∙∙∙∙∙∙███∙∙∙∙∙∙∙███∙███∙
    // ∙██∙∙∙∙∙∙∙∙∙∙∙∙███∙∙∙∙∙∙∙∙∙∙∙███∙
    // ∙███∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙███∙
    // ∙███∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙██∙∙
    // ∙∙██∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙███∙∙
    // ∙∙██∙∙∙█∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙█∙∙███∙∙
    // ∙∙███∙█∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙█∙██∙∙∙
    // ∙∙∙███∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙███∙∙∙
    // ∙∙∙███∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙███∙∙∙∙
    // ∙∙∙∙███∙∙∙∙█∙∙∙∙█∙∙∙∙█∙∙∙████∙∙∙∙
    // ∙∙∙∙∙███∙∙█∙∙∙∙∙█∙∙∙∙∙█∙████∙∙∙∙∙
    // ∙∙∙∙∙∙████∙∙∙∙∙∙█∙∙∙∙∙∙████∙∙∙∙∙∙
    // ∙∙∙∙∙∙∙████∙∙∙∙∙∙∙∙∙∙∙████∙∙∙∙∙∙∙
    // ∙∙∙∙∙∙∙∙██████∙∙∙∙∙██████∙∙∙∙∙∙∙∙
    // ∙∙∙∙∙∙∙∙∙∙█████████████∙∙∙∙∙∙∙∙∙∙
    // ∙∙∙∙∙∙∙∙∙∙∙∙∙████████∙∙∙∙∙∙∙∙∙∙∙∙
    // ∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙∙
    0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x03, 0xe0, 0x00, 0x00, 
    0x00, 0x07, 0xf0, 0x00, 0x00, 
    0x00, 0x0c, 0x18, 0x00, 0x00, 
    0x00, 0x08, 0x08, 0x00, 0x00, 
    0x00, 0x18, 0x0c, 0x00, 0x00, 
    0x00, 0x1b, 0xec, 0x00, 0x00, 
    0x00, 0x0b, 0xe8, 0x00, 0x00, 
    0x00, 0x0d, 0xd8, 0x00, 0x00, 
    0x08, 0x07, 0xf0, 0x18, 0x00, 
    0x1c, 0x07, 0xe0, 0x3c, 0x00, 
    0x3e, 0x01, 0xc0, 0x7e, 0x00, 
    0x3f, 0x1f, 0xfc, 0x7c, 0x00, 
    0x1f, 0x7f, 0xff, 0x7c, 0x00, 
    0x0f, 0xf0, 0x07, 0xf8, 0x00, 
    0x03, 0xc0, 0x83, 0xe0, 0x00, 
    0x07, 0xa0, 0x82, 0xf0, 0x00, 
    0x0f, 0x10, 0x84, 0x78, 0x00, 
    0x1e, 0x00, 0x00, 0x38, 0x00, 
    0x1c, 0x00, 0x00, 0x1c, 0x00, 
    0x1a, 0x00, 0x02, 0x3c, 0x00, 
    0x31, 0x00, 0x06, 0x4e, 0x00, 
    0x30, 0x00, 0x0c, 0x0e, 0x00, 
    0x30, 0x00, 0x18, 0x0e, 0x00, 
    0x70, 0x00, 0x70, 0x06, 0x00, 
    0x60, 0x00, 0xe0, 0x07, 0x00, 
    0x67, 0x01, 0xc0, 0x77, 0x00, 
    0x60, 0x01, 0xc0, 0x07, 0x00, 
    0x70, 0x00, 0x00, 0x07, 0x00, 
    0x70, 0x00, 0x00, 0x06, 0x00, 
    0x30, 0x00, 0x00, 0x0e, 0x00, 
    0x31, 0x00, 0x00, 0x4e, 0x00, 
    0x3a, 0x00, 0x00, 0x2c, 0x00, 
    0x1c, 0x00, 0x00, 0x1c, 0x00, 
    0x1c, 0x00, 0x00, 0x38, 0x00, 
    0x0e, 0x10, 0x84, 0x78, 0x00, 
    0x07, 0x20, 0x82, 0xf0, 0x00, 
    0x03, 0xc0, 0x81, 0xe0, 0x00, 
    0x01, 0xe0, 0x03, 0xc0, 0x00, 
    0x00, 0xfc, 0x1f, 0x80, 0x00, 
    0x00, 0x3f, 0xfe, 0x00, 0x00, 
    0x00, 0x07, 0xf8, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00
};

#endif
