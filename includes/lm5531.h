/*
 * lm5531.h
 *
 *  Created on: Jul 16, 2018
 *      Author: karthik
 */

#ifndef INCLUDES_LM5531_H_
#define INCLUDES_LM5531_H_

#include "stdint.h"

int32_t lm5331_conv_mv_db_out(uint32_t in_mv, uint32_t out_mv);

int32_t lm5331_conv_mv_db_in(uint32_t in_mv, uint32_t out_mv);


#endif /* INCLUDES_LM5531_H_ */
