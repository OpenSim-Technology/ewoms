// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*****************************************************************************
 *   Copyright (C) 2009 by Klaus Mosthaf                                     *
 *   Copyright (C) 2009 by Andreas Lauser                                    *
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
 * \brief Non-isothermal gas injection problem where a gas (e.g. air)
 *        is injected into a fully water saturated medium.
 */
#ifndef DUMUX_WATER_AIR_PROBLEM_HH
#define DUMUX_WATER_AIR_PROBLEM_HH

#include <dumux/material/fluidsystems/h2on2fluidsystem.hh>
#include <dumux/material/fluidstates/immisciblefluidstate.hh>
#include <dumux/boxmodels/2p2cni/2p2cnimodel.hh>
#include <dumux/material/fluidmatrixinteractions/2p/linearmaterial.hh>
#include <dumux/material/fluidmatrixinteractions/2p/regularizedbrookscorey.hh>
#include <dumux/material/fluidmatrixinteractions/2p/efftoabslaw.hh>
#include <dumux/material/fluidmatrixinteractions/mp/2padapter.hh>
#include <dumux/material/heatconduction/somerton.hh>

#include <dune/grid/io/file/dgfparser/dgfug.hh>
#include <dune/grid/io/file/dgfparser/dgfs.hh>
#include <dune/grid/io/file/dgfparser/dgfyasp.hh>
#include <dune/common/fvector.hh>

namespace Dumux
{
template <class TypeTag>
class WaterAirProblem;

namespace Properties
{
NEW_TYPE_TAG(WaterAirProblem, INHERITS_FROM(BoxTwoPTwoCNI));

// Set the grid type
SET_PROP(WaterAirProblem, Grid)
{
    typedef Dune::YaspGrid<2> type;
};

// Set the problem property
SET_PROP(WaterAirProblem, Problem)
{
    typedef Dumux::WaterAirProblem<TypeTag> type;
};
// Set the material Law

SET_PROP(WaterAirProblem, MaterialLaw)
{
private:
    // define the material law which is parameterized by effective
    // saturations
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef RegularizedBrooksCorey<Scalar> EffMaterialLaw;

    // define the material law parameterized by absolute saturations
    typedef EffToAbsLaw<EffMaterialLaw> TwoPMaterialLaw;

    typedef typename GET_PROP_TYPE(TypeTag, FluidSystem) FluidSystem;
    enum { lPhaseIdx = FluidSystem::lPhaseIdx };

public:
    typedef TwoPAdapter<lPhaseIdx, TwoPMaterialLaw> type;
};

// Set the heat conduction law
SET_PROP(WaterAirProblem, HeatConductionLaw)
{
private:
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef typename GET_PROP_TYPE(TypeTag, FluidSystem) FluidSystem;

public:
    // define the material law parameterized by absolute saturations
    typedef Dumux::Somerton<FluidSystem, Scalar> type;
};


// Set the wetting phase
SET_TYPE_PROP(WaterAirProblem, FluidSystem, 
              Dumux::FluidSystems::H2ON2<typename GET_PROP_TYPE(TypeTag, Scalar),
                                         /*complexRelations=*/true>);

// Enable gravity
SET_BOOL_PROP(WaterAirProblem, EnableGravity, true);

// Enable constraints
SET_BOOL_PROP(WaterAirProblem, EnableConstraints, true);

// Use forward differences instead of central differences
SET_INT_PROP(WaterAirProblem, NumericDifferenceMethod, +1);

// Write newton convergence
SET_BOOL_PROP(WaterAirProblem, NewtonWriteConvergence, false);

#if ! HAVE_ISTL_FIXPOINT_CRITERION
// no fixpoint convergence criterion in ISTL -> either use SuperLU or
// print a warning.
#if HAVE_SUPERLU
#warning "ISTL has not been patched to support fixpoint convergence criteria. using SuperLU backend" 
SET_TYPE_PROP(WaterAirProblem, LinearSolver, SuperLUBackend<TypeTag>);
#else
#warning "ISTL has not been patched to support fixpoint convergence criteria"
#warning "and SuperLU backend is not available. The linear solver probably won't converge!"
#endif
#endif 
}


/*!
 * \ingroup TwoPTwoCNIModel
 * \ingroup BoxTestProblems
 * \brief Non-isothermal gas injection problem where a gas (e.g. air)
 *        is injected into a fully water saturated medium. During
 *        buoyancy driven upward migration the gas passes a high
 *        temperature area.
 *
 * The domain is sized 40 m times 40 m. The rectangular area with the
 * increased temperature (380 K) starts at (20 m, 5 m) and ends at (30
 * m, 35 m).
 *
 * For the mass conservation equation neumann boundary conditions are used on
 * the top and on the bottom of the domain, while dirichlet conditions
 * apply on the left and the right boundary.
 * For the energy conservation equation dirichlet boundary conditions are applied
 * on all boundaries.
 *
 * Gas is injected at the bottom boundary from 15 m to 25 m at a rate of
 * 0.001 kg/(s m), the remaining neumann boundaries are no-flow
 * boundaries.
 *
 * At the dirichlet boundaries a hydrostatic pressure, a gas saturation of zero and
 * a geothermal temperature gradient of 0.03 K/m are applied.
 *
 * This problem uses the \ref TwoPTwoCNIModel.
 *
 * This problem should typically be simulated for 300000 s.
 * A good choice for the initial time step size is 1000 s.
 *
 * To run the simulation execute the following line in shell:
 * <tt>./test_2p2cni -parameterFile test_2p2cni.input</tt>
 *  */
template <class TypeTag >
class WaterAirProblem 
    : public GET_PROP_TYPE(TypeTag, BaseProblem)
{
    typedef typename GET_PROP_TYPE(TypeTag, BaseProblem) ParentType;

    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef typename GET_PROP_TYPE(TypeTag, GridView) GridView;
    typedef typename GridView::Grid Grid;

    // copy some indices for convenience
    typedef typename GET_PROP_TYPE(TypeTag, FluidSystem) FluidSystem;
    typedef typename GET_PROP_TYPE(TypeTag, Indices) Indices;
    enum {
        numPhases = FluidSystem::numPhases,

        // energy related indices
        temperatureIdx = Indices::temperatureIdx,
        energyEqIdx = Indices::energyEqIdx,

        // component indices
        H2OIdx = FluidSystem::H2OIdx,
        N2Idx = FluidSystem::N2Idx,

        // phase indices
        lPhaseIdx = FluidSystem::lPhaseIdx,
        gPhaseIdx = FluidSystem::gPhaseIdx,

        // equation indices
        conti0EqIdx = Indices::conti0EqIdx,

        // Grid and world dimension
        dim = GridView::dimension,
        dimWorld = GridView::dimensionworld
    };

    typedef typename GET_PROP_TYPE(TypeTag, RateVector) RateVector;
    typedef typename GET_PROP_TYPE(TypeTag, BoundaryRateVector) BoundaryRateVector;
    typedef typename GET_PROP_TYPE(TypeTag, PrimaryVariables) PrimaryVariables;
    typedef typename GET_PROP_TYPE(TypeTag, Constraints) Constraints;
    typedef typename GET_PROP_TYPE(TypeTag, TimeManager) TimeManager;

    typedef typename GET_PROP_TYPE(TypeTag, MaterialLaw) MaterialLaw;
    typedef typename GET_PROP_TYPE(TypeTag, MaterialLawParams) MaterialLawParams;

    typedef typename GET_PROP_TYPE(TypeTag, HeatConductionLaw) HeatConductionLaw;
    typedef typename GET_PROP_TYPE(TypeTag, HeatConductionLawParams) HeatConductionLawParams;

    typedef typename GridView::ctype CoordScalar;
    typedef Dune::FieldVector<CoordScalar, dimWorld> GlobalPosition;

    typedef Dune::FieldMatrix<Scalar, dimWorld, dimWorld> DimMatrix;

public:
    /*!
     * \brief The constructor
     *
     * \param timeManager The time manager
     * \param gridView The grid view
     */
    WaterAirProblem(TimeManager &timeManager)
        : ParentType(timeManager, GET_PROP_TYPE(TypeTag, GridCreator)::grid().leafView())
    {
        maxDepth_ = 1000.0; // [m]
        eps_ = 1e-6;

        FluidSystem::init(/*Tmin=*/275, /*Tmax=*/600, /*nT=*/100,
                          /*pmin=*/9.5e6, /*pmax=*/10.5e6, /*np=*/200);

        layerBottom_ = 22.0;

        // intrinsic permeabilities
        fineK_ = this->toDimMatrix_(1e-13);
        coarseK_ = this->toDimMatrix_(1e-12);

        // porosities
        finePorosity_ = 0.3;
        coarsePorosity_ = 0.3;

        // residual saturations
        fineMaterialParams_.setSwr(0.2);
        fineMaterialParams_.setSnr(0.0);
        coarseMaterialParams_.setSwr(0.2);
        coarseMaterialParams_.setSnr(0.0);

        // parameters for the Brooks-Corey law
        fineMaterialParams_.setPe(1e4);
        coarseMaterialParams_.setPe(1e4);
        fineMaterialParams_.setLambda(2.0);
        coarseMaterialParams_.setLambda(2.0);

        // parameters for the somerton law of heat conduction
        computeHeatCondParams_(fineHeatCondParams_, finePorosity_);
        computeHeatCondParams_(coarseHeatCondParams_, coarsePorosity_);
    }

    /*!
     * \name Problem parameters
     */
    // \{

    /*!
     * \brief The problem name.
     *
     * This is used as a prefix for files generated by the simulation.
     */
    const char *name() const
    { return "waterair"; }

    /*!
     * \brief Apply the intrinsic permeability tensor to a pressure
     *        potential gradient.
     *
     * \param element The current finite element
     * \param fvElemGeom The current finite volume geometry of the element
     * \param scvIdx The index of the sub-control volume
     */
    template <class Context>
    const DimMatrix &intrinsicPermeability(const Context &context, int spaceIdx, int timeIdx) const
    {
        const GlobalPosition &pos = context.pos(spaceIdx, timeIdx);
        if (isFineMaterial_(pos))
            return fineK_;
        return coarseK_;
    }

    /*!
     * \brief Define the porosity \f$[-]\f$ of the spatial parameters
     *
     * \param element The finite element
     * \param fvElemGeom The finite volume geometry
     * \param scvIdx The local index of the sub-control volume where
     *                    the porosity needs to be defined
     */
    template <class Context>
    Scalar porosity(const Context &context, int spaceIdx, int timeIdx) const
    {
        const GlobalPosition &pos = context.pos(spaceIdx, timeIdx);
        if (isFineMaterial_(pos))
            return finePorosity_;
        else
            return coarsePorosity_;
    }


    /*!
     * \brief return the parameter object for the Brooks-Corey material law which depends on the position
     *
    * \param element The current finite element
    * \param fvElemGeom The current finite volume geometry of the element
    * \param scvIdx The index of the sub-control volume
    */
    template <class Context>
    const MaterialLawParams& materialLawParams(const Context &context, int spaceIdx, int timeIdx) const
    {
        const GlobalPosition &pos = context.pos(spaceIdx, timeIdx);
        if (isFineMaterial_(pos))
            return fineMaterialParams_;
        else
            return coarseMaterialParams_;
    }

    /*!
     * \brief Returns the specific heat capacity \f$[J/(m^3 K)]\f$ of the rock matrix.
     *
     * This is only required for non-isothermal models.
     *
     * \param element The finite element
     * \param fvElemGeom The finite volume geometry
     * \param scvIdx The local index of the sub-control volume where
     *                    the heat capacity needs to be defined
     */
    template <class Context>
    Scalar heatCapacitySolid(const Context &context, int spaceIdx, int timeIdx) const
    {
        return
            790 // specific heat capacity of granite [J / (kg K)]
            * 2700; // density of granite [kg/m^3]
    }

    /*!
     * \brief Return the parameter object for the heat conductivty law
     *        for a given position
     */
    template <class Context>
    const HeatConductionLawParams&
    heatConductionParams(const Context &context, int spaceIdx, int timeIdx) const
    {
        const GlobalPosition &pos = context.pos(spaceIdx, timeIdx);
        if (isFineMaterial_(pos))
            return fineHeatCondParams_;
        return coarseHeatCondParams_;
    }


    template <class Context>
    void source(RateVector &values,
                const Context &context, int spaceIdx, int timeIdx) const
    { values = 0; }

    // \}

    /*!
     * \name Boundary conditions
     */
    // \{

    /*!
     * \brief Evaluate the boundary conditions for a boundary segment.
     */
    template <class Context>
    void boundary(BoundaryRateVector &values,
                  const Context &context,
                  int spaceIdx, int timeIdx) const
    {
        const auto &pos = context.cvCenter(spaceIdx, timeIdx);
        assert(onLeftBoundary_(pos) || 
               onLowerBoundary_(pos) || 
               onRightBoundary_(pos) || 
               onUpperBoundary_(pos));

        if (onInlet_(pos)) {
            RateVector massRate(0.0);
            massRate[conti0EqIdx + N2Idx] = -1e-3; // [kg/(m^2 s)]
            //massRate[conti0EqIdx + H2OIdx] = -1e-2; // [kg/(m^2 s)]

            // impose an forced inflow boundary condition on the inlet
            values.setMassRate(massRate);
        }
        else if (onLeftBoundary_(pos) || onRightBoundary_(pos)) {                 
            //int globalIdx = context.elemContext().globalSpaceIndex(context.insideScvIndex(spaceIdx,timeIdx), timeIdx);

            Dumux::CompositionalFluidState<Scalar, FluidSystem> fs;
            initialFluidState_(fs, context, spaceIdx, timeIdx);

            // impose an freeflow boundary condition
            values.setFreeFlow(context, spaceIdx, timeIdx, fs);
        } 
        else
            // no flow on top and bottom
            values.setNoFlow();
    }

    // \}

    /*!
     * \name Constraints
     */
    // \{

    /*!
     * \brief Set the constraints of this problem.
     *
     * This method sets temperature constraints for the finite volumes
     * adacent to the inlet.
     */
    template <class Context>
    void constraints(Constraints &values,
                     const Context &context,
                     int spaceIdx, int timeIdx) const
    {
        const auto &pos = context.pos(spaceIdx, timeIdx);

        if (onInlet_(pos)) {
            values.setConstraint(temperatureIdx, energyEqIdx, 380);;
        }
    }

    // \}

    /*!
     * \name Volume terms
     */
    // \{

    /*!
     * \brief Evaluate the initial value for a control volume.
     *
     * \param values The initial values for the primary variables
     * \param element The finite element
     * \param fvElemGeom The finite-volume geometry in the box scheme
     * \param scvIdx The local vertex index
     *
     * For this method, the \a values parameter stores primary
     * variables.
     */
    template <class Context>
    void initial(PrimaryVariables &values, const Context &context, int spaceIdx, int timeIdx) const
    {
        //int globalIdx = context.globalSpaceIndex(spaceIdx, timeIdx);

        Dumux::CompositionalFluidState<Scalar, FluidSystem> fs;
        initialFluidState_(fs, context, spaceIdx, timeIdx);
        
        const auto &matParams = materialLawParams(context, spaceIdx, timeIdx);
        values.assignMassConservative(fs, matParams, /*inEquilibrium=*/true);
    }

private:
    bool onLeftBoundary_(const GlobalPosition &pos) const
    { return pos[0] < eps_; }

    bool onRightBoundary_(const GlobalPosition &pos) const
    { return pos[0] > this->bboxMax()[0] - eps_; }

    bool onLowerBoundary_(const GlobalPosition &pos) const
    { return pos[1] < eps_; }

    bool onUpperBoundary_(const GlobalPosition &pos) const
    { return pos[1] > this->bboxMax()[1] - eps_; }

    bool onInlet_(const GlobalPosition &pos) const
    { return onLowerBoundary_(pos) && (15.0 < pos[0]) && (pos[0] < 25.0); }

    bool inHighTemperatureRegion_(const GlobalPosition &pos) const
    { return (20 < pos[0]) && (pos[0] < 30) && (pos[1] < 30); }

    template <class Context, class FluidState>
    void initialFluidState_(FluidState &fs, const Context &context, int spaceIdx, int timeIdx) const
    {
        const GlobalPosition &pos = context.pos(spaceIdx, timeIdx);

        Scalar densityW = 1000.0;
        fs.setPressure(lPhaseIdx, 1e5 + (maxDepth_ - pos[1])*densityW*9.81);
        fs.setSaturation(lPhaseIdx, 1.0);
        fs.setMoleFraction(lPhaseIdx, H2OIdx, 1.0);
        fs.setMoleFraction(lPhaseIdx, N2Idx, 0.0);
       
        if (inHighTemperatureRegion_(pos))
            fs.setTemperature(380);
        else
            fs.setTemperature(283.0 + (maxDepth_ - pos[1])*0.03);

        // set the gas saturation and pressure
        fs.setSaturation(gPhaseIdx, 0);
        Scalar pc[numPhases];
        const auto &matParams = materialLawParams(context, spaceIdx, timeIdx);
        MaterialLaw::capillaryPressures(pc, matParams, fs);
        fs.setPressure(gPhaseIdx, fs.pressure(lPhaseIdx) + (pc[gPhaseIdx] - pc[lPhaseIdx]));

        typename FluidSystem::ParameterCache paramCache;
        typedef Dumux::ComputeFromReferencePhase<Scalar, FluidSystem> CFRP;
        CFRP::solve(fs, paramCache, lPhaseIdx, /*setViscosity=*/false,  /*setEnthalpy=*/true);
    }

    void computeHeatCondParams_(HeatConductionLawParams &params, Scalar poro)
    {            
        Scalar lambdaGranite = 2.8; // [W / (K m)]

        // create a Fluid state which has all phases present
        Dumux::ImmiscibleFluidState<Scalar, FluidSystem> fs;
        fs.setTemperature(293.15);
        for (int phaseIdx = 0; phaseIdx < numPhases; ++phaseIdx) {
            fs.setPressure(phaseIdx, 1.0135e5);
        }

        typename FluidSystem::ParameterCache paramCache;
        paramCache.updateAll(fs);
        for (int phaseIdx = 0; phaseIdx < numPhases; ++phaseIdx) {
            Scalar rho = FluidSystem::density(fs, paramCache, phaseIdx);
            fs.setDensity(phaseIdx, rho);
        }

        for (int phaseIdx = 0; phaseIdx < numPhases; ++phaseIdx) {
            Scalar lambdaSaturated;
            if (FluidSystem::isLiquid(phaseIdx)) {
                Scalar lambdaFluid =
                    FluidSystem::thermalConductivity(fs, paramCache, phaseIdx);
                lambdaSaturated = std::pow(lambdaGranite, (1-poro)) + std::pow(lambdaFluid, poro);
            }
            else
                lambdaSaturated = std::pow(lambdaGranite, (1-poro));
            
            params.setFullySaturatedLambda(phaseIdx, lambdaSaturated);
            if (!FluidSystem::isLiquid(phaseIdx))
                params.setVacuumLambda(lambdaSaturated);
        }
    }

    bool isFineMaterial_(const GlobalPosition &pos) const
    { return pos[dim-1] > layerBottom_; }

    DimMatrix fineK_;
    DimMatrix coarseK_;
    Scalar layerBottom_;

    Scalar finePorosity_;
    Scalar coarsePorosity_;

    MaterialLawParams fineMaterialParams_;
    MaterialLawParams coarseMaterialParams_;

    HeatConductionLawParams fineHeatCondParams_;
    HeatConductionLawParams coarseHeatCondParams_;

    Scalar maxDepth_;
    Scalar eps_;
};
} //end namespace

#endif
