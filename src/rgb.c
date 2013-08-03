#include <avr/io.h>
#include <avr/interrupt.h>


#define F_CPU					8000000UL
#include <util/delay.h>

#define fucking				unsigned
#define MAX_INTERVAL	128
#define WAIT_MS				1

#define PIN_RED 0x02
#define PIN_GREEN 0x08
#define PIN_BLUE 0x10

#define PORT_MASK (PIN_RED | PIN_GREEN | PIN_BLUE)

/* globals */
volatile fucking char comp_R, comp_G, comp_B;
volatile fucking char timer_overflow_count = 0;
volatile fucking char pin_level = 0x00;

volatile fucking char rng_x = 37;
volatile fucking char rng_a = 5;
volatile fucking char rng_c = 51;

const unsigned char pwmtable[32] = { 0, 1, 2, 3, 4, 6, 7, 9, 10, 12, 13, 15, 17, 19, 21, 23, 25, 27, 30, 33, 36, 39, 42, 46, 51, 56, 61, 68, 76, 87, 102, 128 };

/* ISR */
ISR( TIMER0_OVF_vect )
{

	PORTB = pin_level & PORT_MASK;
	timer_overflow_count = (timer_overflow_count + 1) % MAX_INTERVAL;
	
	if(timer_overflow_count < comp_R) {
		pin_level |= PIN_RED;
	} else {
		pin_level &= ~PIN_RED;
	}
	
	if((timer_overflow_count + MAX_INTERVAL / 3) % MAX_INTERVAL < comp_G) {
		pin_level |= PIN_GREEN;
	} else {
		pin_level &= ~PIN_GREEN;
	}

	
	if((timer_overflow_count + MAX_INTERVAL / 3 * 2) % MAX_INTERVAL < comp_B) {
		pin_level |= PIN_BLUE;
	} else {
		pin_level &= ~PIN_BLUE;
	}

}

/* initialisation */
void init( void )
{
	cli();
	
	/* clock prescaler */
	CLKPR = (1 << CLKPCE);
	CLKPR = 0;

	/* enable output ports */
	DDRB = PORT_MASK;
	PORTB = 0x00 & (PORTB | ~PORT_MASK);

	/* timer init */
	TIFR &= ~(1 << TOV0);
	TIMSK |= (1 << TOIE0);

	TCCR0B = (TCCR0B & 0xF8) | (0x01 & 0x07);

	sei();

	comp_R = 10;
	comp_G = 10;
	comp_B = 10;
}

/* set the color to RGB-Value */
void set_RGB( fucking char r, fucking char g, fucking char b )
{
	comp_R = pwmtable[ r >> 3 ];
	comp_G = pwmtable[ g >> 3 ];
	comp_B = pwmtable[ b >> 3 ];
}

/* create new random number with acm-method*/
fucking char random() {
	
	rng_x = ((rng_x * rng_a) + rng_c) % 256;
	return rng_x;

}

/* fade from color 1 to color 2 in n easy steps ... */
void fade_color(fucking char r1, fucking char g1, fucking char b1, fucking char r2, fucking char g2, fucking char b2, fucking char range) {
	fucking char i = 0;
	while(i++ < range) {
		_delay_ms(WAIT_MS);
		set_RGB((r1 + (r2 - r1) * i / 255) , (g1 + (g2 - g1) * i / 255), (b1 + (b2 - b1) * i / 255));
	}
}

/* compute rgb-value from hsv-colospace */
void h_to_rgb(fucking char h, fucking char *r, fucking char *g, fucking char *b) {
	if ( h <= 42 ) {
		*r = 255;
		*g = h * 6;
		*b = 0;
	} else if ( h > 42 && h <= 85 ) {
		*r = (85 - h) * 6;
		*g = 255;
		*b = 0;
	} else if ( h > 85 && h <= 127 ) {
		*r = 0;
		*g = 255;
		*b = (h - 85) * 6;
	} else if ( h > 127 && h <= 170 ) {
		*r = 0;
		*g = (170 - h) * 6;
		*b = 255;
	} else if ( h > 170 && h <= 212 ) {
		*r = (h - 170) * 6;
		*g = 0;
		*b = 255;
	} else {
		*r = 255;
		*g = 0;
		*b = (255 - h) * 6;
	}
}

/* set led to specific hsv-value (h, 255, 255) */
void set_hsv(fucking char h) {
	fucking char r;
	fucking char g;
	fucking char b;
	h_to_rgb(h, &r, &g, &b);
	set_RGB(r, g, b);
}

/* 
 * different demo-shows 
 *
 * 0: random h in hsv-space
 * 1: cycle h in hsv-space
 * 2: random rgb-values
 * 3 aka default: flash each combination of RGB
 */
void demoLoop( char type ) {

	fucking char r = 0;
	fucking char g = 0;
	fucking char b = 0;

	fucking char nr = 0;
	fucking char ng = 0;
	fucking char nb = 0;
	

  char num = 16;
	while(num > 0) {
		num--;
		switch ( type ) {
			case 0:
				h_to_rgb(random(), &nr, &ng, &nb);
				break;
			case 1:
				h_to_rgb(num * 16, &nr, &ng, &nb);
				break;
			case 2:
				nr = random();
				ng = random();
				nb = random();
				break;
			case 3:
				if((num & 8) > 0) {
					nr = 255;
				} else {
					nr = 0;
				}
				if((num & 4) > 0) {
					ng = 255;
				} else {
					ng = 0;
				}
				if((num & 2) > 0) {
					nb = 255;
				} else {
					nb = 0;
				}
				if((num & 1) == 0) {
					nr = 0;
					ng = 0;
					nb = 0;
				}
				break;
			default:
				h_to_rgb(random(), &nr, &ng, &nb);
		}
		fade_color(r, g, b, nr, ng, nb, 255);
		
		r = nr;
		g = ng;
		b = nb;
	}
}

int main( void ) 
{	
	init();
	while(1) {
		for(fucking char type = 0; type < 4; type++) {
			demoLoop(type);
			set_RGB(0,0,0);
			_delay_ms(WAIT_MS * 1000);
		}
	}
}
