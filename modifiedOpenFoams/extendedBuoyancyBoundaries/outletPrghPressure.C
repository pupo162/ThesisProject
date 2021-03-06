/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2016 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.
    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.
\*---------------------------------------------------------------------------*/

#include "outletPrghPressure.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "surfaceFields.H"
#include "addToRunTimeSelectionTable.H"
#include "fvCFD.H"

#include "uniformDimensionedFields.H"
// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::outletPrghPressure::outletPrghPressure
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedGradientFvPatchScalarField(p, iF)
{}


Foam::outletPrghPressure::outletPrghPressure
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedGradientFvPatchScalarField(p, iF)
{
    if (dict.found("value") && dict.found("gradient"))
    {
         fvPatchField<scalar>::operator=
         (
             scalarField("value", dict, p.size())
         );
         gradient() = scalarField("gradient", dict, p.size());
    }
    else
    {
        const fvPatchField<scalar>& rhop =
            patch().lookupPatchField<volScalarField, scalar>("rho"); 

        const fvPatchField<scalar>& psip =
            patch().lookupPatchField<volScalarField, scalar>("psi"); 

        const scalarField& ghfp =
            patch().lookupPatchField<surfaceScalarField, scalar>("ghf");   

        const fvPatchField<scalar>& pp =
            patch().lookupPatchField<volScalarField, scalar>("p");

        fvPatchField<scalar>::operator=( 
            pp.patchInternalField() - rhop * ghfp
        );

        //gradient() = -1.0 * rhop.snGrad() * ghfp;
        gradient() = -1.0 * ghfp * psip * rhop * -9.81 ; 
    }
}


Foam::outletPrghPressure::outletPrghPressure
(
    const outletPrghPressure& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedGradientFvPatchScalarField(p, iF)
{
    patchType() = ptf.patchType();

    // Map gradient. Set unmapped values and overwrite with mapped ptf
    gradient() = 0.0;
    gradient().map(ptf.gradient(), mapper);

    // Evaluate the value field from the gradient if the internal field is valid
    if (notNull(iF) && iF.size())
    {
        scalarField::operator=
        (
            //patchInternalField() + gradient()/patch().deltaCoeffs()
            // ***HGW Hack to avoid the construction of mesh.deltaCoeffs
            // which fails for AMI patches for some mapping operations
            patchInternalField() + gradient()*(patch().nf() & patch().delta())
        );
    }
    else
    {
        // Enforce mapping of values so we have a valid starting value. This
        // constructor is used when reconstructing fields
        this->map(ptf, mapper);
    }
}


Foam::outletPrghPressure::outletPrghPressure
(
    const outletPrghPressure& wbppsf
)
:
    fixedGradientFvPatchScalarField(wbppsf)
{}


Foam::outletPrghPressure::outletPrghPressure
(
    const outletPrghPressure& wbppsf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedGradientFvPatchScalarField(wbppsf, iF)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::outletPrghPressure::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    const fvPatchField<scalar>& rhop =
        patch().lookupPatchField<volScalarField, scalar>("rho"); 

    const fvPatchField<scalar>& psip =
        patch().lookupPatchField<volScalarField, scalar>("psi"); 

    const scalarField& ghfp =
        patch().lookupPatchField<surfaceScalarField, scalar>("ghf");

    //gradient() =  -1.0 * rhop.snGrad() * ghfp; 

    gradient() =  -1.0 * ghfp * psip * rhop * -9.81 ;
}

void Foam::outletPrghPressure::write(Ostream& os) const
{
    fixedGradientFvPatchScalarField::write(os);
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
namespace Foam
{
    makePatchTypeField
    (
        fvPatchScalarField,
        outletPrghPressure
    );
}

// ************************************************************************* //