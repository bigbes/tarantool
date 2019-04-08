#ifndef TARANTOOL_LIB_CORE_DECIMAL_H_INCLUDED
#define TARANTOOL_LIB_CORE_DECIMAL_H_INCLUDED
/*
 * Copyright 2019, Tarantool AUTHORS, please see AUTHORS file.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#define TARANTOOL_MAX_DECIMAL_DIGITS 38
#define DECNUMDIGITS TARANTOOL_MAX_DECIMAL_DIGITS
#define DECSTRING_NO_EXPONENT
#include "third_party/decNumber/decNumber.h"

typedef decNumber decimal;

/**
 * @return decimal precision,
 * i.e. the amount of decimal digits in
 * its representation.
 */
uint8_t
decimal_precision(const decimal *dec);

/**
 * @return decimal scale,
 * i.e. the number of decimal digits after
 * the decimal separator.
 */
uint8_t
decimal_scale(const decimal *dec);

/**
 * Initialize a zero decimal number.
 */
decimal *
decimal_zero(decimal *dec);

/**
 * Initialize a decimal with a value from the string.
 *
 * If the number is less, than 10^TARANTOOL_MAX_DECIMAL_DIGITS,
 * but has excess digits in fractional part, it will be rounded.
 *
 * @return NULL if string is invalid or
 * the number is too big (>= 10^TARANTOOL_MAX_DECIMAL_DIGITS)
 */
decimal *
decimal_from_string(decimal *dec, const char *str);

/**
 * Initialize a decimal with an integer value.
 *
 * @return NULL if precicion is insufficient to hold
 * the value or precision/scale are out of bounds.
*/
decimal *
decimal_from_int(decimal *dec, int32_t num);

/** @copydoc decimal_from_int */
decimal *
decimal_from_uint(decimal *dec, uint32_t num);

/**
 * Write the decimal to a string.
 * A string has to be at least dec->digits + 3 bytes in size.
 */
char *
decimal_to_string(const decimal *dec, char *buf);

/**
 * Cast decimal to an integer value. The number will be rounded
 * if it has a fractional part.
 */
int32_t
decimal_to_int(const decimal *dec);

/** @copydoc decimal_to_int */
uint32_t
decimal_to_uint(const decimal *dec);

/**
 * Compare 2 decimal values.
 * @return -1, lhs < rhs,
 *	    0, lhs = rhs,
 *	    1, lhs > rhs
 */
int
decimal_compare(const decimal *lhs, const decimal *rhs);

/**
 * res is set to the absolute value of dec
 * decimal_abs(&a, &a) is allowed.
 */
decimal *
decimal_abs(decimal *res, const decimal *dec);

/*
 * Arithmetic ops: add, subtract, multiply and divide.
 * Return result pointer on success, NULL on an error (overflow).
 */

decimal *
decimal_add(decimal *res, const decimal *lhs, const decimal *rhs);

decimal *
decimal_sub(decimal *res, const decimal *lhs, const decimal *rhs);

decimal *
decimal_mul(decimal *res, const decimal *lhs, const decimal *rhs);

decimal *
decimal_div(decimal *res, const decimal *lhs, const decimal *rhs);

/*
 * log10, ln, pow, exp, sqrt.
 * Calculate the appropriate function and then round the result
 * to the desired precision.
 * Return result pointer on success, NULL on an error (overflow).
 */
decimal *
decimal_log10(decimal *res, const decimal *lhs, uint32_t precision);

decimal *
decimal_ln(decimal *res, const decimal *lhs, uint32_t precision);

decimal *
decimal_pow(decimal *res, const decimal *lhs, const decimal *rhs, uint32_t precision);

decimal *
decimal_exp(decimal *res, const decimal *lhs, uint32_t precision);

decimal *
decimal_sqrt(decimal *res, const decimal *lhs, uint32_t precision);

#endif /* TARANTOOL_LIB_CORE_DECIMAL_H_INCLUDED */
