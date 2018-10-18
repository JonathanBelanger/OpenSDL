/*
 * Copyright (C) Jonathan D. Belanger 2018.
 *
 *  OpenSDL is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  OpenSDL is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Description:
 *
 *  This file, struct_test.c, TODO
 *
 * Revision History:
 *
 *  V01.000	Oct 18, 2018	Jonathan D. Belanger
 *  Initially written.
 */
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <complex.h>

struct my_unpacked_struct
{
    char c;
    int i;
} test_1;

struct __attribute__ ((__packed__)) my_packed_struct
{
    char c;
    int  i;
    struct my_unpacked_struct s;
} test_2;

struct S
{
    short f[3];
} __attribute__ ((aligned (8))) test_3;

typedef int more_aligned_int1 __attribute__ ((aligned (4)));
typedef int more_aligned_int2 __attribute__ ((aligned (8)));

more_aligned_int1 test_4a;
more_aligned_int2 test_4b;

struct __attribute__ ((aligned (16))) r_test_8i
{
    int16_t w_opcode;
    union
    {
	struct
	{
	    int v_resolved : 1;
	    int v_psv : 1;
	    int v_mark1 : 1;
	    int v_spare_bits : 5;
	} r_pli_bits;
	struct
	{
	    int v_value_variable_size : 1;
	    int v_psv : 1;
	    int v_expanded : 1;
	    int v_resolved : 1;
	    int v_reduced : 1;
	    int v_spare_bits : 3;
	} r_c_bits;
    } r_lang_bits;
} test_12;

struct nod$_node
{
    void *nod$a_flink;
    void *nod$a_blink;
    void *nod$a_paren;
    void *nod$a_child;
    void *nod$a_comment;
    union
    {
	int nod$l_typeinfo;
	void *nod$a_typeinfo;
    } nod$r_info;
    union
    {
	int nod$l_typeinfo2;
	void *nod$a_typeinfo2;
	void *nod$a_symtab;
    } nod$r_info2;
    char nod$b_type;
    char nod$b_boundary;
    short nod$w_datatype;
    int nod$l_offset;
    union
    {
	int nod$l_fldsiz;
	void *nod$a_fldsiz;
    } nod$r_fldsiz;
    union
    {
	int nod$l_hidim;
	void *hidim;
    } mod$r_hidim;
    union
    {
	int nod$l_lodim;
	void *nod$a_lodim;
    } nof$r_lodim;
    union
    {
	int nod$l_initial;
	void *nod$a_initial;
    } nod$r_initial;
    int nod$l_srcline;
    int nod$l_nodeid;
    union
    {
	unsigned int nod$l_flags;
	int nod$l_fixflags;
	struct
	{
	    unsigned int nod$v_value : 1;
	    unsigned int nod$v_mask : 1;
	    unsigned int nod$v_unsigned : 1;
	    unsigned int nod$v_common : 1;
	    unsigned int nod$v_global : 1;
	    unsigned int nod$v_varying : 1;
	    unsigned int nod$v_variable : 1;
	    unsigned int nod$v_based : 1;
	    unsigned int nod$v_desc : 1;
	    unsigned int nod$v_dimen : 1;	/* is dimensioned */
	    unsigned int nod$v_in : 1;
	    unsigned int nod$v_out : 1;
	    unsigned int nod$v_bottom : 1;
	    unsigned int nod$v_bound : 1;
	    unsigned int nod$v_ref : 1;
	    unsigned int nod$v_userfill : 1;
	    unsigned int nod$v_alias : 1;
	    unsigned int nod$v_default : 1;	/* DEFAULT */
	    unsigned int nod$v_vardim : 1;	/* "DIMENSION *" */
	    unsigned int nod$v_link : 1;
	    unsigned int nod$v_optional : 1;
	    unsigned int nod$v_signed : 1;
	    unsigned int nod$v_fixed_fldsiz : 1;
	    unsigned int nod$v_generated : 1;
	    unsigned int nod$v_module : 1;
	    unsigned int nod$v_list : 1;
	    unsigned int nod$v_rtl_str_desc : 1;
	    unsigned int nod$v_complex : 1;
	    unsigned int nod$v_typedef : 1;
	    unsigned int nod$v_declared : 1;
	    unsigned int nod$v_forward : 1;
	    unsigned int nod$v_align : 1;
	} nod$r_flagstruc;
    } nod$r_flagunion;
    union
    {
	unsigned int nod$l_flags2;
	int nod$l_fixflags2;
	struct
	{
	    unsigned int nod$v_has_object : 1;
	    unsigned int nod$v_offset_fixed : 1;
	    unsigned int nod$v_length : 1;
	    unsigned int nod$v_hidim : 1;
	    unsigned int nod$v_lodim : 1;
	    unsigned int nod$v_initial : 1;
	    unsigned int nod$v_base_align : 1;
	    unsigned int nod$v_offset_ref : 1;
	} nod$r_flags2struc;
    } nod$r_flags2union;
    struct
    {
	short string_length;
	char string_text[34];
    } nod$t_naked;
    struct
    {
	short string_length;
	char string_text[34];
    } nod$t_name;
    struct
    {
	short string_length;
	char string_text[34];
    } nod$t_return_name;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_prefix;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_marker;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_tag;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_typename;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_maskstr;
};

struct __attribute__ ((__packed__))  nod$_node_packed
{
    void *nod$a_flink;
    void *nod$a_blink;
    void *nod$a_paren;
    void *nod$a_child;
    void *nod$a_comment;
    union
    {
	int nod$l_typeinfo;
	void *nod$a_typeinfo;
    } nod$r_info;
    union
    {
	int nod$l_typeinfo2;
	void *nod$a_typeinfo2;
	void *nod$a_symtab;
    } nod$r_info2;
    char nod$b_type;
    char nod$b_boundary;
    short nod$w_datatype;
    int nod$l_offset;
    union
    {
	int nod$l_fldsiz;
	void *nod$a_fldsiz;
    } nod$r_fldsiz;
    union
    {
	int nod$l_hidim;
	void *hidim;
    } mod$r_hidim;
    union
    {
	int nod$l_lodim;
	void *nod$a_lodim;
    } nof$r_lodim;
    union
    {
	int nod$l_initial;
	void *nod$a_initial;
    } nod$r_initial;
    int nod$l_srcline;
    int nod$l_nodeid;
    union
    {
	unsigned int nod$l_flags;
	int nod$l_fixflags;
	struct
	{
	    unsigned int nod$v_value : 1;
	    unsigned int nod$v_mask : 1;
	    unsigned int nod$v_unsigned : 1;
	    unsigned int nod$v_common : 1;
	    unsigned int nod$v_global : 1;
	    unsigned int nod$v_varying : 1;
	    unsigned int nod$v_variable : 1;
	    unsigned int nod$v_based : 1;
	    unsigned int nod$v_desc : 1;
	    unsigned int nod$v_dimen : 1;	/* is dimensioned */
	    unsigned int nod$v_in : 1;
	    unsigned int nod$v_out : 1;
	    unsigned int nod$v_bottom : 1;
	    unsigned int nod$v_bound : 1;
	    unsigned int nod$v_ref : 1;
	    unsigned int nod$v_userfill : 1;
	    unsigned int nod$v_alias : 1;
	    unsigned int nod$v_default : 1;	/* DEFAULT */
	    unsigned int nod$v_vardim : 1;	/* "DIMENSION *" */
	    unsigned int nod$v_link : 1;
	    unsigned int nod$v_optional : 1;
	    unsigned int nod$v_signed : 1;
	    unsigned int nod$v_fixed_fldsiz : 1;
	    unsigned int nod$v_generated : 1;
	    unsigned int nod$v_module : 1;
	    unsigned int nod$v_list : 1;
	    unsigned int nod$v_rtl_str_desc : 1;
	    unsigned int nod$v_complex : 1;
	    unsigned int nod$v_typedef : 1;
	    unsigned int nod$v_declared : 1;
	    unsigned int nod$v_forward : 1;
	    unsigned int nod$v_align : 1;
	} nod$r_flagstruc;
    } nod$r_flagunion;
    union
    {
	unsigned int nod$l_flags2;
	int nod$l_fixflags2;
	struct
	{
	    unsigned int nod$v_has_object : 1;
	    unsigned int nod$v_offset_fixed : 1;
	    unsigned int nod$v_length : 1;
	    unsigned int nod$v_hidim : 1;
	    unsigned int nod$v_lodim : 1;
	    unsigned int nod$v_initial : 1;
	    unsigned int nod$v_base_align : 1;
	    unsigned int nod$v_offset_ref : 1;
	} nod$r_flags2struc;
    } nod$r_flags2union;
    struct
    {
	short string_length;
	char string_text[34];
    } nod$t_naked;
    struct
    {
	short string_length;
	char string_text[34];
    } nod$t_name;
    struct
    {
	short string_length;
	char string_text[34];
    } nod$t_return_name;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_prefix;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_marker;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_tag;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_typename;
    struct
    {
	short string_length;
	char string_text[32];
    } nod$t_maskstr;
};

typedef struct nod$_node _nothing;
typedef struct nod$_node_packed _packed;
typedef struct nod$_node _byte __attribute__ ((aligned (1)));
typedef struct nod$_node _word __attribute__ ((aligned (2)));
typedef struct nod$_node _long __attribute__ ((aligned (4)));
typedef struct nod$_node _quad __attribute__ ((aligned (8)));
typedef struct nod$_node _octa __attribute__ ((aligned (16)));

_nothing test_5;
_packed test_6;
_byte test_7;
_word test_8;
_long test_9;
_quad test_10;
_octa test_11;

char	*testCase[12] =
{
     "Base",
     "Last",
     "Size",
     "flagunion",
     "flags",
     "fixflags",
     "flagstruct",
     "flags2union",
     "flags2",
     "fixflags2",
     "flags2struct",
     "naked"
};

int64_t	testResults[7][12];
int	ii, jj;

int main(int argc, char **argv)
{
    int	retVal = 0;

    for (ii = 0; ii < 7; ii++)
	for (jj = 0; jj < 12; jj++)
	    testResults[ii][jj] = 0;

    /*
     * Test 1:
     */
    printf("\nTest 1: No attribute specified.\n");
    printf("\tBase address: %ld (0x%lx)\n", (int64_t) &test_1, (int64_t) &test_1);
    printf("\tSize: %ld (0x%lx)\n", sizeof(test_1), sizeof(test_1));
    printf("\tOffset to 'c': %ld (0x%lx)\n", ((int64_t) &test_1.c - (int64_t) &test_1), ((int64_t) &test_1.c - (int64_t) &test_1));
    printf("\tOffset to 'i': %ld (0x%lx)\n", ((int64_t) &test_1.i - (int64_t) &test_1), ((int64_t) &test_1.i - (int64_t) &test_1));

    /*
     * Test 2:
     */
    printf("\nTest 2: Packed and unpacked specified.\n");
    printf("\tBase address: %ld (0x%lx)\n", (int64_t) &test_2, (int64_t) &test_2);
    printf("\tSize: %ld (0x%lx)\n", sizeof(test_2), sizeof(test_2));
    printf("\tOffset to 'c': %ld (0x%lx)\n", ((int64_t) &test_2.c - (int64_t) &test_2), ((int64_t) &test_2.c - (int64_t) &test_2));
    printf("\tOffset to 'i': %ld (0x%lx)\n", ((int64_t) &test_2.i - (int64_t) &test_2), ((int64_t) &test_2.i - (int64_t) &test_2));
    printf("\tOffset to 's.c': %ld (0x%lx)\n", ((int64_t) &test_2.s.c - (int64_t) &test_2), ((int64_t) &test_2.s.c - (int64_t) &test_2));
    printf("\tOffset to 's.i': %ld (0x%lx)\n", ((int64_t) &test_2.s.i - (int64_t) &test_2), ((int64_t) &test_2.s.i - (int64_t) &test_2));

    /*
     * Test 3:
     */
    printf("\nTest 3: aligned(8) specified.\n");
    printf("\tBase address: %ld (0x%lx)\n", (int64_t) &test_3, (int64_t) &test_3);
    printf("\tSize: %ld (0x%lx)\n", sizeof(test_3), sizeof(test_3));
    printf("\tOffset to 'f[0]': %ld (0x%lx)\n", ((int64_t) &test_3.f[0] - (int64_t) &test_3), ((int64_t) &test_3.f[0] - (int64_t) &test_3));
    printf("\tOffset to 'f[1]': %ld (0x%lx)\n", ((int64_t) &test_3.f[1] - (int64_t) &test_3), ((int64_t) &test_3.f[1] - (int64_t) &test_3));
    printf("\tOffset to 'f[2]': %ld (0x%lx)\n", ((int64_t) &test_3.f[2] - (int64_t) &test_3), ((int64_t) &test_3.f[2] - (int64_t) &test_3));

    /*
     * Test 4:
     */
    printf("\nTest 4: Int aligned(4) and aligned(8) specified.\n");
    printf("\tBase address aligned(4): %ld (0x%lx)\n", (int64_t) &test_4a, (int64_t) &test_4a);
    printf("\tSize: %ld (0x%lx)\n", sizeof(test_4a), sizeof(test_4a));
    printf("\tBase address aligned(8): %ld (0x%lx)\n", (int64_t) &test_4b, (int64_t) &test_4b);
    printf("\tSize: %ld (0x%lx)\n", sizeof(test_4b), sizeof(test_4b));
    printf("\tOffset from 4a to 4b: %ld\n", ((int64_t) &test_4b - (int64_t) &test_4a));

    /*
     * Test 12:
     */
    printf("\nTest 12: No attribute specified.\n");
    printf("\tBase address: %ld (0x%lx)\n", (int64_t) &test_12, (int64_t) &test_12);
    printf("\tSize: %ld (0x%lx)\n", sizeof(test_12), sizeof(test_12));
    printf("\tOffset to 'w_opcode': %ld (0x%lx)\n", ((int64_t) &test_12.w_opcode - (int64_t) &test_12), ((int64_t) &test_12.w_opcode - (int64_t) &test_12));
    printf("\tOffset to 'r_lang_bits': %ld (0x%lx)\n", ((int64_t) &test_12.r_lang_bits - (int64_t) &test_12), ((int64_t) &test_12.r_lang_bits - (int64_t) &test_12));
    printf("\tOffset to 'r_pli_bits': %ld (0x%lx)\n", ((int64_t) &test_12.r_lang_bits.r_pli_bits - (int64_t) &test_12), ((int64_t) &test_12.r_lang_bits.r_pli_bits - (int64_t) &test_12));
    printf("\tOffset to 'r_c_bits': %ld (0x%lx)\n", ((int64_t) &test_12.r_lang_bits.r_c_bits - (int64_t) &test_12), ((int64_t) &test_12.r_lang_bits.r_c_bits - (int64_t) &test_12));

    /*
     * Test 5:
     */
    ii = 0;
    printf("\nTest 5: No attribute specified.\n");
    testResults[ii][0] = (int64_t) &test_5;
    testResults[ii][1] = ((int64_t) &test_5.nod$t_maskstr - testResults[ii][0]);
    testResults[ii][2] = sizeof(test_5);
    testResults[ii][3] = ((int64_t) &test_5.nod$r_flagunion - testResults[ii][0]);
    testResults[ii][4] = ((int64_t) &test_5.nod$r_flagunion.nod$l_flags - testResults[ii][0]);
    testResults[ii][5] = ((int64_t) &test_5.nod$r_flagunion.nod$l_fixflags - testResults[ii][0]);
    testResults[ii][6] = ((int64_t) &test_5.nod$r_flagunion.nod$r_flagstruc - testResults[ii][0]);
    testResults[ii][7] = ((int64_t) &test_5.nod$r_flags2union - testResults[ii][0]);
    testResults[ii][8] = ((int64_t) &test_5.nod$r_flags2union.nod$l_flags2 - testResults[ii][0]);
    testResults[ii][9] = ((int64_t) &test_5.nod$r_flags2union.nod$l_fixflags2 - testResults[ii][0]);
    testResults[ii][10] = ((int64_t) &test_5.nod$r_flags2union.nod$r_flags2struc - testResults[ii][0]);
    testResults[ii][11] = ((int64_t) &test_5.nod$t_naked - testResults[ii][0]);
    printf("\tBase address: %ld (0x%lx)\n", testResults[ii][0], testResults[ii][0]);
    printf("\tLast offset: %ld (0x%lx)\n", testResults[ii][1], testResults[ii][1]);
    printf("\tSize: %ld (0x%lx)\n", testResults[ii][2], testResults[ii][2]);
    printf("\t'nod$r_flagunion' offset: %ld (0x%lx)\n", testResults[ii][3], testResults[ii][3]);
    printf("\t'nod$r_flagunion.nod$l_flags' offset: %ld (0x%lx)\n", testResults[ii][4], testResults[ii][4]);
    printf("\t'nod$r_flagunion.nod$l_fixflags' offset: %ld (0x%lx)\n", testResults[ii][5], testResults[ii][5]);
    printf("\t'nod$r_flagunion.nod$r_flagstuct' offset: %ld (0x%lx)\n", testResults[ii][6], testResults[ii][6]);
    printf("\t'nod$r_flags2union' offset: %ld (0x%lx)\n", testResults[ii][7], testResults[ii][7]);
    printf("\t'nod$r_flags2union.nod$l_flags2' offset: %ld (0x%lx)\n", testResults[ii][8], testResults[ii][8]);
    printf("\t'nod$r_flags2union.nod$l_fixflags2' offset: %ld (0x%lx)\n", testResults[ii][9], testResults[ii][9]);
    printf("\t'nod$r_flags2union.nod$r_flagstuct' offset: %ld (0x%lx)\n", testResults[ii][10], testResults[ii][10]);
    printf("\t'nod$t_naked' offset: %ld (0x%lx)\n", testResults[ii][11], testResults[ii][11]);

    /*
     * Test 6:
     */
    ii++;
    printf("\nTest 6: Packed specified.\n");
    testResults[ii][0] = (int64_t) &test_6;
    testResults[ii][1] = ((int64_t) &test_6.nod$t_maskstr - testResults[ii][0]);
    testResults[ii][2] = sizeof(test_6);
    testResults[ii][3] = ((int64_t) &test_6.nod$r_flagunion - testResults[ii][0]);
    testResults[ii][4] = ((int64_t) &test_6.nod$r_flagunion.nod$l_flags - testResults[ii][0]);
    testResults[ii][5] = ((int64_t) &test_6.nod$r_flagunion.nod$l_fixflags - testResults[ii][0]);
    testResults[ii][6] = ((int64_t) &test_6.nod$r_flagunion.nod$r_flagstruc - testResults[ii][0]);
    testResults[ii][7] = ((int64_t) &test_6.nod$r_flags2union - testResults[ii][0]);
    testResults[ii][8] = ((int64_t) &test_6.nod$r_flags2union.nod$l_flags2 - testResults[ii][0]);
    testResults[ii][9] = ((int64_t) &test_6.nod$r_flags2union.nod$l_fixflags2 - testResults[ii][0]);
    testResults[ii][10] = ((int64_t) &test_6.nod$r_flags2union.nod$r_flags2struc - testResults[ii][0]);
    testResults[ii][11] = ((int64_t) &test_6.nod$t_naked - testResults[ii][0]);
    printf("\tBase address: %ld (0x%lx)\n", testResults[ii][0], testResults[ii][0]);
    printf("\tLast offset: %ld (0x%lx)\n", testResults[ii][1], testResults[ii][1]);
    printf("\tSize: %ld (0x%lx)\n", testResults[ii][2], testResults[ii][2]);
    printf("\t'nod$r_flagunion' offset: %ld (0x%lx)\n", testResults[ii][3], testResults[ii][3]);
    printf("\t'nod$r_flagunion.nod$l_flags' offset: %ld (0x%lx)\n", testResults[ii][4], testResults[ii][4]);
    printf("\t'nod$r_flagunion.nod$l_fixflags' offset: %ld (0x%lx)\n", testResults[ii][5], testResults[ii][5]);
    printf("\t'nod$r_flagunion.nod$r_flagstuct' offset: %ld (0x%lx)\n", testResults[ii][6], testResults[ii][6]);
    printf("\t'nod$r_flags2union' offset: %ld (0x%lx)\n", testResults[ii][7], testResults[ii][7]);
    printf("\t'nod$r_flags2union.nod$l_flags2' offset: %ld (0x%lx)\n", testResults[ii][8], testResults[ii][8]);
    printf("\t'nod$r_flags2union.nod$l_fixflags2' offset: %ld (0x%lx)\n", testResults[ii][9], testResults[ii][9]);
    printf("\t'nod$r_flags2union.nod$r_flagstuct' offset: %ld (0x%lx)\n", testResults[ii][10], testResults[ii][10]);
    printf("\t'nod$t_naked' offset: %ld (0x%lx)\n", testResults[ii][11], testResults[ii][11]);

    /*
     * Test 7:
     */
    ii++;
    printf("\nTest 7: Align(1) specified.\n");
    testResults[ii][0] = (int64_t) &test_7;
    testResults[ii][1] = ((int64_t) &test_7.nod$t_maskstr - testResults[ii][0]);
    testResults[ii][2] = sizeof(test_7);
    testResults[ii][3] = ((int64_t) &test_7.nod$r_flagunion - testResults[ii][0]);
    testResults[ii][4] = ((int64_t) &test_7.nod$r_flagunion.nod$l_flags - testResults[ii][0]);
    testResults[ii][5] = ((int64_t) &test_7.nod$r_flagunion.nod$l_fixflags - testResults[ii][0]);
    testResults[ii][6] = ((int64_t) &test_7.nod$r_flagunion.nod$r_flagstruc - testResults[ii][0]);
    testResults[ii][7] = ((int64_t) &test_7.nod$r_flags2union - testResults[ii][0]);
    testResults[ii][8] = ((int64_t) &test_7.nod$r_flags2union.nod$l_flags2 - testResults[ii][0]);
    testResults[ii][9] = ((int64_t) &test_7.nod$r_flags2union.nod$l_fixflags2 - testResults[ii][0]);
    testResults[ii][10] = ((int64_t) &test_7.nod$r_flags2union.nod$r_flags2struc - testResults[ii][0]);
    testResults[ii][11] = ((int64_t) &test_7.nod$t_naked - testResults[ii][0]);
    printf("\tBase address: %ld (0x%lx)\n", testResults[ii][0], testResults[ii][0]);
    printf("\tLast offset: %ld (0x%lx)\n", testResults[ii][1], testResults[ii][1]);
    printf("\tSize: %ld (0x%lx)\n", testResults[ii][2], testResults[ii][2]);
    printf("\t'nod$r_flagunion' offset: %ld (0x%lx)\n", testResults[ii][3], testResults[ii][3]);
    printf("\t'nod$r_flagunion.nod$l_flags' offset: %ld (0x%lx)\n", testResults[ii][4], testResults[ii][4]);
    printf("\t'nod$r_flagunion.nod$l_fixflags' offset: %ld (0x%lx)\n", testResults[ii][5], testResults[ii][5]);
    printf("\t'nod$r_flagunion.nod$r_flagstuct' offset: %ld (0x%lx)\n", testResults[ii][6], testResults[ii][6]);
    printf("\t'nod$r_flags2union' offset: %ld (0x%lx)\n", testResults[ii][7], testResults[ii][7]);
    printf("\t'nod$r_flags2union.nod$l_flags2' offset: %ld (0x%lx)\n", testResults[ii][8], testResults[ii][8]);
    printf("\t'nod$r_flags2union.nod$l_fixflags2' offset: %ld (0x%lx)\n", testResults[ii][9], testResults[ii][9]);
    printf("\t'nod$r_flags2union.nod$r_flagstuct' offset: %ld (0x%lx)\n", testResults[ii][10], testResults[ii][10]);
    printf("\t'nod$t_naked' offset: %ld (0x%lx)\n", testResults[ii][11], testResults[ii][11]);

    /*
     * Test 8:
     */
    ii++;
    printf("\nTest 8: Align(2) specified.\n");
    testResults[ii][0] = (int64_t) &test_8;
    testResults[ii][1] = ((int64_t) &test_8.nod$t_maskstr - testResults[ii][0]);
    testResults[ii][2] = sizeof(test_8);
    testResults[ii][3] = ((int64_t) &test_8.nod$r_flagunion - testResults[ii][0]);
    testResults[ii][4] = ((int64_t) &test_8.nod$r_flagunion.nod$l_flags - testResults[ii][0]);
    testResults[ii][5] = ((int64_t) &test_8.nod$r_flagunion.nod$l_fixflags - testResults[ii][0]);
    testResults[ii][6] = ((int64_t) &test_8.nod$r_flagunion.nod$r_flagstruc - testResults[ii][0]);
    testResults[ii][7] = ((int64_t) &test_8.nod$r_flags2union - testResults[ii][0]);
    testResults[ii][8] = ((int64_t) &test_8.nod$r_flags2union.nod$l_flags2 - testResults[ii][0]);
    testResults[ii][9] = ((int64_t) &test_8.nod$r_flags2union.nod$l_fixflags2 - testResults[ii][0]);
    testResults[ii][10] = ((int64_t) &test_8.nod$r_flags2union.nod$r_flags2struc - testResults[ii][0]);
    testResults[ii][11] = ((int64_t) &test_8.nod$t_naked - testResults[ii][0]);
    printf("\tBase address: %ld (0x%lx)\n", testResults[ii][0], testResults[ii][0]);
    printf("\tLast offset: %ld (0x%lx)\n", testResults[ii][1], testResults[ii][1]);
    printf("\tSize: %ld (0x%lx)\n", testResults[ii][2], testResults[ii][2]);
    printf("\t'nod$r_flagunion' offset: %ld (0x%lx)\n", testResults[ii][3], testResults[ii][3]);
    printf("\t'nod$r_flagunion.nod$l_flags' offset: %ld (0x%lx)\n", testResults[ii][4], testResults[ii][4]);
    printf("\t'nod$r_flagunion.nod$l_fixflags' offset: %ld (0x%lx)\n", testResults[ii][5], testResults[ii][5]);
    printf("\t'nod$r_flagunion.nod$r_flagstuct' offset: %ld (0x%lx)\n", testResults[ii][6], testResults[ii][6]);
    printf("\t'nod$r_flags2union' offset: %ld (0x%lx)\n", testResults[ii][7], testResults[ii][7]);
    printf("\t'nod$r_flags2union.nod$l_flags2' offset: %ld (0x%lx)\n", testResults[ii][8], testResults[ii][8]);
    printf("\t'nod$r_flags2union.nod$l_fixflags2' offset: %ld (0x%lx)\n", testResults[ii][9], testResults[ii][9]);
    printf("\t'nod$r_flags2union.nod$r_flagstuct' offset: %ld (0x%lx)\n", testResults[ii][10], testResults[ii][10]);
    printf("\t'nod$t_naked' offset: %ld (0x%lx)\n", testResults[ii][11], testResults[ii][11]);

    /*
     * Test 9:
     */
    ii++;
    printf("\nTest 9: Align(4) specified.\n");
    testResults[ii][0] = (int64_t) &test_9;
    testResults[ii][1] = ((int64_t) &test_9.nod$t_maskstr - testResults[ii][0]);
    testResults[ii][2] = sizeof(test_9);
    testResults[ii][3] = ((int64_t) &test_9.nod$r_flagunion - testResults[ii][0]);
    testResults[ii][4] = ((int64_t) &test_9.nod$r_flagunion.nod$l_flags - testResults[ii][0]);
    testResults[ii][5] = ((int64_t) &test_9.nod$r_flagunion.nod$l_fixflags - testResults[ii][0]);
    testResults[ii][6] = ((int64_t) &test_9.nod$r_flagunion.nod$r_flagstruc - testResults[ii][0]);
    testResults[ii][7] = ((int64_t) &test_9.nod$r_flags2union - testResults[ii][0]);
    testResults[ii][8] = ((int64_t) &test_9.nod$r_flags2union.nod$l_flags2 - testResults[ii][0]);
    testResults[ii][9] = ((int64_t) &test_9.nod$r_flags2union.nod$l_fixflags2 - testResults[ii][0]);
    testResults[ii][10] = ((int64_t) &test_9.nod$r_flags2union.nod$r_flags2struc - testResults[ii][0]);
    testResults[ii][11] = ((int64_t) &test_9.nod$t_naked - testResults[ii][0]);
    printf("\tBase address: %ld (0x%lx)\n", testResults[ii][0], testResults[ii][0]);
    printf("\tLast offset: %ld (0x%lx)\n", testResults[ii][1], testResults[ii][1]);
    printf("\tSize: %ld (0x%lx)\n", testResults[ii][2], testResults[ii][2]);
    printf("\t'nod$r_flagunion' offset: %ld (0x%lx)\n", testResults[ii][3], testResults[ii][3]);
    printf("\t'nod$r_flagunion.nod$l_flags' offset: %ld (0x%lx)\n", testResults[ii][4], testResults[ii][4]);
    printf("\t'nod$r_flagunion.nod$l_fixflags' offset: %ld (0x%lx)\n", testResults[ii][5], testResults[ii][5]);
    printf("\t'nod$r_flagunion.nod$r_flagstuct' offset: %ld (0x%lx)\n", testResults[ii][6], testResults[ii][6]);
    printf("\t'nod$r_flags2union' offset: %ld (0x%lx)\n", testResults[ii][7], testResults[ii][7]);
    printf("\t'nod$r_flags2union.nod$l_flags2' offset: %ld (0x%lx)\n", testResults[ii][8], testResults[ii][8]);
    printf("\t'nod$r_flags2union.nod$l_fixflags2' offset: %ld (0x%lx)\n", testResults[ii][9], testResults[ii][9]);
    printf("\t'nod$r_flags2union.nod$r_flagstuct' offset: %ld (0x%lx)\n", testResults[ii][10], testResults[ii][10]);
    printf("\t'nod$t_naked' offset: %ld (0x%lx)\n", testResults[ii][11], testResults[ii][11]);

    /*
     * Test 10:
     */
    ii++;
    printf("\nTest 10: Align(8) specified.\n");
    testResults[ii][0] = (int64_t) &test_10;
    testResults[ii][1] = ((int64_t) &test_10.nod$t_maskstr - testResults[ii][0]);
    testResults[ii][2] = sizeof(test_10);
    testResults[ii][3] = ((int64_t) &test_10.nod$r_flagunion - testResults[ii][0]);
    testResults[ii][4] = ((int64_t) &test_10.nod$r_flagunion.nod$l_flags - testResults[ii][0]);
    testResults[ii][5] = ((int64_t) &test_10.nod$r_flagunion.nod$l_fixflags - testResults[ii][0]);
    testResults[ii][6] = ((int64_t) &test_10.nod$r_flagunion.nod$r_flagstruc - testResults[ii][0]);
    testResults[ii][7] = ((int64_t) &test_10.nod$r_flags2union - testResults[ii][0]);
    testResults[ii][8] = ((int64_t) &test_10.nod$r_flags2union.nod$l_flags2 - testResults[ii][0]);
    testResults[ii][9] = ((int64_t) &test_10.nod$r_flags2union.nod$l_fixflags2 - testResults[ii][0]);
    testResults[ii][10] = ((int64_t) &test_10.nod$r_flags2union.nod$r_flags2struc - testResults[ii][0]);
    testResults[ii][11] = ((int64_t) &test_10.nod$t_naked - testResults[ii][0]);
    printf("\tBase address: %ld (0x%lx)\n", testResults[ii][0], testResults[ii][0]);
    printf("\tLast offset: %ld (0x%lx)\n", testResults[ii][1], testResults[ii][1]);
    printf("\tSize: %ld (0x%lx)\n", testResults[ii][2], testResults[ii][2]);
    printf("\t'nod$r_flagunion' offset: %ld (0x%lx)\n", testResults[ii][3], testResults[ii][3]);
    printf("\t'nod$r_flagunion.nod$l_flags' offset: %ld (0x%lx)\n", testResults[ii][4], testResults[ii][4]);
    printf("\t'nod$r_flagunion.nod$l_fixflags' offset: %ld (0x%lx)\n", testResults[ii][5], testResults[ii][5]);
    printf("\t'nod$r_flagunion.nod$r_flagstuct' offset: %ld (0x%lx)\n", testResults[ii][6], testResults[ii][6]);
    printf("\t'nod$r_flags2union' offset: %ld (0x%lx)\n", testResults[ii][7], testResults[ii][7]);
    printf("\t'nod$r_flags2union.nod$l_flags2' offset: %ld (0x%lx)\n", testResults[ii][8], testResults[ii][8]);
    printf("\t'nod$r_flags2union.nod$l_fixflags2' offset: %ld (0x%lx)\n", testResults[ii][9], testResults[ii][9]);
    printf("\t'nod$r_flags2union.nod$r_flagstuct' offset: %ld (0x%lx)\n", testResults[ii][10], testResults[ii][10]);
    printf("\t'nod$t_naked' offset: %ld (0x%lx)\n", testResults[ii][11], testResults[ii][11]);

    /*
     * Test 11:
     */
    ii++;
    printf("\nTest 11: Align(16) specified.\n");
    testResults[ii][0] = (int64_t) &test_11;
    testResults[ii][1] = ((int64_t) &test_11.nod$t_maskstr - testResults[ii][0]);
    testResults[ii][2] = sizeof(test_11);
    testResults[ii][3] = ((int64_t) &test_11.nod$r_flagunion - testResults[ii][0]);
    testResults[ii][4] = ((int64_t) &test_11.nod$r_flagunion.nod$l_flags - testResults[ii][0]);
    testResults[ii][5] = ((int64_t) &test_11.nod$r_flagunion.nod$l_fixflags - testResults[ii][0]);
    testResults[ii][6] = ((int64_t) &test_11.nod$r_flagunion.nod$r_flagstruc - testResults[ii][0]);
    testResults[ii][7] = ((int64_t) &test_11.nod$r_flags2union - testResults[ii][0]);
    testResults[ii][8] = ((int64_t) &test_11.nod$r_flags2union.nod$l_flags2 - testResults[ii][0]);
    testResults[ii][9] = ((int64_t) &test_11.nod$r_flags2union.nod$l_fixflags2 - testResults[ii][0]);
    testResults[ii][10] = ((int64_t) &test_11.nod$r_flags2union.nod$r_flags2struc - testResults[ii][0]);
    testResults[ii][11] = ((int64_t) &test_11.nod$t_naked - testResults[ii][0]);
    printf("\tBase address: %ld (0x%lx)\n", testResults[ii][0], testResults[ii][0]);
    printf("\tLast offset: %ld (0x%lx)\n", testResults[ii][1], testResults[ii][1]);
    printf("\tSize: %ld (0x%lx)\n", testResults[ii][2], testResults[ii][2]);
    printf("\t'nod$r_flagunion' offset: %ld (0x%lx)\n", testResults[ii][3], testResults[ii][3]);
    printf("\t'nod$r_flagunion.nod$l_flags' offset: %ld (0x%lx)\n", testResults[ii][4], testResults[ii][4]);
    printf("\t'nod$r_flagunion.nod$l_fixflags' offset: %ld (0x%lx)\n", testResults[ii][5], testResults[ii][5]);
    printf("\t'nod$r_flagunion.nod$r_flagstuct' offset: %ld (0x%lx)\n", testResults[ii][6], testResults[ii][6]);
    printf("\t'nod$r_flags2union' offset: %ld (0x%lx)\n", testResults[ii][7], testResults[ii][7]);
    printf("\t'nod$r_flags2union.nod$l_flags2' offset: %ld (0x%lx)\n", testResults[ii][8], testResults[ii][8]);
    printf("\t'nod$r_flags2union.nod$l_fixflags2' offset: %ld (0x%lx)\n", testResults[ii][9], testResults[ii][9]);
    printf("\t'nod$r_flags2union.nod$r_flagstuct' offset: %ld (0x%lx)\n", testResults[ii][10], testResults[ii][10]);
    printf("\t'nod$t_naked' offset: %ld (0x%lx)\n", testResults[ii][11], testResults[ii][11]);

    printf("\n\nTest\t\tTest 5\t\tTest 6\t\tTest 7\t\tTests 8\t\tTest 9\t\tTest 10\t\tTest 11\n");
    for (jj = 0; jj < 12; jj++)
    {
	printf("%s\t", testCase[jj]);
	if (strlen(testCase[jj]) < 8)
	    printf("\t");
	for (ii = 0; ii < 7; ii++)
	    printf("0x%.8lx\t", testResults[ii][jj]);
	printf("\n");
    }
    printf("\n");
    return(retVal);
}
