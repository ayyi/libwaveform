/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2018-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#ifndef __ayyi_value_h__
#define __ayyi_value_h__

typedef union
{
    int         i;
    unsigned    u;
    float       f;
    double      d;
    char*       c;
    int64_t     b;
    void*       p;
    struct { int32_t val, prev; } s;
} AyyiVal;

#endif
