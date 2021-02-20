/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2013-2021 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
* +----------------------------------------------------------------------+
* | WaveformGrid draws timeline markers onto a shared opengl drawable.   |
* +----------------------------------------------------------------------+
* | The scaling is controlled by the sample_rate and samples_per_pixel   |
* | properties of the WaveformContext.                                   |
* +----------------------------------------------------------------------+
*/

#ifndef __wf_grid_actor_h__
#define __wf_grid_actor_h__

AGlActor* grid_actor (WaveformActor*);

#endif
