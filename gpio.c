/*
 *  Squeezelite - lightweight headless squeezebox emulator
 *
 *  (c) Adrian Smith 2012-2015, triode1@btinternet.com
 *      Ralph Irving 2015-2016, ralph_irving@hotmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * gpio.c (c) Paul Hermann, 2015-2016 under the same license terms
 *   -Control of Raspberry pi GPIO for amplifier power
 *   -Launch script on power status change from LMS
 */

#if GPIO

#include "squeezelite.h"
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

int gpio_state = -1;
int initialized = -1;
int power_state = -1;
static log_level loglevel;

void relay( int state) {
    gpio_state = state;

  // Set up gpio  using BCM Pin #'s
  if (initialized == -1){
	setenv("WIRINGPI_GPIOMEM", "1", 1);
	wiringPiSetupGpio();
	initialized = 1;
	pinMode (gpio_pin, OUTPUT);
  }

    if(gpio_state == 1)
        digitalWrite(gpio_pin, HIGH^gpio_active_low);
    else if(gpio_state == 0)
        digitalWrite(gpio_pin, LOW^gpio_active_low);
    // Done!
}

char *cmdline;
int argloc;

void relay_script( int state) {
	gpio_state = state;
	int err;

  // Call script with init parameter
	if (initialized == -1){
		int strsize = strlen(power_script);
		cmdline = (char*) malloc(strsize+3);
		argloc = strsize + 1;

		strcpy(cmdline, power_script);
		strcat(cmdline, " 2");
		if ((err = system(cmdline)) != 0){ 
			fprintf (stderr, "%s exit status = %d\n", cmdline, err);
		}
		else{
			initialized = 1;
		}
  	}

  // Call Script to turn on or off  on = 1, off = 0
  // Checks current status to avoid calling script excessivly on track changes where alsa re-inits.

	if( (gpio_state == 1) && (power_state != 1)){
		cmdline[argloc] = '1';
		if ((err = system(cmdline)) != 0){ 
			fprintf (stderr, "%s exit status = %d\n", cmdline, err);
		}
		else {
			power_state = 1;
		}
	}
	else if( (gpio_state == 0) && (power_state != 0)){
		cmdline[argloc] = '0';
		if ((err = system(cmdline)) != 0){ 
			fprintf (stderr, "%s exit status = %d\n", cmdline, err);
		}
		else {
			power_state = 0;
		}
	}
// Done!
}


static bool globalActive = false;
struct timeval t1_light,t2;

int showElapsedTime(void) {
    static int seconds,useconds;
    gettimeofday(&t2, NULL);
    seconds  = t2.tv_sec  - t1_light.tv_sec;
    useconds = t2.tv_usec - t1_light.tv_usec;
    if(useconds < 0) {
      useconds += 1000000;
      seconds--;
    }
    LOG_DEBUG("Button start/end: %d sek %d msek\n",seconds,useconds/1000);
    return seconds;
}

void interrupt_button(){   
  int pin = digitalRead(gpio_button_pin);
  if(pin == LOW && globalActive == false){
    globalActive = true;
    LOG_DEBUG("Button pressed");
    gettimeofday(&t1_light, NULL);
  }else if(pin == HIGH && globalActive == true){    
    int seconds = showElapsedTime();
    LOG_DEBUG("Button Released: %i seconds", seconds);
    sendPlayPauseCLI();
    globalActive = false;
  }  
}

// Button to Pause / Play - add Interrupt that Starts / Pauses player on Button press
void initializeButton(log_level _loglevel) {
  loglevel = _loglevel;
  if (initialized == -1){
	setenv("WIRINGPI_GPIOMEM", "1", 1);
	wiringPiSetupGpio();
	initialized = 1;
	pinMode (gpio_button_pin, INPUT);
	digitalWrite(gpio_button_pin, 0);
	LOG_DEBUG ("... Initialized GPIO Button ... ");
  }  
  wiringPiISR (gpio_button_pin, INT_EDGE_BOTH, &interrupt_button) ;
}
#endif // GPIO
