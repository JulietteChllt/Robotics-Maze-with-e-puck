#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

float get_distance_cm(void);
uint8_t get_direction(void);
void choose_direction(uint8_t* image_l, uint8_t* image_r);
void process_image_start(void);
uint16_t detect_black_line(uint8_t *image);
uint16_t extract_line_width(uint8_t *buffer);
#endif /* PROCESS_IMAGE_H */
