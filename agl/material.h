/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | copyright (C) 2013-2017 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __agl_material_h__
#define __agl_material_h__
#include "agl/utils.h"

typedef struct _AGlMaterial AGlMaterial;

typedef struct {
    GLuint     texture;
    AGlShader* shader;

    void       (*init)   ();
    void       (*free)   (AGlMaterial*);
    void       (*render) (AGlMaterial*);

} AGlMaterialClass;

struct _AGlMaterial {
    AGlMaterialClass* material_class;
};

void         agl_use_material (AGlMaterial*);

AGlMaterial* agl_aa_line_new  ();

#endif
