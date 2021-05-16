#include "ch.h"
#include "hal.h"
#include <usbcfg.h>
#include "stdio.h"
#include <main.h>
#include <camera/po8030.h>
#include <process_image.h>
#include <controle.h>

static volatile uint8_t color=0;
static volatile uint8_t direction=0;

/*=================================================================================================================*/
/*================================================ Thread definition ==============================================*/
/*=================================================================================================================*/

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

// thread  to capture an image
static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void)arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

	while(1){
		wait_semaphore_ready();
		//starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
	}
}

// thread to process the image after it is captured
static THD_WORKING_AREA(waProcessImage, 4096);
static THD_FUNCTION(ProcessImage, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void)arg;

	uint8_t *img_buff_ptr;
	uint8_t image_left[IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_right[IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_arrival[IMAGE_BUFFER_SIZE]={0};
	uint8_t val_red=0, val_blue=0, val_green=0;

	while(1){
		//waits until an image has been captured
		chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr();
		val_blue = avg_color(get_blue(image_right,img_buff_ptr));
		val_red = avg_color(get_red(image_left, img_buff_ptr));
		val_green = avg_color(get_green(image_arrival, img_buff_ptr));
		direction = determine_direction(val_red, val_blue);
		color = determine_color(val_red,val_green,val_blue);
	}
}

/*=================================================================================================================*/
/*================================================ Helper functions ===============================================*/
/*=================================================================================================================*/

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO+1, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO+1, CaptureImage, NULL);
}

uint8_t* get_red(uint8_t *image_red, uint8_t *img_buff_ptr){
	for(uint16_t i =0; i<IMAGE_BUFFER_SIZE; i++){
		image_red[i]=(uint8_t)((img_buff_ptr[2*i]& MASK_RED)>>HALF_NB_BIT_GREEN);
	}
	return image_red;
}

uint8_t* get_blue(uint8_t *image_blue, uint8_t *img_buff_ptr){
	for(uint16_t i =0; i<IMAGE_BUFFER_SIZE; i++){
		image_blue[i]=(uint8_t)(img_buff_ptr[2*i+1]& MASK_BLUE);
	}
	return image_blue;
}

uint8_t* get_green(uint8_t *image_green, uint8_t *img_buff_ptr){
	for(uint16_t i =0; i<IMAGE_BUFFER_SIZE; i++){
		image_green[i]=(uint8_t)(((img_buff_ptr[2*i+1]& ~MASK_BLUE)>>NB_BIT_BLUE_RED)|((img_buff_ptr[2*i]& ~MASK_RED)<<HALF_NB_BIT_GREEN));
	}
	return image_green;
}

uint8_t avg_color(uint8_t* image_color){
	uint32_t avg_pixel = 0;
	for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE ; i++){
		avg_pixel += image_color[i];
	}
	return (uint8_t)(avg_pixel/IMAGE_BUFFER_SIZE);
}

uint8_t determine_color(uint8_t avg_red, uint8_t avg_green, uint8_t avg_blue){
	if(avg_blue>avg_green && avg_blue>= avg_red)
		return CODE_BLUE;
	else if (avg_green>avg_blue && avg_green>avg_red)
		return CODE_GREEN;
	else
		return CODE_RED;
}

uint8_t get_color(void){
return color;
}

uint8_t determine_direction(uint8_t avg_red, uint8_t avg_blue){
	if(avg_blue >= avg_red)
		return CODE_BLUE;
	else
		return CODE_RED;
}

uint8_t get_direction(void){
	return direction;
}
