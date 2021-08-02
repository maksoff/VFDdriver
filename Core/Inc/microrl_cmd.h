/*
 * microrl_cmd.h
 *
 *  Created on: Oct 28, 2018
 *      Author: makso
 */

#ifndef MICRORL_CMD_H_
#define MICRORL_CMD_H_

#include "microrl.h"

#define _VER "VFD Driver v1.0"

//extern microrl_t * p_mcrl;

// color


#define COLOR_CODE_LENGTH		(9)

#define COLOR_NC				"\e[0m" 	// default
#define COLOR_WHITE				"\e[1;37m"
#define COLOR_BLACK				"\e[0;30m"
#define COLOR_BLUE				"\e[0;34m"
#define COLOR_LIGHT_BLUE		"\e[1;34m"
#define COLOR_GREEN				"\e[0;32m"
#define COLOR_LIGHT_GREEN		"\e[1;32m"
#define COLOR_CYAN				"\e[0;36m"
#define COLOR_LIGHT_CYAN		"\e[1;36m"
#define COLOR_RED				"\e[0;31m"
#define COLOR_LIGHT_RED			"\e[1;31m"
#define COLOR_PURPLE			"\e[0;35m"
#define COLOR_LIGHT_PURPLE		"\e[1;35m"
#define COLOR_BROWN				"\e[0;33m"
#define COLOR_YELLOW			"\e[1;33m"
#define COLOR_GRAY				"\e[0;30m"
#define COLOR_LIGHT_GRAY		"\e[0;37m"

const typedef enum {
	C_NC,
	C_WHITE,
	C_BLACK,
	C_BLUE,
	C_L_BLUE,
	C_GREEN,
	C_L_GREEN,
	C_CYAN,
	C_L_CYAN,
	C_RED,
	C_L_RED,
	C_PURPLE,
	C_L_PURPLE,
	C_BROWN,
	C_YELLOW,
	C_GRAY,
	C_L_GRAY
} microrl_color_e;

//main features

void microrl_print_char(char buf);

void init_microrl(void);

void print (const char * str);
int print_color(const char * str, microrl_color_e color);
void set_CDC_ready(void);

// main functions

int print_help 		(int argc, const char * const * argv);
int echo_toggle 	(int argc, const char * const * argv);
int echo_on 		(int argc, const char * const * argv);
int echo_off 		(int argc, const char * const * argv);
int echo_show 		(int argc, const char * const * argv);
int echo_enter 		(int argc, const char * const * argv);
int color_toggle 	(int argc, const char * const * argv);
int color_on 		(int argc, const char * const * argv);
int color_off 		(int argc, const char * const * argv);
int color_show 		(int argc, const char * const * argv);
int clear_screen 	(int argc, const char * const * argv);

int nema_toggle 	(int argc, const char * const * argv);
int nema_on 		(int argc, const char * const * argv);
int nema_off 		(int argc, const char * const * argv);

int vfd 			(int argc, const char * const * argv);

int show_encoder	(int argc, const char * const * argv);

int set_td		(int argc, const char * const * argv);
int get_td		(int argc, const char * const * argv);
int get_temp		(int argc, const char * const * argv);
int leds_toggle		(int argc, const char * const * argv);

bool get_nema(void);

extern bool show_clock;
extern bool use_leds;

//int led_on 			(int argc, const char * const * argv);
//int led_off 		(int argc, const char * const * argv);
//int led_toggle 		(int argc, const char * const * argv);
//int led_show 		(int argc, const char * const * argv);
//int led_tick 		(int argc, const char * const * argv);
//int led_dcf77 		(int argc, const char * const * argv);
//int time_show 		(int argc, const char * const * argv);
//int time_set 		(int argc, const char * const * argv);
//int print_time 		(int argc, const char * const * argv);
//int date_show 		(int argc, const char * const * argv);
//int date_set 		(int argc, const char * const * argv);
//int print_date 		(int argc, const char * const * argv);
//int print_weekday 	(int argc, const char * const * argv);
//int tack_toggle		(int argc, const char * const * argv);

#define EMPTY_CMD_HELP "[]"

#define MICRORL_CMD_LENGTH (9)
#define MICRORL_HELP_MSG_LENGTH (26)

typedef struct{
	int level;									// 0: top, 1: next, 2: next next; -1: same functions as above
	char cmd	  [MICRORL_CMD_LENGTH];			// command name
	char help_msg [MICRORL_HELP_MSG_LENGTH];	// help message
	int (*func)   (int argc, const char * const * argv ); // pointer to function
} microrl_action_t;





#endif /* MICRORL_CMD_H_ */
