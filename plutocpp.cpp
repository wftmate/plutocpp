//============================================================================
// Name        : libiiocpp.cpp
// Author      : Turner Titties
// Version     :
// Copyright   : Your copyright notice
// Description : PlutoSDR Example code from the link below
// URL         : https://wiki.analog.com/university/tools/pluto/controlling_the_transceiver_and_transferring_data
//============================================================================

#include <stdio.h>
#include <iostream>
#include "iio.h"

// -- Print I/Q Data --------------------------------------------------------
void print(int16_t i, int16_t q){
	//std::cout << "i = " << i << " | " << "q = " << q << std::endl;

	// space = include sign
	// 4 = 3 digits + sign
	// i = integer
	printf("i = % 4i", i);
	printf(" | q = % 4i\n", q);
}

// -- Receiving data ----------------------------------------------------
//
//    Get the RX capture device structure
//    Get the IQ input channels
//    Enable I and Q channel
//    Create the RX buffer
//    Fill the buffer
//    Process samples

//this is a change

int receive(struct iio_context *ctx)
{
	printf("receive reached\n");
	struct iio_device *dev;
	struct iio_channel *rx0_i, *rx0_q;
	struct iio_buffer *rxbuf;

	dev = iio_context_find_device(ctx, "cf-ad9361-lpc");

	rx0_i = iio_device_find_channel(dev, "voltage0", 0);
	rx0_q = iio_device_find_channel(dev, "voltage1", 0);

	iio_channel_enable(rx0_i);
	iio_channel_enable(rx0_q);

	rxbuf = iio_device_create_buffer(dev, 4096, false);
	if (!rxbuf) {
		perror("Could not create RX buffer");
		//shutdown();
	}

	bool track = false;
	while (true) {

		if(track == false){
			printf("while loop reached\n");
			track = true;
		}
		void *p_dat, *p_end, *t_dat;
		ptrdiff_t p_inc;

		iio_buffer_refill(rxbuf);

		p_inc = iio_buffer_step(rxbuf);
		p_end = iio_buffer_end(rxbuf);

		for (p_dat = iio_buffer_first(rxbuf, rx0_i); p_dat < p_end; p_dat += p_inc, t_dat += p_inc) {
			const int16_t i = ((int16_t*)p_dat)[0]; // Real (I)
			const int16_t q = ((int16_t*)p_dat)[1]; // Imag (Q)

			/* Process here */
			print(i, q);
		}
	}

	iio_buffer_destroy(rxbuf);

	printf("receive finished");
}

// -- Controlling the Transceiver -----------------------------------------------------------------------
// The code snippet below is a minimalistic example without error checking. It shows how to control the
// AD936x transceiver via a remote connection.
//
//    Create IIO IP Network context. Instead of ip:xxx.xxx.xxx.xxx it'll also accept usb:XX.XX.X
//    Get the AD936x PHY device structure
//    Set the TX LO frequency (see AD9361 device driver documentation)
//    Set RX baseband rate

int main (int argc, char **argv)
{
	std::cout << "pluto has compiled" << std::endl;
	struct iio_context *ctx;
	struct iio_device *phy;

	ctx = iio_create_context_from_uri("ip:192.168.2.1");

	phy = iio_context_find_device(ctx, "ad9361-phy");

	iio_channel_attr_write_longlong(
		iio_device_find_channel(phy, "altvoltage0", true),
		"frequency",
		2400000000); /* RX LO frequency 2.4GHz */

	iio_channel_attr_write_longlong(
		iio_device_find_channel(phy, "voltage0", false),
		"sampling_frequency",
		5000000); /* RX baseband rate 5 MSPS */

	printf("about to call receive\n");
	receive(ctx);

	iio_context_destroy(ctx);
	printf("context destroyed");

	return 0;
}
