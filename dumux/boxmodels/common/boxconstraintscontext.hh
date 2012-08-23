// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*****************************************************************************
 *   Copyright (C) 2011-2012 by Andreas Lauser                               *
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
 * \ingroup BoxModel
 *
 * \brief Represents all quantities which available for calculating constraints
 */
#ifndef DUMUX_BOX_CONSTRAINTS_CONTEXT_HH
#define DUMUX_BOX_CONSTRAINTS_CONTEXT_HH

#include "boxproperties.hh"
#include "boxelementcontext.hh"

#include <dune/common/fvector.hh>

namespace Dumux
{

/*!
 * \ingroup BoxModel
 *
 * \brief Represents all quantities which available for calculating constraints
 */
template<class TypeTag>
class BoxConstraintsContext
{
    typedef typename GET_PROP_TYPE(TypeTag, Problem) Problem;
    typedef typename GET_PROP_TYPE(TypeTag, Model) Model;
    typedef typename GET_PROP_TYPE(TypeTag, ElementContext) ElementContext;
    typedef typename GET_PROP_TYPE(TypeTag, GridView) GridView;
    typedef typename GridView::template Codim<0>::Entity Element;

    enum { dimWorld = GridView::dimensionworld };

    typedef typename GridView::ctype CoordScalar;
    typedef Dune::FieldVector<CoordScalar, dimWorld> GlobalPosition;

public:
    /*!
     * \brief The constructor.
     */
    explicit BoxConstraintsContext(const ElementContext &elemCtx)
        : elemCtx_(elemCtx)
    { }

    /*!
     * \copydoc Dumux::BoxElementContext::problem()
     */
    const Problem &problem() const
    { return elemCtx_.problem(); }

    /*!
     * \copydoc Dumux::BoxElementContext::model()
     */
    const Model &model() const
    { return elemCtx_.model(); }

    /*!
     * \copydoc Dumux::BoxElementContext::gridView()
     */
    const GridView &gridView() const
    { return elemCtx_.gridView(); }

    /*!
     * \copydoc Dumux::BoxElementContext::element()
     */
    const Element &element() const
    { return elemCtx_.element(); }

    /*!
     * \copydoc Dumux::BoxElementContext::numScv()
     */
    int numScv() const
    { return elemCtx_.numScv(); }

    /*!
     * \copydoc Dumux::BoxElementContext::numScvf()
     */
    int numScvf() const
    { return elemCtx_.numScvf(); }

    /*!
     * \copydoc Dumux::BoxElementContext::globalSpaceIndex
     */
    int globalSpaceIndex(int scvIdx, int timeIdx) const
    { return elemCtx_.globalSpaceIndex(scvIdx, timeIdx); }

    /*!
     * \copydoc Dumux::BoxElementContext::pos
     */
    const GlobalPosition pos(int scvIdx, int timeIdx) const
    { return elemCtx_.pos(scvIdx, timeIdx); }

protected:
    const ElementContext &elemCtx_;
};

} // namespace Dumux

#endif
