/****************************************************************************

 Copyright (C) 2010-2012 the Office National des Forêts (ONF), France
                     and the Association de Recherche Technologie et Sciences (ARTS), Ecole Nationale Supérieure d'Arts et Métiers (ENSAM), Cluny, France.
                     All rights reserved.

 Contact : alexandre.piboule@onf.fr

 Developers : Michaël KREBS (ARTS/ENSAM)

 This file is part of PluginShared library 2.0.

 PluginShared is free library: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 PluginShared is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with PluginShared.  If not, see <http://www.gnu.org/licenses/lgpl.html>.

*****************************************************************************/

#ifndef CT_ABSTRACTSTEPCANBEADDEDFIRST_H
#define CT_ABSTRACTSTEPCANBEADDEDFIRST_H

#include "ct_abstractstepserializable.h"

/**
 * @brief Represent a step that can be added in root of the tree of in middle but not necessary load file.
 */
class PLUGINSHAREDSHARED_EXPORT CT_AbstractStepCanBeAddedFirst : public CT_AbstractStepSerializable
{
    Q_OBJECT

public:
    CT_AbstractStepCanBeAddedFirst(CT_StepInitializeData &dataInit);
    virtual ~CT_AbstractStepCanBeAddedFirst();
};

#endif // CT_ABSTRACTSTEPCANBEADDEDFIRST_H
