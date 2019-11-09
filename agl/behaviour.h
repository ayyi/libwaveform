/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2019-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/

#ifndef __agl_behaviour_h__
#define __agl_behaviour_h__

typedef struct _AGlBehaviour AGlBehaviour;

typedef AGlBehaviour* (*AGlBehaviourNew)   ();
typedef void          (*AGlBehaviourFn)    (AGlBehaviour*);
typedef void          (*AGlBehaviourInit)  (AGlBehaviour*, AGlActor*);

typedef struct
{
    AGlBehaviourNew      new;
    AGlBehaviourFn       free;
    AGlBehaviourInit     init;
} AGlBehaviourClass;

struct _AGlBehaviour
{
    AGlBehaviourClass* klass;
};

#endif
