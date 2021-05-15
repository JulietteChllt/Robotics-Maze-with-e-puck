#ifndef CONTROLE_H_
#define CONTROLE_H_

#define SENSORFRONT1 				0
#define SENSORFRONT2				7
#define SENSORLEFT					5
#define SENSORRIGHT					2
#define THRESHOLD_CLOSE_OBSTACLE 	100 							//adapted depending on ambient light
#define THRESHOLD_WALL_IN_FRONT 	THRESHOLD_CLOSE_OBSTACLE+80
#define STABILITY_THRESHOLD 		20
#define QUARTER_TURN 				310
#define SMALL_STEP_FORWARD 			500
#define SMALL_STEP_BACKWARD 		300

//Starts the thread
void controle_start(void);

//Puts on hold the semaphore dealing with the camera
void wait_semaphore_ready(void);

//Notifies that priority should be given to the thread capturing an image with the camera
void semaphore_ready(void);

//Return 0 if the sensor detects a wall, 1 otherwise
uint8_t get_free_space(uint8_t sensor1);

//Return 0 if the sensor detects a wall in front (using the 2 IR sensors in front), and 1 otherwise
uint8_t get_free_space_front(void);

//Return 0 if the sensor detects a wall on its left, 1 otherwise
uint8_t get_free_space_left(void);

//Return 0 if the sensor detects a wall on its right , 1 otherwise
uint8_t get_free_space_right(void);

// Computes the reference for the wall to follow
void do_new_reference(uint8_t sensor);

//Getter for the variable_reference
int get_reference(void);

//Returns the number of possible directions
uint8_t get_possible_directions(void);

//Getter to know if the direction has changed
uint8_t get_direction_changed(void);

#endif /* CONTROLE_H_ */
