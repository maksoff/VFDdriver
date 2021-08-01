/*
 * vfd.h
 *
 *  Created on: Jul 12, 2021
 *      Author: makso
 */

#ifndef INC_VFD_H_
#define INC_VFD_H_

uint16_t get_char(char input);

extern const uint16_t vfd_digits [];
extern const uint16_t vfd_alpha [];
extern const uint16_t vfd_alpha_ru [];
extern const uint16_t vfd_special [];
extern const char vfd_special_char [];

extern const uint8_t DIGITS;
extern const uint8_t ALPHAS;
extern const uint8_t ALPHAR;
extern const uint8_t SPECIAL;


#endif /* INC_VFD_H_ */
