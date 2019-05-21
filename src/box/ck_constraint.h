#ifndef INCLUDES_BOX_CK_CONSTRAINT_H
#define INCLUDES_BOX_CK_CONSTRAINT_H
/*
 * Copyright 2010-2019, Tarantool AUTHORS, please see AUTHORS file.
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

#include <stdint.h>
#include "small/rlist.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct space;
struct space_def;
struct Expr;

/** The supported language of the ck constraint. */
enum ck_constraint_language {
  CK_CONSTRAINT_LANGUAGE_SQL,
  ck_constraint_language_MAX,
};

/** The supported languages strings of the ck constraint.  */
extern const char *ck_constraint_language_strs[];

/**
 * Check constraint definition.
 * See ck_constraint_def_sizeof() definition for implementation
 * details and memory layout.
 */
struct ck_constraint_def {
	/**
	 * The 0-terminated string, a name of the check
	 * constraint. Must be unique for a given space.
	 */
	char *name;
	/**
	 * The 0-terminated string that defines check constraint
	 * expression.
	 *
	 * For instance: "field1 + field2 > 2 * 3".
	 */
	char *expr_str;
	/** The language of ck constraint. */
	enum ck_constraint_language language;
};

/**
 * Structure representing check constraint.
 * See ck_constraint_new() definition.
 */
struct ck_constraint {
	/** The check constraint definition. */
	struct ck_constraint_def *def;
	/**
	 * The check constraint expression AST is built for
	 * ck_constraint::def::expr_str with sql_expr_compile
	 * and resolved with sql_resolve_self_reference for
	 * space with space[ck_constraint::space_id] definition.
	 */
	struct Expr *expr;
	/**
	 * The id of the space this check constraint is
	 * built for.
	 */
	uint32_t space_id;
	/**
	 * Organize check constraint structs into linked list
	 * with space::ck_constraint.
	 */
	struct rlist link;
};

/**
 * Calculate check constraint definition memory size and fields
 * offsets for given arguments.
 *
 * Alongside with struct ck_constraint_def itself, we reserve
 * memory for string containing its name and expression string.
 *
 * Memory layout:
 * +-----------------------------+ <- Allocated memory starts here
 * |   struct ck_constraint_def  |
 * |-----------------------------|
 * |          name + \0          |
 * |-----------------------------|
 * |        expr_str + \0        |
 * +-----------------------------+
 *
 * @param name_len The length of the name.
 * @param expr_str_len The length of the expr_str.
 * @param[out] name_offset The offset of the name string.
 * @param[out] expr_str_offset The offset of the expr_str string.
 */
static inline uint32_t
ck_constraint_def_sizeof(uint32_t name_len, uint32_t expr_str_len,
			 uint32_t *name_offset, uint32_t *expr_str_offset)
{
	*name_offset = sizeof(struct ck_constraint_def);
	*expr_str_offset = *name_offset + name_len + 1;
	return *expr_str_offset + expr_str_len + 1;
}

/**
 * Create a new check constraint object by given check constraint
 * definition and definition of the space this constraint is
 * related to.
 *
 * @param ck_constraint_def The check constraint definition object
 *                          to use. Expected to be allocated with
 *                          malloc. Ck constraint object manages
 *                          this allocation in case of successful
 *                          creation.
 * @param space_def The space definition of the space this check
 *                  constraint must be constructed for.
 * @retval not NULL Check constraint object pointer on success.
 * @retval NULL Otherwise.
*/
struct ck_constraint *
ck_constraint_new(struct ck_constraint_def *ck_constraint_def,
		  struct space_def *space_def);

/**
 * Destroy check constraint memory, release acquired resources.
 * @param ck_constraint The check constraint object to destroy.
 */
void
ck_constraint_delete(struct ck_constraint *ck_constraint);

/**
 * Find check constraint object in space by given name and
 * name_len.
 * @param space The space to lookup check constraint.
 * @param name The check constraint name.
 * @param name_len The length of the name.
 * @retval not NULL Check constrain if exists, NULL otherwise.
 */
struct ck_constraint *
space_ck_constraint_by_name(struct space *space, const char *name,
			    uint32_t name_len);

#if defined(__cplusplus)
} /* extern "C" { */
#endif

#endif /* INCLUDES_BOX_CK_CONSTRAINT_H */
