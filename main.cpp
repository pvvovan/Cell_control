#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "ring_buffer.h"
#include "scope_lock.h"
#include "sw_uart.h"
#include "uart.h"

static void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	/* Wait for completion of previous write */
	while (EECR & (1 << EEPE)) {
		;
	}
	/* Set up address and Data Registers */
	EEAR = uiAddress;
	EEDR = ucData;

	scope_lock sl{};
	/* Write logical one to EEMPE */
	EECR |= (1<<EEMPE);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);
}

static unsigned char EEPROM_read(unsigned int uiAddress)
{
	/* Wait for completion of previous write */
	while (EECR & (1 << EEPE)) {
		;
	}

	scope_lock sl{};
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from Data Register */
	return EEDR;
}

class storage {
	long long counter{};
	public:
		void print() {

		}
};

int main() {
	uart::init(9600, 20, 20);
	uart::start();

	sw_uart::init(115200, &DDRB, &PORTB, PORTB5);

	for ( ; ; ) {
		_delay_ms(500);
		PORTB ^= 1 << PORTB5;
		unsigned char c = EEPROM_read(0);
		c++;
		char str[20] = { '\0', };
		snprintf(str, sizeof (str), "%d\r\n", static_cast<int>(c));
		uart::print(str);
		EEPROM_write(0, c);
	}

	unsigned char data = '0';
	ring_buffer buf(64);

	for (;;) {
		sw_uart::print("Hallo from sw uart\r\n");

		uart::print("Hello\r\n");
		uart::write(&data, 1);
		if (++data > '9') {
			data = '0';
		}

		buf.write(data);
	}

	return 0;
}
