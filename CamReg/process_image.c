#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>
#include "stdio.h"

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>
#define MASK_RED	7
#define MASK_BLUE	0xE0
#define THRESHOLD	30
#define THRESHOLD_BLACK 25
#define MIN_LINE_WIDTH	40
#define PXTOCM 			1570.0f
#define MAX_DISTANCE 	25.0f
#define WIDTH_SLOPE		57
#define GOAL_DISTANCE 	10.0f




static uint16_t line_position = IMAGE_BUFFER_SIZE/2;
volatile static float distance_cm = 0;

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
        //starts a capture
		dcmi_capture_start();
		//time=chVTGetSystemTime();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
		//time=chVTGetSystemTime() - time;
		//chprintf((BaseSequentialStream *) &SDU1, "time = %d \n",time); //regarder sur COM9

    }
}


static THD_WORKING_AREA(waProcessImage, 1024);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr;
	uint8_t image_left[IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_right[IMAGE_BUFFER_SIZE] = {0};
	uint8_t nbr_image=0;
	uint16_t i =0;

    while(1){
    	//waits until an image has been captured
        chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr();

		/*
		*	To complete
		*/

		for(i=0;i<IMAGE_BUFFER_SIZE; i++)
		{
			image_left[i]=img_buff_ptr[2*i] & ~MASK_RED;
			image_right[i]=img_buff_ptr[2*i+1] & ~MASK_BLUE;
		}

		choose_direction(image_left, image_right);
		/*nbr_image=(nbr_image+1)%2;

		if(!nbr_image)
			SendUint8ToComputer(image, IMAGE_BUFFER_SIZE);
			*/

		//distance_cm = PXTOCM/extract_line_width(image);

		//chprintf((BaseSequentialStream *) &SDU1, "width black line = %ld \n distance cm = %f",extract_line_width(image),distance_cm);

    }
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

	for(i;i<IMAGE_BUFFER_SIZE;i++){
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
