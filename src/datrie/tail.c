/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * libdatrie - Double-Array Trie Library
 * Copyright (C) 2006  Theppitak Karoonboonyanan <thep@linux.thai.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 * tail.c - trie tail for keeping suffixes
 * Created: 2006-08-15
 * Author:  Theppitak Karoonboonyanan <thep@linux.thai.net>
 */

#include <string.h>
#include <stdlib.h>
#ifndef _MSC_VER /* for SIZE_MAX */
# include <stdint.h>
#endif
#include <stdio.h>

#include "tail.h"

/*----------------------------------*
 *    INTERNAL TYPES DECLARATIONS   *
 *----------------------------------*/

/*-----------------------------------*
 *    PRIVATE METHODS DECLARATIONS   *
 *-----------------------------------*/

static TrieIndex    tail_alloc_block (Tail *t);
static void         tail_free_block (Tail *t, TrieIndex block);

/* ==================== BEGIN IMPLEMENTATION PART ====================  */

/*------------------------------------*
 *   INTERNAL TYPES IMPLEMENTATIONS   *
 *------------------------------------*/

/*------------------------------*
 *    PRIVATE DATA DEFINITONS   *
 *------------------------------*/

typedef struct {
    TrieIndex   next_free;
    TrieData    data;
    TrieChar   *suffix;
} TailBlock;

struct _Tail {
    TrieIndex   num_tails;
    TailBlock  *tails;
    TrieIndex   first_free;
};

/*-----------------------------*
 *    METHODS IMPLEMENTAIONS   *
 *-----------------------------*/

#define TAIL_SIGNATURE      0xDFFCDFFC
#define TAIL_START_BLOCKNO  1

/* Tail Header:
 * INT32: signature
 * INT32: pointer to first free slot
 * INT32: number of tail blocks
 *
 * Tail Blocks:
 * INT32: pointer to next free block (-1 for allocated blocks)
 * INT32: data for the key
 * INT16: length
 * BYTES[length]: suffix string (no terminating '\0')
 */

/**
 * @brief Create a new tail object
 *
 * Create a new empty tail object.
 */
Tail *
tail_new (void)
{
    Tail       *t;

    t = (Tail *) malloc (sizeof (Tail));
    if (!t)
        return NULL;

    t->first_free = 0;
    t->num_tails  = 0;
    t->tails      = NULL;

    return t;
}

/**
 * @brief Free tail data
 *
 * @param t : the tail data
 *
 * @return 0 on success, non-zero on failure
 *
 * Free the given tail data.
 */
void
tail_free (Tail *t)
{
    TrieIndex   i;

    if (t->tails) {
        for (i = 0; i < t->num_tails; i++)
            if (t->tails[i].suffix)
                free (t->tails[i].suffix);
        free (t->tails);
    }
    free (t);
}


/**
 * @brief Get suffix
 *
 * @param t     : the tail data
 * @param index : the index of the suffix
 *
 * @return pointer to the indexed suffix string.
 *
 * Get suffix from tail with given @a index. The returned string is a pointer
 * to internal storage, which should be accessed read-only by the caller.
 * No need to free() it.
 */
const TrieChar *
tail_get_suffix (const Tail *t, TrieIndex index)
{
    index -= TAIL_START_BLOCKNO;
    return (index < t->num_tails) ? t->tails[index].suffix : NULL;
}

/**
 * @brief Set suffix of existing entry
 *
 * @param t      : the tail data
 * @param index  : the index of the suffix
 * @param suffix : the new suffix
 *
 * Set suffix of existing entry of given @a index in tail.
 */
bool
tail_set_suffix (Tail *t, TrieIndex index, const TrieChar *suffix)
{
    index -= TAIL_START_BLOCKNO;
    if (index < t->num_tails) {
        /* suffix and t->tails[index].suffix may overlap;
         * so, dup it before it's overwritten
         */
        TrieChar *tmp = NULL;
        if (suffix)
            tmp = (TrieChar *) strdup ((const char *)suffix);
        if (t->tails[index].suffix)
            free (t->tails[index].suffix);
        t->tails[index].suffix = tmp;

        return true;
    }
    return false;
}

/**
 * @brief Add a new suffix
 *
 * @param t      : the tail data
 * @param suffix : the new suffix
 *
 * @return the index of the newly added suffix.
 *
 * Add a new suffix entry to tail.
 */
TrieIndex
tail_add_suffix (Tail *t, const TrieChar *suffix)
{
    TrieIndex   new_block;

    new_block = tail_alloc_block (t);
    tail_set_suffix (t, new_block, suffix);

    return new_block;
}

static TrieIndex
tail_alloc_block (Tail *t)
{
    TrieIndex   block;

    if (0 != t->first_free) {
        block = t->first_free;
        t->first_free = t->tails[block].next_free;
    } else {
        block = t->num_tails;
        t->tails = (TailBlock *) realloc (t->tails,
                                          ++t->num_tails * sizeof (TailBlock));
    }
    t->tails[block].next_free = -1;
    t->tails[block].data = TRIE_DATA_ERROR;
    t->tails[block].suffix = NULL;

    return block + TAIL_START_BLOCKNO;
}

static void
tail_free_block (Tail *t, TrieIndex block)
{
    TrieIndex   i, j;

    block -= TAIL_START_BLOCKNO;

    if (block >= t->num_tails)
        return;

    t->tails[block].data = TRIE_DATA_ERROR;
    if (NULL != t->tails[block].suffix) {
        free (t->tails[block].suffix);
        t->tails[block].suffix = NULL;
    }

    /* find insertion point */
    j = 0;
    for (i = t->first_free; i != 0 && i < block; i = t->tails[i].next_free)
        j = i;

    /* insert free block between j and i */
    t->tails[block].next_free = i;
    if (0 != j)
        t->tails[j].next_free = block;
    else
        t->first_free = block;
}

/**
 * @brief Get data associated to suffix entry
 *
 * @param t      : the tail data
 * @param index  : the index of the suffix
 *
 * @return the data associated to the suffix entry
 *
 * Get data associated to suffix entry @a index in tail data.
 */
TrieData
tail_get_data (const Tail *t, TrieIndex index)
{
    index -= TAIL_START_BLOCKNO;
    return (index < t->num_tails) ? t->tails[index].data : TRIE_DATA_ERROR;
}

/**
 * @brief Set data associated to suffix entry
 *
 * @param t      : the tail data
 * @param index  : the index of the suffix
 * @param data   : the data to set
 *
 * @return boolean indicating success
 *
 * Set data associated to suffix entry @a index in tail data.
 */
bool
tail_set_data (Tail *t, TrieIndex index, TrieData data)
{
    index -= TAIL_START_BLOCKNO;
    if (index < t->num_tails) {
        t->tails[index].data = data;
        return true;
    }
    return false;
}

/**
 * @brief Delete suffix entry
 *
 * @param t      : the tail data
 * @param index  : the index of the suffix to delete
 *
 * Delete suffix entry from the tail data.
 */
void
tail_delete (Tail *t, TrieIndex index)
{
    tail_free_block (t, index);
}

/**
 * @brief Walk in tail with a string
 *
 * @param t          : the tail data
 * @param s          : the tail data index
 * @param suffix_idx : pointer to current character index in suffix
 * @param str        : the string to use in walking
 * @param len        : total characters in @a str to walk
 *
 * @return total number of characters successfully walked
 *
 * Walk in the tail data @a t at entry @a s, from given character position
 * @a *suffix_idx, using @a len characters of given string @a str. On return,
 * @a *suffix_idx is updated to the position after the last successful walk,
 * and the function returns the total number of character succesfully walked.
 */
int
tail_walk_str  (const Tail      *t,
                TrieIndex        s,
                short           *suffix_idx,
                const TrieChar  *str,
                int              len)
{
    const TrieChar *suffix;
    int             i;
    short           j;

    suffix = tail_get_suffix (t, s);
    if (!suffix)
        return false;

    i = 0; j = *suffix_idx;
    while (i < len) {
        if (str[i] != suffix[j])
            break;
        ++i;
        /* stop and stay at null-terminator */
        if (0 == suffix[j])
            break;
        ++j;
    }
    *suffix_idx = j;
    return i;
}

/**
 * @brief Walk in tail with a character
 *
 * @param t          : the tail data
 * @param s          : the tail data index
 * @param suffix_idx : pointer to current character index in suffix
 * @param c          : the character to use in walking
 *
 * @return boolean indicating success
 *
 * Walk in the tail data @a t at entry @a s, from given character position
 * @a *suffix_idx, using given character @a c. If the walk is successful,
 * it returns true, and @a *suffix_idx is updated to the next character.
 * Otherwise, it returns false, and @a *suffix_idx is left unchanged.
 */
bool
tail_walk_char (const Tail      *t,
                TrieIndex        s,
                short           *suffix_idx,
                TrieChar         c)
{
    const TrieChar *suffix;
    TrieChar        suffix_char;

    suffix = tail_get_suffix (t, s);
    if (!suffix)
        return false;

    suffix_char = suffix[*suffix_idx];
    if (suffix_char == c) {
        if (0 != suffix_char)
            ++*suffix_idx;
        return true;
    }
    return false;
}

/*
vi:ts=4:ai:expandtab
*/
