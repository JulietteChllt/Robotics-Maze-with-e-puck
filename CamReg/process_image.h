#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H


#define MASK_RED			7
#define MASK_BLUE			0xE0
#define NB_BIT_BLUE_RED 	5
#define HALF_NB_BIT_GREEN	6
#define CODE_BLUE			1
#define CODE_GREEN			2
#define CODE_RED			3
#define THRESHOLD			30
#define THRESHOLD_BLACK 	25
#define MIN_LINE_WIDTH		40
#define PXTOCM 				1570.0f
#define MAX_DISTANCE 		25.0f
#define WIDTH_SLOPE			57
#define GOAL_DISTANCE 		10.0f

float get_distance_cm(void);
uint8_t get_direction(void);
void choose_direction(uint8_t* image_l, uint8_t* image_r);
void process_image_start(void);
uint16_t detect_black_line(uint8_t *image);
uint16_t extract_line_width(uint8_t *buffer);
uint8_t* get_red(uint8_t *image_red, uint8_t *img_buff_ptr);
uint8_t* get_blue(uint8_t *image_blue, uint8_t *img_buff_ptr);
uint8_t* get_green(uint8_t *image_green, uint8_t *img_buff_ptr);
uint8_t get_color(void);
uint8_t determine_color(uint8_t avg_blue, uint8_t avg_green, uint8_t avg_red);
uint8_t avg_color(uint8_t *image_color);
#endif /* PROCESS_IMAGE_H */
