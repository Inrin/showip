#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "showip.c"

static void reduce_v6_full(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("12341234123412341234123412341234"), "1234:1234:1234:1234:1234:1234:1234:1234");
}

static void reduce_v6_exceeds_32(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("12341234123412341234123412341234ffff"), "1234:1234:1234:1234:1234:1234:1234:1234");
}

static void reduce_v6_less_than_32(void **state) {
	(void) state; /* unused */
	assert_null(reduce_v6(NULL));
	assert_null(reduce_v6("0000000000000000000000000000000"));
}

static void reduce_v6_all_zero(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("00000000000000000000000000000000"), "::");
}

static void reduce_v6_loopback(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("00000000000000000000000000000001"), "::1");
}

static void reduce_v6_middle(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("abcd000000000000000000000000dcba"), "abcd::dcba");
	assert_string_equal(reduce_v6("0bcd000000000000000000000000dcba"), "bcd::dcba");
	assert_string_equal(reduce_v6("abcd0000000000000000000000000cba"), "abcd::cba");
	assert_string_equal(reduce_v6("0bcd0000000000000000000000000cba"), "bcd::cba");
	assert_string_equal(reduce_v6("00cd0000000000000000000000000cba"), "cd::cba");
	assert_string_equal(reduce_v6("0bcd00000000000000000000000000ba"), "bcd::ba");
	assert_string_equal(reduce_v6("00cd00000000000000000000000000ba"), "cd::ba");
	assert_string_equal(reduce_v6("000d00000000000000000000000000ba"), "d::ba");
	assert_string_equal(reduce_v6("00cd000000000000000000000000000a"), "cd::a");
	assert_string_equal(reduce_v6("000d000000000000000000000000000a"), "d::a");
}

static void reduce_v6_right(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("0000000000000000000000000000dcba"), "::dcba");
	assert_string_equal(reduce_v6("00000000000000000000000000000cba"), "::cba");
	assert_string_equal(reduce_v6("000000000000000000000000000000ba"), "::ba");
	assert_string_equal(reduce_v6("0000000000000000000000000000000a"), "::a");
}

static void reduce_v6_left(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("abcd0000000000000000000000000000"), "abcd::");
	assert_string_equal(reduce_v6("0bcd0000000000000000000000000000"), "bcd::");
	assert_string_equal(reduce_v6("00cd0000000000000000000000000000"), "cd::");
	assert_string_equal(reduce_v6("000d0000000000000000000000000000"), "d::");
}

static void reduce_v6_middle_left_one(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("0000abcd00000000000000000000dcba"), "0:abcd::dcba");
	assert_string_equal(reduce_v6("00000bcd00000000000000000000dcba"), "0:bcd::dcba");
	assert_string_equal(reduce_v6("0000abcd000000000000000000000cba"), "0:abcd::cba");
	assert_string_equal(reduce_v6("00000bcd000000000000000000000cba"), "0:bcd::cba");
	assert_string_equal(reduce_v6("000000cd000000000000000000000cba"), "0:cd::cba");
	assert_string_equal(reduce_v6("00000bcd0000000000000000000000ba"), "0:bcd::ba");
	assert_string_equal(reduce_v6("000000cd0000000000000000000000ba"), "0:cd::ba");
	assert_string_equal(reduce_v6("0000000d0000000000000000000000ba"), "0:d::ba");
	assert_string_equal(reduce_v6("000000cd00000000000000000000000a"), "0:cd::a");
	assert_string_equal(reduce_v6("0000000d00000000000000000000000a"), "0:d::a");
}

static void reduce_v6_middle_left_two(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("00000000abcd0000000000000000dcba"), "0:0:abcd::dcba");
	assert_string_equal(reduce_v6("000000000bcd0000000000000000dcba"), "0:0:bcd::dcba");
	assert_string_equal(reduce_v6("00000000abcd00000000000000000cba"), "0:0:abcd::cba");
	assert_string_equal(reduce_v6("000000000bcd00000000000000000cba"), "0:0:bcd::cba");
	assert_string_equal(reduce_v6("0000000000cd00000000000000000cba"), "0:0:cd::cba");
	assert_string_equal(reduce_v6("000000000bcd000000000000000000ba"), "0:0:bcd::ba");
	assert_string_equal(reduce_v6("0000000000cd000000000000000000ba"), "0:0:cd::ba");
	assert_string_equal(reduce_v6("00000000000d000000000000000000ba"), "0:0:d::ba");
	assert_string_equal(reduce_v6("0000000000cd0000000000000000000a"), "0:0:cd::a");
	assert_string_equal(reduce_v6("00000000000d0000000000000000000a"), "0:0:d::a");
}
static void reduce_v6_middle_left_three(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("000000000000abcd000000000000dcba"), "::abcd:0:0:0:dcba");
	assert_string_equal(reduce_v6("0000000000000bcd000000000000dcba"), "::bcd:0:0:0:dcba");
	assert_string_equal(reduce_v6("000000000000abcd0000000000000cba"), "::abcd:0:0:0:cba");
	assert_string_equal(reduce_v6("0000000000000bcd0000000000000cba"), "::bcd:0:0:0:cba");
	assert_string_equal(reduce_v6("00000000000000cd0000000000000cba"), "::cd:0:0:0:cba");
	assert_string_equal(reduce_v6("0000000000000bcd00000000000000ba"), "::bcd:0:0:0:ba");
	assert_string_equal(reduce_v6("00000000000000cd00000000000000ba"), "::cd:0:0:0:ba");
	assert_string_equal(reduce_v6("000000000000000d00000000000000ba"), "::d:0:0:0:ba");
	assert_string_equal(reduce_v6("00000000000000cd000000000000000a"), "::cd:0:0:0:a");
	assert_string_equal(reduce_v6("000000000000000d000000000000000a"), "::d:0:0:0:a");
}

static void reduce_v6_middle_right_one(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("abcd00000000000000000000dcba0000"), "abcd::dcba:0");
	assert_string_equal(reduce_v6("0bcd00000000000000000000dcba0000"), "bcd::dcba:0");
	assert_string_equal(reduce_v6("abcd000000000000000000000cba0000"), "abcd::cba:0");
	assert_string_equal(reduce_v6("0bcd000000000000000000000cba0000"), "bcd::cba:0");
	assert_string_equal(reduce_v6("00cd000000000000000000000cba0000"), "cd::cba:0");
	assert_string_equal(reduce_v6("0bcd0000000000000000000000ba0000"), "bcd::ba:0");
	assert_string_equal(reduce_v6("00cd0000000000000000000000ba0000"), "cd::ba:0");
	assert_string_equal(reduce_v6("000d0000000000000000000000ba0000"), "d::ba:0");
	assert_string_equal(reduce_v6("00cd00000000000000000000000a0000"), "cd::a:0");
	assert_string_equal(reduce_v6("000d00000000000000000000000a0000"), "d::a:0");
}

static void reduce_v6_middle_right_two(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("abcd0000000000000000dcba00000000"), "abcd::dcba:0:0");
	assert_string_equal(reduce_v6("0bcd0000000000000000dcba00000000"), "bcd::dcba:0:0");
	assert_string_equal(reduce_v6("abcd00000000000000000cba00000000"), "abcd::cba:0:0");
	assert_string_equal(reduce_v6("0bcd00000000000000000cba00000000"), "bcd::cba:0:0");
	assert_string_equal(reduce_v6("00cd00000000000000000cba00000000"), "cd::cba:0:0");
	assert_string_equal(reduce_v6("0bcd000000000000000000ba00000000"), "bcd::ba:0:0");
	assert_string_equal(reduce_v6("00cd000000000000000000ba00000000"), "cd::ba:0:0");
	assert_string_equal(reduce_v6("000d000000000000000000ba00000000"), "d::ba:0:0");
	assert_string_equal(reduce_v6("00cd0000000000000000000a00000000"), "cd::a:0:0");
	assert_string_equal(reduce_v6("000d0000000000000000000a00000000"), "d::a:0:0");
}

static void reduce_v6_middle_right_three(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("abcd000000000000dcba000000000000"), "abcd::dcba:0:0:0");
	assert_string_equal(reduce_v6("0bcd000000000000dcba000000000000"), "bcd::dcba:0:0:0");
	assert_string_equal(reduce_v6("abcd0000000000000cba000000000000"), "abcd::cba:0:0:0");
	assert_string_equal(reduce_v6("0bcd0000000000000cba000000000000"), "bcd::cba:0:0:0");
	assert_string_equal(reduce_v6("00cd0000000000000cba000000000000"), "cd::cba:0:0:0");
	assert_string_equal(reduce_v6("0bcd00000000000000ba000000000000"), "bcd::ba:0:0:0");
	assert_string_equal(reduce_v6("00cd00000000000000ba000000000000"), "cd::ba:0:0:0");
	assert_string_equal(reduce_v6("000d00000000000000ba000000000000"), "d::ba:0:0:0");
	assert_string_equal(reduce_v6("00cd000000000000000a000000000000"), "cd::a:0:0:0");
	assert_string_equal(reduce_v6("000d000000000000000a000000000000"), "d::a:0:0:0");
}

static void reduce_v6_leading_zeros(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("00010002000300040005000600070008"), "1:2:3:4:5:6:7:8");
	assert_string_equal(reduce_v6("00110022003300440055006600770088"), "11:22:33:44:55:66:77:88");
	assert_string_equal(reduce_v6("01110222033304440555066607770888"), "111:222:333:444:555:666:777:888");
}

static void reduce_v6_dont_strip(void **state) {
	(void) state; /* unused */
	assert_string_equal(reduce_v6("10002000300040005000600070008000"), "1000:2000:3000:4000:5000:6000:7000:8000");
}

int main(void) {
	const struct CMUnitTest reduce_v6_tests[] = {
		cmocka_unit_test(reduce_v6_full),
		cmocka_unit_test(reduce_v6_exceeds_32),
		cmocka_unit_test(reduce_v6_less_than_32),
		cmocka_unit_test(reduce_v6_all_zero),
		cmocka_unit_test(reduce_v6_loopback),
		cmocka_unit_test(reduce_v6_middle),
		cmocka_unit_test(reduce_v6_left),
		cmocka_unit_test(reduce_v6_right),
		cmocka_unit_test(reduce_v6_middle_left_one),
		cmocka_unit_test(reduce_v6_middle_left_two),
		cmocka_unit_test(reduce_v6_middle_left_three),
		cmocka_unit_test(reduce_v6_middle_right_one),
		cmocka_unit_test(reduce_v6_middle_right_two),
		cmocka_unit_test(reduce_v6_middle_right_three),
		cmocka_unit_test(reduce_v6_leading_zeros),
		cmocka_unit_test(reduce_v6_dont_strip),
	};
	return cmocka_run_group_tests(reduce_v6_tests, NULL, NULL);
}
