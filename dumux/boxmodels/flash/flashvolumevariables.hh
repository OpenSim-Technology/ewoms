// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*****************************************************************************
 *   Copyright (C) 2012 by Andreas Lauser,                                   *
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
 * \brief Contains the quantities which are constant within a
 *        finite volume for the flash-based compositional model.
 */
#ifndef DUMUX_FLASH_VOLUME_VARIABLES_HH
#define DUMUX_FLASH_VOLUME_VARIABLES_HH

#include "flashproperties.hh"
#include "flashindices.hh"

#include <dumux/boxmodels/common/boxmodel.hh>
#include <dumux/material/fluidstates/compositionalfluidstate.hh>
#include <dumux/material/constraintsolvers/ncpflash.hh>
#include <dumux/common/math.hh>

#include <dune/common/collectivecommunication.hh>
#include <dune/common/fvector.hh>

namespace Dumux
{

/*!
 * \ingroup FlashModel
 * \ingroup BoxVolumeVariables
 * \brief Contains the quantities which are constant within a
 *        finite volume for the flash-based compositional model.
 */
template <class TypeTag>
class FlashVolumeVariables : public BoxVolumeVariables<TypeTag>
{
    typedef BoxVolumeVariables<TypeTag> ParentType;

    typedef typename GET_PROP_TYPE(TypeTag, VolumeVariables) Implementation;
    typedef typename GET_PROP_TYPE(TypeTag, RateVector) RateVector;
    typedef typename GET_PROP_TYPE(TypeTag, PrimaryVariables) PrimaryVariables;
    typedef typename GET_PROP_TYPE(TypeTag, ElementContext) ElementContext;
    typedef typename GET_PROP_TYPE(TypeTag, MaterialLaw) MaterialLaw;
    typedef typename GET_PROP_TYPE(TypeTag, MaterialLawParams) MaterialLawParams;
    typedef typename GET_PROP_TYPE(TypeTag, Indices) Indices;

    // primary variable indices
    enum { cTot0Idx = Indices::cTot0Idx };

    enum { numPhases = GET_PROP_VALUE(TypeTag, NumPhases) };
    enum { numComponents = GET_PROP_VALUE(TypeTag, NumComponents) };

    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef typename GET_PROP_TYPE(TypeTag, FluidSystem) FluidSystem;
    typedef Dumux::NcpFlash<Scalar, FluidSystem> Flash;
    typedef Dune::FieldVector<Scalar, numPhases> PhaseVector;
    typedef Dune::FieldVector<Scalar, numComponents> ComponentVector;
    
public:
    //! The type of the object returned by the fluidState() method
    typedef Dumux::CompositionalFluidState<Scalar, FluidSystem> FluidState;

    /*!
     * \brief Update all quantities for a given control volume.
     */
    void update(const ElementContext &elemCtx,
                int scvIdx,
                int timeIdx)
    {
        ParentType::update(elemCtx,
                           scvIdx,
                           timeIdx);

        asImp_().updateTemperature_(elemCtx, scvIdx, timeIdx);

        const auto &priVars = elemCtx.primaryVars(scvIdx, timeIdx);
        const auto &problem = elemCtx.problem();

        // extract the total molar densities of the components
        ComponentVector cTotal;
        for (int compIdx = 0; compIdx < numComponents; ++compIdx)
            cTotal[compIdx] = priVars[cTot0Idx + compIdx];
        
        typename FluidSystem::ParameterCache paramCache;
        const Implementation *hint = elemCtx.hint(scvIdx, timeIdx);
        if (hint)
            fluidState_.assign(hint->fluidState());
        else
            Flash::guessInitial(fluidState_, paramCache, cTotal);
        
        // compute the phase compositions, densities and pressures
        const MaterialLawParams &materialParams =
            problem.materialLawParams(elemCtx, scvIdx, timeIdx);
        Flash::template solve<MaterialLaw>(fluidState_, paramCache, materialParams, cTotal);

        // set the phase viscosities
        for (int phaseIdx = 0; phaseIdx < numPhases; ++ phaseIdx) {
            Scalar mu = FluidSystem::viscosity(fluidState_, paramCache, phaseIdx);
            fluidState_.setViscosity(phaseIdx, mu);
        }
        
        /////////////
        // calculate the remaining quantities
        /////////////

        // calculate relative permeabilities
        MaterialLaw::relativePermeabilities(relativePermeability_, materialParams, fluidState_);
        Valgrind::CheckDefined(relativePermeability_);

        // energy related quantities
        asImp_().updateEnergy_(paramCache, elemCtx, scvIdx, timeIdx);

#if 0
        for (int phaseIdx = 0; phaseIdx < numPhases; ++phaseIdx) {
            // binary diffusion coefficents
            diffCoeff_[phaseIdx] =
                FluidSystem::binaryDiffusionCoefficient(fluidState_,
                                                        paramCache,
                                                        phaseIdx,
                                                        /*compIdx=*/0,
                                                        /*compIdx=*/1);
            Valgrind::CheckDefined(diffCoeff_[phaseIdx]);
        }
#endif

        // porosity
        porosity_ = problem.porosity(elemCtx, scvIdx, timeIdx);
        Valgrind::CheckDefined(porosity_);

    }

    /*!
     * \brief Returns the phase state for the control-volume.
     */
    const FluidState &fluidState() const
    { return fluidState_; }

    /*!
     * \brief Returns the relative permeability of a given phase
     *        within the control volume.
     *
     * \param phaseIdx The phase index
     */
    Scalar relativePermeability(int phaseIdx) const
    { return relativePermeability_[phaseIdx]; }

    /*!
     * \brief Returns the effective mobility of a given phase within
     *        the control volume.
     *
     * \param phaseIdx The phase index
     */
    Scalar mobility(int phaseIdx) const
    { return relativePermeability(phaseIdx)/fluidState().viscosity(phaseIdx); }

    /*!
     * \brief Returns the average porosity within the control volume.
     */
    Scalar porosity() const
    { return porosity_; }

#if 0
    /*!
     * \brief Returns the binary diffusion coefficients for a phase
     */
    Scalar diffCoeff(int phaseIdx) const
    { return diffCoeff_[phaseIdx]; }
#endif // 0

    /*!
     * \brief Given a fluid state, set the temperature in the primary variables
     */
    template <class FluidState>
    static void setPriVarTemperatures(PrimaryVariables &priVars, const FluidState &fs)
    {}                                    
    
    /*!
     * \brief Set the enthalpy rate per second of a rate vector, .
     */
    static void setEnthalpyRate(RateVector &rateVec, Scalar rate)
    { }

    /*!
     * \brief Given a fluid state, set the enthalpy rate which emerges
     *        from a volumetric rate.
     */
    template <class FluidState>
    static void setEnthalpyRate(RateVector &v,
                                const FluidState &fluidState, 
                                int phaseIdx, 
                                Scalar volume)
    { }
    
protected:
    void updateTemperature_(const ElementContext &elemCtx,
                            int scvIdx,
                            int timeIdx)
    { fluidState_.setTemperature(elemCtx.problem().temperature(elemCtx, scvIdx, timeIdx)); }

    /*!
     * \brief Called by update() to compute the energy related quantities
     */
    void updateEnergy_(typename FluidSystem::ParameterCache &paramCache,
                       const ElementContext &elemCtx,
                       int scvIdx,
                       int timeIdx)
    {
        for (int phaseIdx = 0; phaseIdx < numPhases; ++ phaseIdx) {
            fluidState_.setEnthalpy(phaseIdx, 0);
        }
    }

    Scalar porosity_;        //!< Effective porosity within the control volume
    Scalar relativePermeability_[numPhases]; //!< Relative permeability within the control volume
    //Scalar diffCoeff_[numPhases]; //!< Binary diffusion coefficients for the phases
    FluidState fluidState_;

private:
    Implementation &asImp_()
    { return *static_cast<Implementation*>(this); }

    const Implementation &asImp_() const
    { return *static_cast<const Implementation*>(this); }
};

} // end namepace

#endif
