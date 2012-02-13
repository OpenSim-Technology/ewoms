// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*****************************************************************************
 *   Copyright (C) 2008-2009 by Andreas Lauser                               *
 *   Copyright (C) 2008-2009 by Klaus Mosthaf                                *
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
#ifndef DUMUX_OBSTACLE_SPATIAL_PARAMS_HH
#define DUMUX_OBSTACLE_SPATIAL_PARAMS_HH

#include <dumux/material/spatialparameters/boxspatialparameters.hh>
#include <dumux/material/fluidmatrixinteractions/2p/linearmaterial.hh>
#include <dumux/material/fluidmatrixinteractions/2p/regularizedlinearmaterial.hh>
#include <dumux/material/fluidmatrixinteractions/2p/regularizedbrookscorey.hh>
#include <dumux/material/fluidmatrixinteractions/2p/efftoabslaw.hh>

#include <dumux/boxmodels/mpnc/mpncmodel.hh>
#include <dumux/material/fluidmatrixinteractions/mp/mplinearmaterial.hh>
#include <dumux/material/fluidmatrixinteractions/mp/2padapter.hh>

namespace Dumux
{

//forward declaration
template<class TypeTag>
class ObstacleSpatialParameters;

namespace Properties
{
// The spatial parameters TypeTag
NEW_TYPE_TAG(ObstacleSpatialParameters);

// Set the spatial parameters
SET_TYPE_PROP(ObstacleSpatialParameters, SpatialParameters, Dumux::ObstacleSpatialParameters<TypeTag>);

// Set the material Law
SET_PROP(ObstacleSpatialParameters, MaterialLaw)
{
private:
    typedef typename GET_PROP_TYPE(TypeTag, FluidSystem) FluidSystem;
    enum {
        lPhaseIdx = FluidSystem::lPhaseIdx
    };
    // define the material law
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    //    typedef RegularizedBrooksCorey<Scalar> EffMaterialLaw;
        typedef RegularizedLinearMaterial<Scalar> EffMaterialLaw;
        typedef EffToAbsLaw<EffMaterialLaw> TwoPMaterialLaw;
public:
    typedef TwoPAdapter<lPhaseIdx, TwoPMaterialLaw> type;
};
}

/**
 * \ingroup MPNCModel
 * \ingroup BoxTestProblems
 * \brief Definition of the spatial params properties for the obstacle problem
 *
 */
template<class TypeTag>
class ObstacleSpatialParameters : public BoxSpatialParameters<TypeTag>
{
    typedef BoxSpatialParameters<TypeTag> ParentType;
    typedef typename GET_PROP_TYPE(TypeTag, Grid) Grid;
    typedef typename GET_PROP_TYPE(TypeTag, GridView) GridView;
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef typename GET_PROP_TYPE(TypeTag, SolutionVector) SolutionVector;
    typedef typename GET_PROP_TYPE(TypeTag, FVElementGeometry) FVElementGeometry;
    typedef typename GET_PROP_TYPE(TypeTag, FluidSystem) FluidSystem;

    typedef typename Grid::ctype CoordScalar;
    enum {
        dim=GridView::dimension,
        dimWorld=GridView::dimensionworld
    };

    enum {
        lPhaseIdx = FluidSystem::lPhaseIdx
    };

    typedef typename GridView::template Codim<0>::Entity Element;
    typedef Dune::FieldVector<CoordScalar,dimWorld> GlobalPosition;

public:
    typedef typename GET_PROP_TYPE(TypeTag, MaterialLaw) MaterialLaw;
    typedef typename MaterialLaw::Params MaterialLawParams;

    ObstacleSpatialParameters(const GridView &gv)
        : ParentType(gv)
    {
        // intrinsic permeabilities
        coarseK_ = 1e-12;
        fineK_ = 1e-15;

        // the porosity
        porosity_ = 0.3;

        // residual saturations
        fineMaterialParams_.setSwr(0.0);
        fineMaterialParams_.setSnr(0.0);
        coarseMaterialParams_.setSwr(0.0);
        coarseMaterialParams_.setSnr(0.0);

        // parameters for the linear law, i.e. minimum and maximum
        // pressures
        fineMaterialParams_.setEntryPC(0.0);
        coarseMaterialParams_.setEntryPC(0.0);
        fineMaterialParams_.setMaxPC(0.0);
        coarseMaterialParams_.setMaxPC(0.0);

        /*
        // entry pressures for Brooks-Corey
        fineMaterialParams_.setPe(5e3);
        coarseMaterialParams_.setPe(1e3);

        // Brooks-Corey shape parameters
        fineMaterialParams_.setLambda(2);
        coarseMaterialParams_.setLambda(2);
        */
    }

    ~ObstacleSpatialParameters()
    {}

    /*!
     * \brief Update the spatial parameters with the flow solution
     *        after a timestep.
     *
     * \param globalSol The current solution vector
     */
    void update(const SolutionVector &globalSol)
    {
    };

    /*!
     * \brief Returns the intrinsic permeability tensor.
     *
     * \param element       The current finite element
     * \param fvElemGeom    The current finite volume geometry of the element
     * \param scvIdx        The index sub-control volume where the
     *                      intrinsic permeability is given.
     */
    Scalar intrinsicPermeability(const Element &element,
                                 const FVElementGeometry &fvElemGeom,
                                 int scvIdx) const
    {
        if (isFineMaterial_(fvElemGeom.subContVol[scvIdx].global))
            return fineK_;
        return coarseK_;
    }

    /*!
     * \brief Define the porosity \f$[-]\f$ of the soil
     *
     * \param element     The finite element
     * \param fvElemGeom  The finite volume geometry
     * \param scvIdx      The local index of the sub-control volume where
     *                    the porosity needs to be defined
     */
    double porosity(const Element &element,
                    const FVElementGeometry &fvElemGeom,
                    int scvIdx) const
    {
        return porosity_;
    }

    /*!
     * \brief Returns the heat capacity \f$[J/m^3 K]\f$ of the rock matrix.
     *
     * This is only required for non-isothermal models.
     *
     * \param element     The finite element
     * \param fvElemGeom  The finite volume geometry
     * \param scvIdx      The local index of the sub-control volume where
     *                    the heat capacity needs to be defined
     */
    double heatCapacity(const Element &element,
                        const FVElementGeometry &fvElemGeom,
                        int scvIdx) const
    {
        return 790. ;  // specific heat capacity of granite [J / (kg K)]
    }

//    /* IF COMMENTING IN, PUT A ! FOR DOXYGEN
//     * \brief Calculate the heat flux \f$[W/m^2]\f$ through the
//     *        rock matrix based on the temperature gradient \f$[K / m]\f$
//     *
//     * This is only required for non-isothermal models.
//     *
//     * \param heatFlux    The result vector
//     * \param tempGrad    The temperature gradient
//     * \param element     The current finite element
//     * \param fvElemGeom  The finite volume geometry of the current element
//     * \param scvfIdx     The local index of the sub-control volume face where
//     *                    the matrix heat flux should be calculated
//     */
//    void matrixHeatFlux(Vector &heatFlux,
//                        const FluxVariables &fluxDat,
//                        const ElementVolumeVariables &vDat,
//                        const Vector &tempGrad,
//                        const Element &element,
//                        const FVElementGeometry &fvElemGeom,
//                        int scvfIdx) const
//    {
//        static const Scalar lWater = 0.6;   // [W / (m K ) ]
//        static const Scalar lGranite = 2.8; // [W / (m K ) ]
//
//        // arithmetic mean of the liquid saturation and the porosity
//        const int i = fvElemGeom.subContVolFace[scvfIdx].i;
//        const int j = fvElemGeom.subContVolFace[scvfIdx].j;
//        Scalar Sl = std::max(0.0, (vDat[i].saturation(lPhaseIdx) +
//                                     vDat[j].saturation(lPhaseIdx)) / 2);
//        Scalar poro = (porosity(element, fvElemGeom, i) +
//                       porosity(element, fvElemGeom, j)) / 2;
//
//        Scalar lsat = pow(lGranite, (1-poro)) * pow(lWater, poro);
//        Scalar ldry = pow(lGranite, (1-poro));
//
//        // the heat conductivity of the matrix. in general this is a
//        // tensorial value, but we assume isotropic heat conductivity.
//        Scalar heatCond = ldry + sqrt(Sl) * (ldry - lsat);
//
//        // the matrix heat flux is the negative temperature gradient
//        // times the heat conductivity.
//        heatFlux = tempGrad;
//        heatFlux *= -heatCond;
//    }

    /*!
     * \brief Function for defining the parameters needed by constitutive relationships (kr-Sw, pc-Sw, etc.).
     *
     * \param pos The global position of the sub-control volume.
     * \return the material parameters object
     */
    const MaterialLawParams& materialLawParamsAtPos(const GlobalPosition &pos) const
    {
        if (isFineMaterial_(pos))
            return fineMaterialParams_;
        else
            return coarseMaterialParams_;
    }

    Scalar soilDensity(const Element &element,
                       const FVElementGeometry &fvElemGeom,
                       int scvIdx) const
    {
        return 2700. ; // density of granite [kg/m^3]
    }

    Scalar soilThermalConductivity(const Element &element,
                                   const FVElementGeometry &fvElemGeom,
                                   int scvIdx) const
    {
        return 2.8; // conductivity of granite [W / (m K ) ]
    }

private:
    /*!
     * \brief Returns whether a given global position is in the
     *        fine-permeability region or not.
     */
    static bool isFineMaterial_(const GlobalPosition &pos)
    {
        return
            10 <= pos[0] && pos[0] <= 20 &&
            0 <= pos[1] && pos[1] <= 35;
    };

    Scalar coarseK_;
    Scalar fineK_;
    Scalar porosity_;
    MaterialLawParams fineMaterialParams_;
    MaterialLawParams coarseMaterialParams_;
};

}

#endif