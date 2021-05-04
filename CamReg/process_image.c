#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>
#include "stdio.h"

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>
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




static uint16_t line_position = IMAGE_BUFFER_SIZE/2;
volatile static float distance_cm = 0;
volatile static uint8_t color=0;

static uint8_t direction=0; //gauche = 1 et droite = 2 pour pas avoir de probleme avec 0(?)

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void)arg;
	//systime_t time;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

	while(1){
		//AJOUTER UN SEMAPHORE QUI INDIQUE QUAND PRENDRE UNE IMAGE!!!!!!!!
		wait_semaphore_ready();
		//starts a capture
		dcmi_capture_start();
		//time=chVTGetSystemTime();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
		//time=chVTGetSystemTime() - time;
		//chprintf((BaseSequentialStream *) &SDU1, "dans capture image"); //regarder sur COM9

	}
}


static THD_WORKING_AREA(waProcessImage, 1024);
static THD_FUNCTION(ProcessImage, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void)arg;

	uint8_t *img_buff_ptr;
	uint8_t nbr_image=0;
	uint8_t image_blue[IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_green[IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_red[IMAGE_BUFFER_SIZE] = {0};


	//uint16_t i =0;

	while(1){
		//waits until an image has been captured
		chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr();

		color = get_color(avg_color(get_blue(image_blue, img_buff_ptr)),avg_color(get_green(image_green, img_buff_ptr)),avg_color(get_red(image_red, img_buff_ptr)));
<<<<<<< HEAD
		switch(color){
		case CODE_BLUE:
			//turn right
			break;
		case CODE_RED:
			//turn left
			break;
		case CODE_GREEN:
			//light up leds
			break;
		}
=======

		//chprintf((BaseSequentialStream *) &SDU1,"dans process image color = %d", color);



>>>>>>> balt
	}
}

uint8_t* get_red(uint8_t *image_red, uint8_t *img_buff_ptr){
	for(uint16_t i =0; i<IMAGE_BUFFER_SIZE; i++){
		//extract the 5 MSB of RGB value
		image_red[i]=(uint8_t)((img_buff_ptr[2*i]& ~MASK_RED)>>HALF_NB_BIT_GREEN);
	}
	return image_red;
}

uint8_t* get_blue(uint8_t *image_blue, uint8_t *img_buff_ptr){
	for(uint16_t i =0; i<IMAGE_BUFFER_SIZE; i++){
		//extract the 5 LSB of RGB value
		image_blue[i]=(uint8_t)(img_buff_ptr[2*i+1]& ~MASK_BLUE);
	}
	return image_blue;
}

uint8_t* get_green(uint8_t *image_green, uint8_t *img_buff_ptr){
	for(uint16_t i =0; i<IMAGE_BUFFER_SIZE; i++){
		//extract the 6 bits corresponding to the green color in the RGB value
		image_green[i]=(uint8_t)(((img_buff_ptr[2*i+1]& MASK_BLUE)>>5)+((img_buff_ptr[2*i]&MASK_RED)<<3));
	}
	return image_green;
}

// maybe putting a threshold would be better
uint8_t determine_color(uint8_t avg_blue, uint8_t avg_green, uint8_t avg_red){
	if(avg_blue>avg_green && avg_blue> avg_red)
		return CODE_BLUE;
	else if (avg_green>avg_blue && avg_green>avg_red)
		return CODE_GREEN;
	else
		return CODE_RED;
}

uint8_t get_color(void){
	return color;
}

uint8_t avg_color(uint8_t* image_color){
	uint32_t avg_pixel = 0;
	for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE ; i++){
		avg_pixel += image_color[i];
	}
	return (uint8_t)(avg_pixel / IMAGE_BUFFER_SIZE);

}


float get_distance_cm(void){
	return distance_cm;
}

uint8_t get_direction(void){
	return direction;
}

void choose_direction(uint8_t* image_l, uint8_t* image_r){

	uint16_t value_l=0;
	uint16_t value_r=0; //ATTENTION PEUT ETRE 16 BIT TROP PETIT
	uint16_t i=0;

	for(i=0;i<IMAGE_BUFFER_SIZE;i++){
		value_l+=image_l[i];
		value_r+=image_r[i];
	}

	if(value_l < value_r){ //AJOUTER THRESHOLD??
		direction = RIGHT;
	}

	else direction = LEFT;
}

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}

uint16_t detect_black_line(uint8_t * image){
	uint32_t avg_pixel = 0;
	uint16_t i = 0;
	uint16_t start_pixel = 0;
	uint16_t stop_pixel = 0;
	uint16_t width = 0;
	uint8_t found_start=0;
	uint8_t found_end=0;
	uint8_t found_right_line=0;

	//find mean value for the pixel from this line
	for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE ; i++){
		avg_pixel += image[i];
	}
	avg_pixel /= IMAGE_BUFFER_SIZE;

	for(i=0; i<IMAGE_BUFFER_SIZE-5; i++){
		//find start of black line
		if(found_start==0 && image[i]>avg_pixel && image[i+5]<avg_pixel){
			start_pixel =i;
			found_start=1;
		}
		//if start found, look for the end of the line
		if(found_start==1 && image[i]<avg_pixel && image[i+5]>avg_pixel){
			stop_pixel = i+5;
			found_end = 1;
		}
		//if found end, check that the length is superior to the minimal width
		if(found_right_line==0 && found_end==1 && (stop_pixel-start_pixel) < MIN_LINE_WIDTH){
			i--;
			start_pixel = 0;
			stop_pixel = 0;
			found_start = 0;
			found_end=0;
		}
		else if (found_right_line==0 && found_end==1){
			found_right_line=1;
		}
	}

	if(found_right_line==1){
		width= stop_pixel-start_pixel;
		line_position = (start_pixel+stop_pixel)/2;
		return width;
	}
	else{
		return 0;;
	}
}

uint16_t extract_line_width(uint8_t *buffer){

	uint16_t i = 0, begin = 0, end = 0, width = 0;
	uint8_t stop = 0, wrong_line = 0, line_not_found = 0;
	uint32_t mean = 0;

	static uint16_t last_width = PXTOCM/GOAL_DISTANCE;

	//performs an average
	for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE ; i++){
		mean += buffer[i];
	}
	mean /= IMAGE_BUFFER_SIZE;

	do{
		wrong_line = 0;
		//search for a begin
		while(stop == 0 && i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE))
		{
			//the slope must at least be WIDTH_SLOPE wide and is compared
			//to the mean of the image
			if(buffer[i] > mean && buffer[i+WIDTH_SLOPE] < mean)
			{
				begin = i;
				stop = 1;
			}
			i++;
		}
		//if a begin was found, search for an end
		if (i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE) && begin)
		{
			stop = 0;

			while(stop == 0 && i < IMAGE_BUFFER_SIZE)
			{
				if(buffer[i] > mean && buffer[i-WIDTH_SLOPE] < mean)
				{
					end = i;
					stop = 1;
				}
				i++;
			}
			//if an end was not found
			if (i > IMAGE_BUFFER_SIZE || !end)
			{
				line_not_found = 1;
			}
		}
		else//if no begin was found
		{
			line_not_found = 1;
		}

		//if a line too small has been detected, continues the search
		if(!line_not_found && (end-begin) < MIN_LINE_WIDTH){
			i = end;
			begin = 0;
			end = 0;
			stop = 0;
			wrong_line = 1;
		}
	}while(wrong_line);

	if(line_not_found){
		begin = 0;
		end = 0;
		width = last_width;
	}else{
		last_width = width = (end - begin);
		line_position = (begin + end)/2; //gives the line position.
	}

	//sets a maximum width or returns the measured width
	if((PXTOCM/width) > MAX_DISTANCE){
		return PXTOCM/MAX_DISTANCE;
	}else{
		return width;
	}
}
