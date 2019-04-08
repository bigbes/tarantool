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

#include "decimal.h"
#include "third_party/decNumber/decContext.h"
#include <stdlib.h>
#include <assert.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

/** A single context for all the arithmetic operations. */
static decContext decimal_context = {
	/* Maximum precision during operations. */
	TARANTOOL_MAX_DECIMAL_DIGITS,
	/*
	 * Maximum decimal lagarithm of the number.
	 * Allows for precision = TARANTOOL_MAX_DECIMAL_DIGITS
	 */
	TARANTOOL_MAX_DECIMAL_DIGITS - 1,
	/*
	 * Minimal adjusted exponent. The smallest absolute value will be
	 * exp((1 - TARANTOOL_MAX_DECIMAL_DIGITS) - 1) =
	 * exp(-TARANTOOL_MAX_DECIMAL_DIGITS) allowing for scale =
	 * TARANTOOL_MAX_DECIMAL_DIGITS
	 */
	-1,
	/* Rounding mode: .5 rounds away from 0. */
	DEC_ROUND_HALF_UP,
	/* Turn off signalling for failed operations. */
	0,
	/* Status holding occured events. Initially empty. */
	0,
	/* Turn off exponent clamping. */
	0
};

/**
 * Return operation status and clear it for future checks.
 *
 * @return 0 if ok, bitwise or of decNumber errors, if any.
 */
static inline uint32_t
decimal_get_op_status(decContext *context)
{
	uint32_t status = decContextGetStatus(context);
	decContextZeroStatus(&decimal_context);
	/*
	 * Clear warnings. Every value less than 0.1 is
	 * subnormal, DEC_Inexact and DEC_Rounded result
	 * from rounding. DEC_Inexact with DEC_Subnormal
	 * together result in DEC_Underflow. DEC_Clamped
	 * happens after underflow if rounding to zero.
	 */
	return status & ~(uint32_t)(DEC_Inexact | DEC_Rounded | DEC_Underflow |
				    DEC_Subnormal | DEC_Clamped);
}

/**
 * A finalizer for all the operations.
 * Check the operation context status and empty it.
 *
 * @return NULL if finalization failed.
 *         result pointer otherwise.
 */
static inline decimal *
decimal_finalize(decimal *res, decContext *context)
{
	uint32_t status = decimal_get_op_status(context);
	if (status || ! decNumberIsFinite(res)) {
		return NULL;
	}
	return res;
}

uint8_t decimal_precision(const decimal *dec) {
	return dec->exponent <= 0 ? MAX(dec->digits, -dec->exponent) :
				    dec->digits + dec->exponent;
}

uint8_t decimal_scale(const decimal *dec) {
	return dec->exponent < 0 ? -dec->exponent : 0;
}

decimal *
decimal_zero(decimal *dec)
{
	decNumberZero(dec);
	return dec;
}

decimal *
decimal_from_string(decimal *dec, const char *str)
{
	decNumberFromString(dec, str, &decimal_context);
	return decimal_finalize(dec, &decimal_context);
}

decimal *
decimal_from_int(decimal *dec, int32_t num)
{
	decNumberFromInt32(dec, num);
	return decimal_finalize(dec, &decimal_context);
}

decimal *
decimal_from_uint(decimal *dec, uint32_t num)
{
	decNumberFromUInt32(dec, num);
	return decimal_finalize(dec, &decimal_context);
}

char *
decimal_to_string(const decimal *dec, char *buf)
{
	return decNumberToString(dec, buf);
}

int32_t
decimal_to_int(const decimal *dec)
{
	decNumber res;
	decNumberToIntegralValue(&res, dec, &decimal_context);
	return decNumberToInt32(&res, &decimal_context);
}

uint32_t
decimal_to_uint(const decimal *dec)
{
	decNumber res;
	decNumberToIntegralValue(&res, dec, &decimal_context);
	return decNumberToUInt32(&res, &decimal_context);
}

int
decimal_compare(const decimal *lhs, const decimal *rhs)
{
	decNumber res;
	decNumberCompare(&res, lhs, rhs, &decimal_context);
	return decNumberToInt32(&res, &decimal_context);
}

decimal *
decimal_abs(decimal *res, const decimal *dec)
{
	decNumberAbs(res, dec, &decimal_context);
	return res;
}

decimal *
decimal_add(decimal *res, const decimal *lhs, const decimal *rhs)
{
	decNumberAdd(res, lhs, rhs, &decimal_context);

	return decimal_finalize(res, &decimal_context);
}

decimal *
decimal_sub(decimal *res, const decimal *lhs, const decimal *rhs)
{
	decNumberSubtract(res, lhs, rhs, &decimal_context);

	return decimal_finalize(res, &decimal_context);
}

decimal *
decimal_mul(decimal *res, const decimal *lhs, const decimal *rhs)
{
	decNumberMultiply(res, lhs, rhs, &decimal_context);

	return decimal_finalize(res, &decimal_context);
}

decimal *
decimal_div(decimal *res, const decimal *lhs, const decimal *rhs)
{
	decNumberDivide(res, lhs, rhs, &decimal_context);

	return decimal_finalize(res, &decimal_context);
}

decimal *
decimal_log10(decimal *res, const decimal *lhs, uint32_t precision)
{
	if (precision >= TARANTOOL_MAX_DECIMAL_DIGITS)
		return NULL;

	/* A separate context to enforce desired precision. */
	static decContext op_context;
	op_context.digits = precision;
	op_context.emax = op_context.digits - 1;
	op_context.emin = -1;

	decNumberLog10(res, lhs, &op_context);

	return decimal_finalize(res, &op_context);
}

decimal *
decimal_ln(decimal *res, const decimal *lhs, uint32_t precision)
{
	if (precision >= TARANTOOL_MAX_DECIMAL_DIGITS)
		return NULL;

	/* A separate context to enforce desired precision. */
	static decContext op_context;
	op_context.digits = precision;
	op_context.emax = op_context.digits - 1;
	op_context.emin = -1;

	decNumberLn(res, lhs, &op_context);

	return decimal_finalize(res, &op_context);
}

decimal *
decimal_pow(decimal *res, const decimal *lhs, const decimal *rhs, uint32_t precision)
{
	if (precision >= TARANTOOL_MAX_DECIMAL_DIGITS)
		return NULL;

	/* A separate context to enforce desired precision. */
	static decContext op_context;
	op_context.digits = precision;
	op_context.emax = op_context.digits - 1;
	op_context.emin = -1;

	decNumberPower(res, lhs, rhs, &op_context);

	return decimal_finalize(res, &op_context);
}

decimal *
decimal_exp(decimal *res, const decimal *lhs, uint32_t precision)
{
	if (precision >= TARANTOOL_MAX_DECIMAL_DIGITS)
		return NULL;

	/* A separate context to enforce desired precision. */
	static decContext op_context;
	op_context.digits = precision;
	op_context.emax = op_context.digits - 1;
	op_context.emin = -1;

	decNumberExp(res, lhs, &op_context);

	return decimal_finalize(res, &op_context);
}

decimal *
decimal_sqrt(decimal *res, const decimal *lhs, uint32_t precision)
{
	if (precision >= TARANTOOL_MAX_DECIMAL_DIGITS)
		return NULL;

	/* A separate context to enforce desired precision. */
	static decContext op_context;
	op_context.digits = precision;
	op_context.emax = op_context.digits - 1;
	op_context.emin = -1;

	decNumberSquareRoot(res, lhs, &op_context);

	return decimal_finalize(res, &op_context);
}
