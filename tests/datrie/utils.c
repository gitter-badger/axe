/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * libdatrie - Double-Array Trie Library
 * Copyright (C) 2013  Theppitak Karoonboonyanan <thep@linux.thai.net>
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
 * utils.c - Utility functions for datrie test cases
 * Created: 2013-10-16
 * Author:  Theppitak Karoonboonyanan <thep@linux.thai.net>
 */

#include <datrie/trie.h>
#include "utils.h"

/*---------------------*
 *  Debugging helpers  *
 *---------------------*/
void
msg_step (const char *msg)
{
    printf ("=> %s...\n", msg);
}

/*-------------------------*
 *  Trie creation helpers  *
 *-------------------------*/
static AlphaMap *
en_alpha_map_new ()
{
    AlphaMap *en_map;

    en_map = alpha_map_new ();
    if (!en_map)
        goto err_map_not_created;

    if (alpha_map_add_range (en_map, 0x0061, 0x007a) != 0)
        goto err_map_created;

    return en_map;

err_map_created:
    alpha_map_free (en_map);
err_map_not_created:
    return NULL;
}

Trie *
en_trie_new ()
{
    AlphaMap *en_map;
    Trie     *en_trie;

    en_map = en_alpha_map_new ();
    if (!en_map)
        goto err_map_not_created;

    en_trie = trie_new (en_map);
    if (!en_trie)
        goto err_map_created;

    alpha_map_free (en_map);
    return en_trie;

err_map_created:
    alpha_map_free (en_map);
err_map_not_created:
    return NULL;
}

/*---------------------------*
 *  Dict source for testing  *
 *---------------------------*/
DictRec dict_src[] = {
    {(AlphaChar *)"a",          TRIE_DATA_UNREAD},
    {(AlphaChar *)"abacus",     TRIE_DATA_UNREAD},
    {(AlphaChar *)"abandon",    TRIE_DATA_UNREAD},
    {(AlphaChar *)"accident",   TRIE_DATA_UNREAD},
    {(AlphaChar *)"accredit",   TRIE_DATA_UNREAD},
    {(AlphaChar *)"algorithm",  TRIE_DATA_UNREAD},
    {(AlphaChar *)"ammonia",    TRIE_DATA_UNREAD},
    {(AlphaChar *)"angel",      TRIE_DATA_UNREAD},
    {(AlphaChar *)"angle",      TRIE_DATA_UNREAD},
    {(AlphaChar *)"azure",      TRIE_DATA_UNREAD},
    {(AlphaChar *)"bat",        TRIE_DATA_UNREAD},
    {(AlphaChar *)"bet",        TRIE_DATA_UNREAD},
    {(AlphaChar *)"best",       TRIE_DATA_UNREAD},
    {(AlphaChar *)"home",       TRIE_DATA_UNREAD},
    {(AlphaChar *)"house",      TRIE_DATA_UNREAD},
    {(AlphaChar *)"hut",        TRIE_DATA_UNREAD},
    {(AlphaChar *)"king",       TRIE_DATA_UNREAD},
    {(AlphaChar *)"kite",       TRIE_DATA_UNREAD},
    {(AlphaChar *)"name",       TRIE_DATA_UNREAD},
    {(AlphaChar *)"net",        TRIE_DATA_UNREAD},
    {(AlphaChar *)"network",    TRIE_DATA_UNREAD},
    {(AlphaChar *)"nut",        TRIE_DATA_UNREAD},
    {(AlphaChar *)"nutshell",   TRIE_DATA_UNREAD},
    {(AlphaChar *)"quality",    TRIE_DATA_UNREAD},
    {(AlphaChar *)"quantum",    TRIE_DATA_UNREAD},
    {(AlphaChar *)"quantity",   TRIE_DATA_UNREAD},
    {(AlphaChar *)"quartz",     TRIE_DATA_UNREAD},
    {(AlphaChar *)"quick",      TRIE_DATA_UNREAD},
    {(AlphaChar *)"quiz",       TRIE_DATA_UNREAD},
    {(AlphaChar *)"run",        TRIE_DATA_UNREAD},
    {(AlphaChar *)"tape",       TRIE_DATA_UNREAD},
    {(AlphaChar *)"test",       TRIE_DATA_UNREAD},
    {(AlphaChar *)"what",       TRIE_DATA_UNREAD},
    {(AlphaChar *)"when",       TRIE_DATA_UNREAD},
    {(AlphaChar *)"where",      TRIE_DATA_UNREAD},
    {(AlphaChar *)"which",      TRIE_DATA_UNREAD},
    {(AlphaChar *)"who",        TRIE_DATA_UNREAD},
    {(AlphaChar *)"why",        TRIE_DATA_UNREAD},
    {(AlphaChar *)"zebra",      TRIE_DATA_UNREAD},
    {(AlphaChar *)NULL,          TRIE_DATA_ERROR},
};

int
dict_src_n_entries ()
{
    return sizeof (dict_src) / sizeof (dict_src[0]) - 1;
}

TrieData
dict_src_get_data (const AlphaChar *key)
{
    const DictRec *dict_p;

    for (dict_p = dict_src; dict_p->key; dict_p++) {
        if (alpha_char_strcmp (dict_p->key, key) == 0) {
            return dict_p->data;
        }
    }

    return TRIE_DATA_ERROR;
}

int
dict_src_set_data (const AlphaChar *key, TrieData data)
{
    DictRec *dict_p;

    for (dict_p = dict_src; dict_p->key; dict_p++) {
        if (alpha_char_strcmp (dict_p->key, key) == 0) {
            dict_p->data = data;
            return 0;
        }
    }

    return -1;
}

/*
vi:ts=4:ai:expandtab
*/
