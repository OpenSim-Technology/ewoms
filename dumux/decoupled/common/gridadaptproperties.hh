// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*****************************************************************************
 *   Copyright (C) 2011 by  Markus Wolff                                     *
 *                                                                           *
 *   This program is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation, either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 *****************************************************************************/
/*!
 * \ingroup IMPETProperties
 * \ingroup IMPET
 * \file
 *
 * \brief Defines a type tag and some fundamental properties for
 *        linear solvers
 */
#ifndef DUMUX_GRIDADAPT_PROPERTIES_HH
#define DUMUX_GRIDADAPT_PROPERTIES_HH

#include <dumux/common/basicproperties.hh>

namespace Dumux
{
namespace Properties
{
//! Grid adaption type tag for all decoupled models.
NEW_TYPE_TAG(GridAdaptTypeTag);

//! Defines if the grid is h-adaptive
NEW_PROP_TAG( AdaptiveGrid);

//! Class defining the refinement/coarsening indicator
NEW_PROP_TAG(AdaptionIndicator);

//! Class defining the refinement/coarsening indicator for grid initialization
NEW_PROP_TAG(AdaptionInitializationIndicator);

//! Switch the use of initial grid adaption on/of
NEW_PROP_TAG(EnableInitializationIndicator);

//! Mimimum allowed level
NEW_PROP_TAG(MinLevel);

//! Maximum allowed level
NEW_PROP_TAG(MaxLevel);

//! Tolerance for refinement
NEW_PROP_TAG(RefineTolerance);

//! Tolerance for coarsening
NEW_PROP_TAG(CoarsenTolerance);

//! Tolerance for refinement
NEW_PROP_TAG(RefineThreshold);

//! Tolerance for coarsening
NEW_PROP_TAG(CoarsenThreshold);

//! Time step interval for adaption
NEW_PROP_TAG(AdaptionInterval);

//! Switch for refinement at dirichlet BC's -> not used by all indicators!
NEW_PROP_TAG(RefineAtDirichletBC);

//! Switch for refinement at neumann BC's -> not used by all indicators!
NEW_PROP_TAG(RefineAtFluxBC);

//! Switch for refinement at sources -> not used by all indicators!
NEW_PROP_TAG(RefineAtSource);

//no adaptive grid
SET_BOOL_PROP(GridAdaptTypeTag, AdaptiveGrid, false);

//standard setting
SET_INT_PROP(GridAdaptTypeTag, MinLevel, 0);
SET_INT_PROP(GridAdaptTypeTag, MaxLevel, 1);
SET_SCALAR_PROP(GridAdaptTypeTag, RefineTolerance, 0.05);
SET_SCALAR_PROP(GridAdaptTypeTag, CoarsenTolerance, 0.001);
SET_SCALAR_PROP(GridAdaptTypeTag, RefineThreshold, 0.0);
SET_SCALAR_PROP(GridAdaptTypeTag, CoarsenThreshold, 0.0);
SET_INT_PROP(GridAdaptTypeTag, AdaptionInterval, 1);
//Switch initial grid adaption off per default
SET_BOOL_PROP(GridAdaptTypeTag, EnableInitializationIndicator, false);

// Switch of extra refinement strategy at boundaries/sources
SET_BOOL_PROP(GridAdaptTypeTag, RefineAtDirichletBC, false);
SET_BOOL_PROP(GridAdaptTypeTag, RefineAtFluxBC, false);
SET_BOOL_PROP(GridAdaptTypeTag, RefineAtSource, false);
} // namespace Properties
} // namespace Dumux


#endif
