/*
 * microrl_cmd.c
 *
 *  Created on: Jun 23, 2021
 *      Author: makso
 */

#include "main.h"
#include "microrl_cmd.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "usbd_cdc.h"
#include "freertos_inc.h"

#include "vfd.h"
#include "d3231.h"

microrl_t mcrl;
microrl_t * p_mcrl = &mcrl;

bool color_out = true;
bool CDC_is_ready = false;
bool nema_out = false;
bool show_clock = true;
bool use_leds = USE_LEDS;

/********************************
 *
 */

/*
 * Ex. Menu Structure:						should be formated in this way:
 * help 	-- help message					{0, 	"help", "help message", print_help},
 * h		-- -//-							{-1, 	"h", 	"", 			NULL},
 * clr		-- clear screen					{0,		"clr",  "clear screen",	clear_scr},
 * led  	-- led toggle					{0, 	"led",  "led toggle,	led_toggle},
 * lamp  	-- -//-							{-1,	"lamp",	"",				NULL},
 *   on 	-- turn led on					{1, 	"on",	"turn led on",	led_on},
 *   off 	-- turn led off					{1, 	"off",	"turn led off",	led_off},
 * time		-- show time once				{0,		"time",	"show time once", print_time},
 *   show   -- autoupdate time				{1,		"show",	"autoupdate time", print_time_auto},
 *   auto	-- -//-							{-1,	"auto", "",				NULL},
 *     simple -- autoupdate without esc		{2,		"simple", "autoupdate without esc", print_time_no_esc}
 *
 * !      -//- == synonym for function above
 * !!!    order of lines is important! the alternative names and sublevel commands are referenced for function above.
 */

const microrl_action_t microrl_actions [] =
{
		{ 0, 		"help", 	"this message", 			print_help},
		{-1,		"h", 		"", 						NULL},
		{-1,		"?", 		"", 						NULL},
		{ 0,		"color",	"toggle spec characters",	color_toggle},
		{   1,		"on",		"turn on",					color_on},
		{   1,		"off",		"turn off",					color_off},
		{   1,		"show", 	"show color",				color_show},
		{ 0,		"clear", 	"clear screen", 			clear_screen},
		{-1,		"clr", 		"", 						NULL},
		{-1,		"clrscr",	"", 						NULL},
//		{ 0,		"nema",		"toggle NEMA debug",		nema_toggle},
		{ 0,		"leds",		"toggle LEDs",				leds_toggle},
		{ 0,		"temp",		"read temperature",			get_temp},
		{ 0,		"vfd",		"put text on vfd display",	vfd_text},
		{ 0,		"clock",	"displays clock (or Ctrl+C)",	clock},
		{ 0,		"time",		"print time",				get_td},
		{   1,		"set",		"time set hhmmss",			set_td},
		{ 0,		"date",		"print date",				get_td},
		{   1,		"set",		"date set yymmdd",			set_td},
//		{ 0,		"led",		"toggle led",				led_toggle},
//		{   1,		"on",		"turn on",					led_on},
//		{   1,		"off",		"turn off",					led_off},
//		{   1,		"show", 	"show led",					led_show},
//		{   1, 		"tick",		"toggle every second",		led_tick},
//		{   1, 		"dcf",		"bypass DCF77 signal",		led_dcf77},
//		{ 0,		"time",		"print time",				print_time},
//		{   1,		"auto", 	"auto update",				time_show},
//		{  -1,		"show", 	"",							NULL},
//		{     2,	"simple", 	"auto for logging",			time_show},
//		{ 	1,		"set",		"time set 'hh:mm:ss'",		time_set},
//		{ 0,		"date",		"print date",				print_date},
//		{   1,		"auto", 	"print date with auto time", date_show},
//		{  -1,		"show", 	"",							NULL},
//		{     2,	"simple", 	"auto for logging",			date_show},
//		{ 	1,		"set",		"date set 'YYYY.MM.DD'",	date_set},
//		{ 0,		"set",		"sets time or date", 		NULL},
//		{   1,		"time",		"sets time 'hh:mm:ss'",		time_set},
//		{   1, 		"date", 	"sets date 'YYYY.MM.DD'",	date_set},
//		{ 0,		"weekday",	"day of the week",			print_weekday},
//		{ 0,		"echo",		"toggle echo",		echo_toggle},
//		{   1,		"on",		"turn on",			echo_on},
//		{   1,		"off",		"turn off",			echo_off},
//		{   1,		"show", 	"show echo",		echo_show},
//		{   1,      "once",     "turn off, enable on Enter", echo_enter},
//		{ 0,		"tack",		"toggle seconds update",	tack_toggle},
};

#define microrl_actions_length (sizeof(microrl_actions)/sizeof(microrl_action_t))

// array for completion
char * compl_word [microrl_actions_length + 1];



typedef struct {
	microrl_color_e name;
	char code[10];
} microrl_color_t;

const microrl_color_t microrl_color_lookup [] =
{
		{C_NC,		COLOR_NC},
		{C_WHITE,	COLOR_WHITE},
		{C_BLACK,	COLOR_BLACK},
		{C_BLUE,	COLOR_BLUE},
		{C_L_BLUE,	COLOR_LIGHT_BLUE},
		{C_GREEN,	COLOR_GREEN},
		{C_L_GREEN,	COLOR_LIGHT_GREEN},
		{C_CYAN,	COLOR_CYAN},
		{C_L_CYAN,	COLOR_LIGHT_CYAN},
		{C_RED,		COLOR_RED},
		{C_L_RED,	COLOR_LIGHT_RED},
		{C_PURPLE,	COLOR_PURPLE},
		{C_L_PURPLE,COLOR_LIGHT_PURPLE},
		{C_BROWN,	COLOR_BROWN},
		{C_YELLOW,	COLOR_YELLOW},
		{C_GRAY,	COLOR_GRAY},
		{C_L_GRAY,	COLOR_LIGHT_GRAY}
};

#define microrl_color_lookup_length (sizeof(microrl_color_lookup)/sizeof(microrl_color_t))

const microrl_color_e microrl_help_color [] =
{
		C_GREEN,
		C_L_GREEN,
		C_PURPLE,
		C_L_PURPLE
};

#define microrl_help_color_lenght (sizeof(microrl_help_color)/sizeof(microrl_color_e))


/****************************************************************
 *
 */

void microrl_print_char(char buf)
{
	microrl_insert_char(p_mcrl, (int) buf);
}

void print (const char * str)
{
	if ((!color_out) && (str[0] == '\e')) // don't print escape characters
		return;
	if (!CDC_is_ready)
		return;
	uint16_t len = 0;
	while (str[++len] != 0);
	uint32_t timeout = HAL_GetTick();
	while (((USBD_CDC_HandleTypeDef*)(hUsbDeviceFS.pClassData))->TxState!=0)
		if (HAL_GetTick() - timeout >= 5)
			break;
	CDC_Transmit_FS((uint8_t*)str, len);
}

int find_color_by_name(microrl_color_e color)
{
	for (int i = 0; i < microrl_color_lookup_length; i++)
	{
		if (microrl_color_lookup[i].name == color)
		{
			return i;
		}
	}
	return 0;
}

int print_color(const char * str, microrl_color_e color)
{
	print(microrl_color_lookup[find_color_by_name(color)].code);
	print(str);
	print(COLOR_NC);
	return 0;
}

int str_length(const char * str)
{
	int i = 0;
	while (str[i])
		i++;
	return i;
}


int print_help(int argc, const char * const * argv)
{
	print(_VER);
	print(ENDL);
	print ("Use ");
	print_color("TAB", C_GREEN);
	print(" key for completion");
	print (ENDL);
	print ("Available commands:");
	for (int i = 0; i < microrl_actions_length; i++)
	{
		if (microrl_actions[i].level == -1) // print synonyms
		{
			assert_param(i > 0);
			if (microrl_actions[i - 1].level != -1)
				print_color(" aka ", C_L_PURPLE);
			else
				print_color("/", C_L_PURPLE);
			print_color (microrl_actions[i].cmd, C_PURPLE);
		}
		else
		{
			print(ENDL);
			for (int e = -4; e < microrl_actions[i].level; e++)
				print(" ");
			print_color(microrl_actions[i].cmd, microrl_help_color[microrl_actions[i].level]);
			for (int e = 0; e < MICRORL_CMD_LENGTH + 2 -
								microrl_actions[i].level - str_length(microrl_actions[i].cmd); e++)
				print (" ");
			switch (microrl_actions[i].level){
			case 0:
				print ("-");
				break;
			case 1:
				print ("^");
				break;
			default:
				print ("#");
				break;
			}
			print (" ");
			print (microrl_actions[i].help_msg);
		}
	}
	print(ENDL);
	return 0;
}



int execute (int argc, const char * const * argv)
{
//	print_help(argc, argv);
//	return 0;
	int (*func)   (int argc, const char * const * argv ) = NULL;

	/*
	 * iterate throw levels and synonyms - run the func from the first (main) synonym
	 * run last found functions with all parameters - functions should check or ignore additional parameters
	 * if nothing found - show err msg
	 */

	int last_main_synonym = 0;
	int synonym_level = 0;
	bool tokens_found = false;
	for (int i = 0; i < argc; i++)
	{
		for (int n = last_main_synonym; n < microrl_actions_length; n++)
		{
			tokens_found = false;
			int current_level = microrl_actions[n].level;
			// next higher level command found, break;
			if (current_level != -1)
				synonym_level = current_level; // save the synonym level
			if ((current_level != -1) && (current_level < i))
				break;
			if (current_level == i)
				last_main_synonym = n;
			if ((strcmp(argv[i], microrl_actions[n].cmd) == 0) &&
					(i == synonym_level))
			{
				tokens_found = true;
				func = microrl_actions[last_main_synonym++].func;
				break;
			}
		}
		if (!tokens_found)	// nothing found, nothing to do here
			break;
	}

	if (func != NULL)
	{
		return func(argc, argv); // function found
	} else if (tokens_found)
	{
		print_color ("command: '", C_L_RED);
		print_color ((char*)argv[0], C_L_RED);
		print_color ("' needs additional arguments", C_L_RED);
		print(ENDL);
		print_color ("use '", C_NC);
		print_color ("?", C_GREEN);
		print_color ("' for help", C_NC);
		print (ENDL);
		return 1;
	}
	else
	{
		print_color ("command: '", C_RED);
		print_color ((char*)argv[0], C_RED);
		print_color ("' not found", C_RED);
		print(ENDL);
		print_color ("use '", C_NC);
		print_color ("?", C_GREEN);
		print_color ("' for help", C_NC);
		print (ENDL);
		return 1;

	}
}

#ifdef _USE_COMPLETE
//*****************************************************************************
// completion callback for microrl library
char ** complet (int argc, const char * const * argv)
{
	int j = 0;

	compl_word [0] = NULL;

	/*
	 * if no parameters - print all cmd with friend =="" && father == ""
	 * if parameter == 1 search with parent == ""
	 * if parameter > 1 search with parent == (parameter-2)
	 */

	/*
	 * print cmd and synonyms with level == argc-1.
	 * if argc == 0 print without synonyms
	 */

	if (argc == 0)
	{
		// if there is no token in cmdline, just print all available token
		for (int i = 0; i < microrl_actions_length; i++) {
			if (microrl_actions[i].level == 0)
			compl_word[j++] = (char*) microrl_actions [i].cmd;
		}
	} else {
		// get last entered token
		char * bit = (char*)argv [argc-1];
		// iterate through our available token and match it
		// based on previous tokens in the line, find the correct one shift
		int last_main_synonym = 0;
		int synonym_level = 0;
		bool tokens_found = false;
		for (int i = 0; i < argc; i++)
		{
			for (int n = last_main_synonym; n < microrl_actions_length; n++)
			{
				tokens_found = false;
				int current_level = microrl_actions[n].level;
				// next higher level command found, break;
				if (current_level != -1)
					synonym_level = current_level; // save the synonym level
				if ((current_level != -1) && (current_level < i))
					break;
				if (current_level == i)
					last_main_synonym = n;
				if ((i == argc - 1) && (strstr(microrl_actions [n].cmd, bit) == microrl_actions [n].cmd) &&
										(i == synonym_level))
				{
					tokens_found = true;
					compl_word [j++] =(char*) microrl_actions [n].cmd;
				}
				else if ((strcmp(argv[i], microrl_actions[n].cmd) == 0) && (i == synonym_level))
				{
					last_main_synonym++;
					tokens_found = true;
					break;
				}
			}
			if (!tokens_found)	// nothing found, nothing to do here
				break;
		}
	}

	// note! last ptr in array always must be NULL!!!
	compl_word [j] = NULL;
	// return set of variants
	return compl_word;
}
#endif


void sigint (void)
{
	//TODO add functions
	nema_out = false;
	show_clock = true;

	print (ENDL);
	print ("^C catched!");
	int i = 0;
	while (ENTER[i])
		microrl_insert_char(p_mcrl, ENTER[i++]);
}

void init_microrl(void)
{
	  microrl_init(p_mcrl, print);
	  // set callback for execute
	  microrl_set_execute_callback (p_mcrl, execute);

	#ifdef _USE_COMPLETE
	  // set callback for completion
	  microrl_set_complete_callback (p_mcrl, complet);
	#endif
	  // set callback for Ctrl+C
	  microrl_set_sigint_callback (p_mcrl, sigint);
}


int clear_screen(int argc, const char * const * argv)
{
	print ("\033[2J");    // ESC seq for clear entire screen
	print ("\033[H");     // ESC seq for move cursor at left-top corner
	return 0;
}


void set_CDC_ready(void)
{
	CDC_is_ready = true;
}


int color_toggle 	(int argc, const char * const * argv)
{
	color_out ^= 1;
	return 0;
}

int color_on 		(int argc, const char * const * argv)
{
	color_out = 1;
	print_color ("Color output is ON", C_GREEN);
	print(ENDL);
	return 0;
}

int color_off 		(int argc, const char * const * argv)
{
	color_out = 0;
	print ("Color output is OFF");
	print(ENDL);
	return 0;
}

int color_show 		(int argc, const char * const * argv)
{
	if (color_out)
		print_color("Color output is ON", C_GREEN);
	else
		print ("Color output is OFF");
	print(ENDL);
	return 0;
}


int nema_toggle 	(int argc, const char * const * argv)
{
	nema_out ^= 1;
	return 0;
}

int nema_on 		(int argc, const char * const * argv)
{
	nema_out = 1;
	print(ENDL);
	return 0;
}

int nema_off 		(int argc, const char * const * argv)
{
	nema_out = 0;
	print(ENDL);
	return 0;
}

int vfd_text (int argc, const char * const * argv)
{
	show_clock = false;
	for (int i = 1; i < argc; i++)
	{
		uint16_t temp = 0;
		char * pchar = (char*)argv[i];
		xQueueSendToBack(qVFDHandle, &temp, 100);
		while (*pchar)
		{
			temp = get_char(*(pchar++));
			xQueueSendToBack(qVFDHandle, &temp, 100);
		}
	}
	return 0;
}


int show_encoder (int argc, const char * const * argv)
{

	uint32_t buf = encoder_value;
	char str [8];

	for (int i = 0; i < 5; i++)
	{
		str[4 - i] = buf % 10 + '0';
		buf /= 10;
	}
	str[5] = '\r';
	str[6] = '\n';
	str[7] = '\0';
	print(str);
	return 0;
}


bool get_nema(void)
{
	return nema_out;
}


int leds_toggle		(int argc, const char * const * argv)
{
	use_leds ^= 1;
	print_color("Done", C_GREEN);
	print(ENDL);
	return 0;
}



int set_td		(int argc, const char * const * argv)
{
	if (argc == 3 && str_length(argv[2]) == 6)
	{
		uint8_t arr [3];
		for (int i = 0; i < 3; i++)
		{
			arr[2-i] = (argv[2][i*2+1]-'0')+((argv[2][i*2]-'0')<<4);
		}
		d3231_set(arr, argv[0][0] == 'd');
		return 0;
	}
	print_color("wrong format", C_RED);
	print(ENDL);
	return 0;
}

int get_td		(int argc, const char * const * argv)
{
	bool date = argv[0][0] == 'd';
	char str[9];
	uint8_t * d3231 = d3231_get_all();
	uint8_t offset = date*4;
	str[8] = '\0';
	str[7] = (d3231[offset + 0]&0xF) + '0';
	str[6] = ((d3231[offset + 0]>>4)&0xF) + '0';
	str[4] = (d3231[offset + 1]&0xF) + '0';
	str[3] = ((d3231[offset + 1]>>4)&0xF) + '0';
	str[1] = (d3231[offset + 2]&0xF) + '0';
	str[0] = ((d3231[offset + 2]>>4)&0xF) + '0';
	str[5] = str[2] = date?'-':':';
	print_color(str, C_L_BLUE);
	print(ENDL);
	return 0;
}

int get_temp		(int argc, const char * const * argv)
{
	uint8_t * d3231 = d3231_get_temp();

	bool negative = d3231[0]&(1<<7);
	uint16_t temp = d3231[0]&(~(1<<7));
	uint8_t dec   = d3231[1]>>6;

	dec *= 25; // calculate decimal part
	temp *= 1000;
	temp += dec;


	char str[8];
	str[7] = '\0';
	for (int i = 6; i >= 0; i--)
	{
		str[i] = (temp % 10) + '0';
		temp /= 10;
	}
	str[4] = '.';
	for (int i = 0; i < 7; i++)
	{
		if (str[i] != '0')
		{
			if (i > 0)
				str[i-1] = negative?'-':'+';
			break;
		}
		str[i] = ' ';
	}
	print_color(str, C_GREEN);
	print(ENDL);
	return 0;
}

int clock		(int argc, const char * const * argv)
{
	show_clock = true;
	return 0;
}
