// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*****************************************************************************
 *   Copyright (C) 2009-2011 by Andreas Lauser                               *
 *   Copyright (C) 2008-2009 by Klaus Mosthaf                                *
 *   Copyright (C) 2008 by Bernd Flemisch                                    *
 *   Institute for Modelling Hydraulic and Environmental Systems             *
 *   University of Stuttgart, Germany                                        *
 *   email: <givenname>.<name>@iws.uni-stuttgart.de                          *
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
 * \brief This file contains the data which is required to calculate
 *        all fluxes of components over a face of a finite volume.
 *
 * This means pressure, concentration and temperature gradients, phase
 * densities at the integration point, etc.
 */
#ifndef DUMUX_MPNC_FLUX_VARIABLES_HH
#define DUMUX_MPNC_FLUX_VARIABLES_HH

#include "diffusion/fluxvariables.hh"
#include "energy/mpncfluxvariablesenergy.hh"

#include <dumux/boxmodels/common/boxmultiphasefluxvariables.hh>

#include <dune/common/fvector.hh>

namespace Dumux
{
/*!
 * \ingroup MPNCModel
 * \ingroup BoxFluxVariables
 * \brief This template class contains the data which is required to
 *        calculate all fluxes of components over a face of a finite
 *        volume for the two-phase, three-component model.
 *
 * This means pressure and concentration gradients, phase densities at
 * the intergration point, etc.
 */
template <class TypeTag>
class MPNCFluxVariables : public BoxMultiPhaseFluxVariables<TypeTag>
{
    typedef BoxMultiPhaseFluxVariables<TypeTag> MultiPhaseFluxVariables;

    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef typename GET_PROP_TYPE(TypeTag, GridView) GridView;
    typedef typename GET_PROP_TYPE(TypeTag, ElementContext) ElementContext;

    enum {
        dimWorld = GridView::dimensionworld,

        enableDiffusion = GET_PROP_VALUE(TypeTag, EnableDiffusion),
        enableEnergy = GET_PROP_VALUE(TypeTag, EnableEnergy)
    };

    typedef Dune::FieldVector<Scalar, dimWorld> DimVector;

    typedef MPNCFluxVariablesDiffusion<TypeTag, enableDiffusion> FluxVariablesDiffusion;
    typedef MPNCFluxVariablesEnergy<TypeTag, enableEnergy> FluxVariablesEnergy;

public:
    void update(const ElementContext &elemCtx, int scvfIdx, int timeIdx)
    {
        MultiPhaseFluxVariables::update(elemCtx, scvfIdx, timeIdx);

        // update the flux data of the energy module (i.e. isothermal
        // or non-isothermal)
        energyVars_.update(elemCtx, scvfIdx, timeIdx);

        // update the flux data of the diffusion module (i.e. with or
        // without diffusion)
        diffusionVars_.update(elemCtx, scvfIdx, timeIdx);
    }

    ////////////////////////////////////////////////
    // forward calls to the diffusion module
    Scalar porousDiffCoeffL(int compIdx) const
    { return diffusionVars_.porousDiffCoeffL(compIdx); }

    Scalar porousDiffCoeffG(int compIIdx, int compJIdx) const
    { return diffusionVars_.porousDiffCoeffG(compIIdx, compJIdx); }

    const Scalar moleFraction(int phaseIdx, int compIdx) const
    { return diffusionVars_.moleFraction(phaseIdx, compIdx); }

    const DimVector &moleFracGrad(int phaseIdx,
                               int compIdx) const
    { return diffusionVars_.moleFracGrad(phaseIdx, compIdx); }
    // end of forward calls to the diffusion module
    ////////////////////////////////////////////////

    /*!
     * \brief Returns the variables relevant for the energy module
     */
    const FluxVariablesEnergy &energyVars() const
    { return energyVars_; }

private:
    // data for the diffusion and the energy modules
    FluxVariablesDiffusion diffusionVars_;
    FluxVariablesEnergy energyVars_;
};

} // end namepace

#endif
