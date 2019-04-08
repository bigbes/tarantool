#include "unit.h"
#include "decimal.h"
#include <limits.h>
#include <string.h>

int
main(void)
{
	plan(50);

	char buf[TARANTOOL_MAX_DECIMAL_DIGITS + 3];
	char buf2[TARANTOOL_MAX_DECIMAL_DIGITS + 3];
	char *b = "2.718281828";

	decimal s;
	decimal *ret;

	ret = decimal_from_string(&s, b);
	isnt(ret, NULL, "Basic construction from string.");
	decimal_to_string(&s, buf);
	is(strcmp(buf, b), 0, "Correct construction and to_string conversion.");

	ret = decimal_from_int(&s, INT_MAX);
	decimal_to_string(&s, buf);
	sprintf(buf2, "%d", INT_MAX);
	is(strcmp(buf, buf2), 0, "Correct construction from INT_MAX.");
	is(decimal_to_int(&s), INT_MAX, "Simple conversion back to int INT_MAX");

	ret = decimal_from_int(&s, INT_MIN);
	decimal_to_string(&s, buf);
	sprintf(buf2, "%d", INT_MIN);
	is(strcmp(buf, buf2), 0, "Correct construction from INT_MIN.");
	is(decimal_to_int(&s), INT_MIN, "Simple conversion bact to int INT_MIN");

	char *up1 = "2.5";
	char *down1 = "2.49";
	decimal_from_string(&s, up1);
	is(decimal_to_int(&s), 3, ".5 Rounds up");
	decimal_from_string(&s, down1);
	is(decimal_to_int(&s), 2, ".49 Rounds down");

	char *l = "1.23456789123456789123456789123456789123";
	ret = decimal_from_string(&s, l);
	isnt(ret, NULL, "Precision too high. Rounding happens.");
	ok(decimal_precision(&s) == 38 && decimal_scale(&s) == 37 , "Construction is correct.");
	char *ll = "123456789123456789123456789123456789123";
	ret = decimal_from_string(&s, ll);
	is(ret, NULL, "Precision too high and scale = 0. Cannot round");

	/* 38 digits. */
	char *long_str = "0.0000000000000000000000000000000000001";
	ret = decimal_from_string(&s, long_str);
	isnt(ret, NULL, "Construncting the smallest possible number from string");
	decimal_to_string(&s, buf);
	is(strcmp(buf, long_str), 0, "Correct representation of smallest possible number");

	/* Comparsions. */
	char *max_str = "3.11", *min_str = "3.0999";
	decimal max, min;
	decimal_from_string(&max, max_str);
	decimal_from_string(&min, min_str);
	is(decimal_compare(&max, &min), 1, "max > min");
	is(decimal_compare(&min, &max), -1, "min < max");
	is(decimal_compare(&max, &max), 0, "max == max");

	ret = decimal_from_string(&s, "-3.456");
	isnt(ret, NULL, "Construction from negative numbers");
	decimal_to_string(&s, buf);
	is(strcmp(buf, "-3.456"), 0, "Correct construction for negatives");
	ret = decimal_abs(&s, &s);
	isnt(ret, NULL, "Abs");
	decimal_to_string(&s, buf);
	is(strcmp(buf, "3.456"), 0, "Correct abs");

	/* Arithmetic ops. */
	decimal d, check;
	ret = decimal_from_string(&s, b);
	ret = decimal_from_string(&d, "1.25");
	sprintf(buf2, "%.9f", 1.25 + 2.718281828);
	ret = decimal_add(&d, &d, &s);
	isnt(ret, NULL, "Simple addition");
	decimal_to_string(&d, buf);
	is(strcmp(buf, buf2), 0, "Simple addition is correct");

	ret = decimal_sub(&d, &d, &s);
	isnt(ret, NULL, "Simple subtraction");
	decimal_from_string(&check, "1.25");
	is(decimal_compare(&d, &check), 0, "Simple subtraction is correct");

	decimal_from_int(&s, 4);
	ret = decimal_mul(&s, &s, &d);
	isnt(ret, NULL, "Simple multiplication");
	decimal_from_string(&check, "5.0");
	is(decimal_compare(&s, &check), 0 , "Simple multiplication is correct");

	ret = decimal_div(&s, &s, &d);
	isnt(ret, NULL, "Simple division");
	decimal_from_string(&check, "4.0");
	is(decimal_compare(&s, &check), 0, "Simple division is correct");

	/* Math. */
	ret = decimal_from_string(&s, "40.96");
	ret = decimal_from_string(&check, "6.4");
	ret = decimal_sqrt(&s, &s, 2);
	isnt(ret, NULL, "sqrt");
	is(decimal_compare(&s, &check), 0, "sqrt is correct");

	ret = decimal_from_string(&s, "40.96");
	ret = decimal_from_string(&d, "0.5");
	ret = decimal_pow(&s, &s, &d, 2);
	isnt(ret, NULL, "pow");
	is(decimal_compare(&s, &check), 0, "pow is correct");

	ret = decimal_from_string(&s, "3.000");
	ret = decimal_exp(&d, &s, 4);
	isnt(ret, NULL, "exp");

	ret = decimal_from_string(&check, "20.09");
	is(decimal_compare(&d, &check), 0, "exp is correct")
	ret = decimal_ln(&d, &d, 4);
	isnt(ret, NULL, "ln");
	/*
	 * ln is wrong by 1 unit in last position.
	 * (3.001 instead of 3.000, 3.0001 instead of 3.0000 and so on)
	 */
	decimal_from_string(&s, "3.001");
	is(decimal_compare(&d, &s), 0, "ln is correct");

				      /* 10^3.5 */
	ret = decimal_from_string(&s, "3162.27766");
	ret = decimal_log10(&d, &s, 2);
	isnt(ret, NULL, "log10");
	ret = decimal_from_string(&check, "3.5");
	is(decimal_compare(&d, &check), 0, "log10 is correct");

	/* Advanced test. */
	/* 38 digits. */
	char *bignum = "33.333333333333333333333333333333333333";
	char *test =   "133.33333333333333333333333333333333333";
	ret = decimal_from_string(&s, bignum);
	ret = decimal_from_int(&d, 4);
	ret = decimal_mul(&s, &s, &d);
	isnt(ret, NULL, "Rounding when more than TARANTOOL_MAX_DECIMAL_DIGITS digits");
	ret = decimal_from_string(&check, test);
	is(decimal_compare(&s, &check), 0, "Rounding is correct");
	is(decimal_precision(&s), 38, "Correct precision");
	is(decimal_scale(&s), 35, "Correct scale");

	char *small = "0.000000000000000000000000001";
	ret = decimal_from_string(&s, small);
	ret = decimal_mul(&s, &s, &s);
	isnt(ret, NULL, "Rounding too small number to zero");
	ret = decimal_from_int(&check, 0);
	is(decimal_compare(&s, &check), 0, "Rounding is correct");
	is(decimal_precision(&s), 38, "Correct precision");
	is(decimal_scale(&s), 38, "Correct scale");

	decimal_from_string(&s, small);
	decimal_from_string(&d, "10000000000000000000");
	ret = decimal_div(&s, &s, &d);
	isnt(ret, NULL, "Rounding too small number to zero");
	is(decimal_compare(&s, &check), 0, "Rounding is correct");
	decimal_to_string(&s, buf);
	is(decimal_precision(&s), 38, "Correct precision");
	is(decimal_scale(&s), 38, "Correct scale");

	check_plan();
}
