#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#define MASK_RED			0xF8
#define MASK_BLUE			0x1F
#define NB_BIT_BLUE_RED 	5		// number of bits allocated to the color red (resp blue) in rgb value
#define HALF_NB_BIT_GREEN	3		// half of the number of bits allocated to the color green in rgb value

//Creates the 2 threads
void process_image_start(void);

//returns the 5 MSB of RGB value
uint8_t* get_red(uint8_t *image_red, uint8_t *img_buff_ptr);

//returns the 5 LSB of RGB value
uint8_t* get_blue(uint8_t *image_blue, uint8_t *img_buff_ptr);

//returns the 6 bits corresponding to the green color in the RGB value
uint8_t* get_green(uint8_t *image_green, uint8_t *img_buff_ptr);

//returns the average value of one color over the pixels taken
uint8_t avg_color(uint8_t* image_color);

//returns dominant color over the pixels taken
uint8_t determine_color(uint8_t avg_red, uint8_t avg_green, uint8_t avg_blue);

//getter to access the dominant color of the captured pixels
uint8_t get_color(void);

/*returns the dominant color between blue and red
 * this function will be used to determine which way to go at an intersection
 */
uint8_t determine_direction(uint8_t avg_red, uint8_t avg_blue);

//getter to access the dominant color (between blue and red) of the captured pixels
uint8_t get_direction(void);
#endif /* PROCESS_IMAGE_H */
