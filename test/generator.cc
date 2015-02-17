/*
  copyright (C) 2015 Tim Orford <tim@orford.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 3
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <getopt.h>
#include "cpgrs.h"
#include "generator.h"


Note::Note (Generator* g, int _length)
{
	generator = g;
	length = _length;
	t = 0;

	generator->init();
	generator->on = true;
}


Note::~Note()
{
}


void
Note::compute (int count, double** input, double** output)
{
	generator->compute(count, input, output);

	if(++t > length) generator->on = false;
}

