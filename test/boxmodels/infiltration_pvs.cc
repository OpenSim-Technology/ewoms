// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*****************************************************************************
 *   Copyright (C) 2011      by Holger Class                                 *
 *   Copyright (C) 2007-2008 by Klaus Mosthaf                                *
 *   Copyright (C) 2007-2008 by Melanie Darcis                               *
 *   Copyright (C) 2007-2008 by Bernd Flemisch                               *
 *   Copyright (C) 2008-2009 by Andreas Lauser                               *
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
 * \file
 *
 * \brief test for the primary variable switching box model
 */
#include "config.h"

#include "problems/infiltrationproblem.hh"
#include <dumux/boxmodels/pvs/pvsmodel.hh>
#include <dumux/common/start.hh>

namespace Dumux {
namespace Properties {
NEW_TYPE_TAG(InfiltrationProblem, INHERITS_FROM(BoxPvs, InfiltrationBaseProblem));
}}

int main(int argc, char** argv)
{
    typedef TTAG(InfiltrationProblem) ProblemTypeTag;
    return Dumux::start<ProblemTypeTag>(argc, argv);
}

