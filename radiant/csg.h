/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if !defined(INCLUDED_CSG_H)
#define INCLUDED_CSG_H

#include <string>
#include "iclipper.h"

namespace scene
{
  class Graph;
}
template<typename Element> class BasicVector3;
typedef BasicVector3<double> Vector3;
class Plane3;

void Scene_BrushSetClipPlane(const Plane3& plane);
void Scene_BrushSplitByPlane(const Vector3 planePoints[3], const std::string& shader, EBrushSplit split);

#endif
