/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
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

#include "twoDEldredgePitchPointPatchVectorField.H"
#include "pointPatchFields.H"
#include "addToRunTimeSelectionTable.H"
#include "Time.H"
#include "polyMesh.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

twoDEldredgePitchDisplacementPointPatchVectorField::
twoDEldredgePitchDisplacementPointPatchVectorField
(
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF
)
:
    fixedValuePointPatchField<vector>(p, iF),
    p0_(p.localPoints()),
    axis_(vector::zero),
    origin_(vector::zero),
	pitch_amplitude_(1.0),
	k_parameter_(1.0),
	a_parameter_(0.5),
	chord_(1.0),
	free_stream_velocity_(1.0)
{}


twoDEldredgePitchDisplacementPointPatchVectorField::
twoDEldredgePitchDisplacementPointPatchVectorField
(
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF,
    const dictionary& dict
)
:
    fixedValuePointPatchField<vector>(p, iF, dict),
    axis_(dict.lookup("axis")),
    origin_(dict.lookup("origin")),
	pitch_amplitude_("pitch_amplitude"),
	k_parameter_("k_parameter"),
	a_parameter_("a_parameter"),
	chord_("chord"),
	free_stream_velocity_("free_stream_velocity")
{
    if (!dict.found("value"))
    {
        updateCoeffs();
    }

    if (dict.found("p0"))
    {
        p0_ = vectorField("p0", dict , p.size());
    }
    else
    {
        p0_ = p.localPoints();
    }
}


twoDEldredgePitchDisplacementPointPatchVectorField::
twoDEldredgePitchDisplacementPointPatchVectorField
(
    const twoDOscillatingDisplacementPointPatchVectorField& ptf,
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF,
    const pointPatchFieldMapper& mapper
)
:
    fixedValuePointPatchField<vector>(ptf, p, iF, mapper),
    axis_(ptf.axis_),
    origin_(ptf.origin_),
    p0_(ptf.p0_, mapper),
	pitch_amplitude_(pft.pitch_amplitude_),
	k_parameter_(pft.k_parameter_),
	a_parameter_(pft.a_parameter_),
	chord_(pft.chord_),
	free_stream_velocity_(pft.free_stream_velocity_)
{}


twoDEldredgePitchDisplacementPointPatchVectorField::
twoDEldredgePitchDisplacementPointPatchVectorField
(
    const twoDOscillatingDisplacementPointPatchVectorField& ptf,
    const DimensionedField<vector, pointMesh>& iF
)
:
    fixedValuePointPatchField<vector>(ptf, iF),
    axis_(ptf.axis_),
    origin_(ptf.origin_),
    p0_(ptf.p0_),
	pitch_amplitude_(pft.pitch_amplitude_),
	k_parameter_(pft.k_parameter_),
	a_parameter_(pft.a_parameter_),
	chord_(pft.chord_),
	free_stream_velocity_(pft.free_stream_velocity_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void twoDEldredgePitchDisplacementPointPatchVectorField::autoMap
(
    const pointPatchFieldMapper& m
)
{
    fixedValuePointPatchField<vector>::autoMap(m);

    p0_.autoMap(m);
}


void twoDEldredgePitchDisplacementPointPatchVectorField::rmap
(
    const pointPatchField<vector>& ptf,
    const labelList& addr
)
{
    const twoDOscillatingDisplacementPointPatchVectorField& aODptf =
        refCast<const twoDOscillatingDisplacementPointPatchVectorField>(ptf);

    fixedValuePointPatchField<vector>::rmap(aODptf, addr);

    p0_.rmap(aODptf.p0_, addr);
}


void twoDEldredgePitchDisplacementPointPatchVectorField::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    const polyMesh& mesh = this->dimensionedInternalField().mesh()();
    const Time& t = mesh.time();

	scalar pi = 3.1415926535897932384626433832795028841971693993751058209;
	scalar sm = pi * pi * k_parameter_ / 
	(2. * pitch_amplitude_ * (1 - a_parameter_);
	scalar t1 = chord_ / free_stream_velocity_;
	scalar t2 = t1 + pitch_amplitude_ / (2 * k_parameter)
		* ( chord_ / free_stream_velocity_ );
	scalar time = t.value();
	
    scalar angle = ((K_parameter / sm) * 
		log(cosh(sm * (time - t1)) / cosh( sm * (time - t2)))) +
		pitch_amplitude_ / 2.0;
    
	vector axisHat = axis_/mag(axis_);
    vectorField p0Rel(p0_ - origin_);

    vectorField::operator=
    (
        p0Rel * (cos(angle) - 1)
      + (axisHat ^ p0Rel * sin(angle))
      + (axisHat & p0Rel) * (1 - cos(angle))*axisHat
    );

    fixedValuePointPatchField<vector>::updateCoeffs();
}


void twoDEldredgePitchDisplacementPointPatchVectorField::write
(
    Ostream& os
) const
{
    pointPatchField<vector>::write(os);
    os.writeKeyword("axis")
        << axis_ << token::END_STATEMENT << nl;
    os.writeKeyword("origin")
        << origin_ << token::END_STATEMENT << nl;
    os.writeKeyword("mean_pitch_angle")
        << mean_pitch_angle_ << token::END_STATEMENT << nl;
    os.writeKeyword("pitch_amplitude")
        << pitch_amplitude_ << token::END_STATEMENT << nl;
    os.writeKeyword("omega")
        << omega_ << token::END_STATEMENT << nl;
    os.writeKeyword("heave_amplitude")
        << heave_amplitude_ << token::END_STATEMENT << nl;
    os.writeKeyword("pitch_phase_offset")
        << pitch_phase_offset_ << token::END_STATEMENT << nl;
    p0_.writeEntry("p0", os);
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePointPatchTypeField
(
    pointPatchVectorField,
    twoDOscillatingDisplacementPointPatchVectorField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //