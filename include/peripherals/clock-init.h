/*
 * clock-init.h
 *
 *  Created on: 07-May-2014
 *      Author: prithvi
 */

#ifndef CLOCK_INIT_H_
#define CLOCK_INIT_H_

#define SRC_LFCLK 					CLOCK_LFCLKSRC_SRC_Synth

#define HFCLK_FREQUENCY		  		(16000000UL)			/**< LFCLK frequency in Hertz, constant. */
#define LFCLK_FREQUENCY           	(32768UL)              /**< LFCLK frequency in Hertz, constant. */

void lfclk_init(void);
void lfclk_deinit(void);
void hfclk_xtal_init(void);
void hfclk_xtal_deinit(void);
#endif /* CLOCK_INIT_H_ */
