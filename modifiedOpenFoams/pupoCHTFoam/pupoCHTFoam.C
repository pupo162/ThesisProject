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

Application
    chtMultiRegionSimpleFoam

Description
    Steady-state solver for buoyant, turbulent fluid flow and solid heat
    conduction with conjugate heat transfer between solid and fluid regions.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "rhoThermo.H"
#include "turbulentFluidThermoModel.H"
#include "fixedGradientFvPatchFields.H"
#include "regionProperties.H"
#include "solidThermo.H"
#include "radiationModel.H"
#include "fvOptions.H"
#include "coordinateSystem.H"
#include "specie.H"
    
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #define NO_CONTROL
    #define CREATE_MESH createMeshesPostProcess.H
    #include "postProcess.H"

    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMeshes.H"
    #include "createFields.H"
    #include "initContinuityErrs.H"


fvMesh& mesh = fluidRegions[0];

    label inlet;
    label outlet; 

    while (runTime.loop())
    {
        Info<< "Time = " << runTime.timeName() << nl << endl;

        forAll(fluidRegions, i)
        {
            Info<< "\nSolving for fluid region " << fluidRegions[i].name() << endl;
            #include "setRegionFluidFields.H"
            #include "readFluidMultiRegionSIMPLEControls.H"
            #include "solveFluid.H"

            inlet     = mesh.boundaryMesh().findPatchID("Inlet"); 
            outlet    = mesh.boundaryMesh().findPatchID("Outlet");

            Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"  << "  ClockTime = " << runTime.elapsedClockTime() << " s"  << endl;

            Info <<  fluidRegions[i].name() <<" Inflow      : "   << -1.0* gSum( phi.boundaryField()[inlet]  )   <<" [kg/s]" << endl;
            Info <<  fluidRegions[i].name() <<" Outflow     : "   <<       gSum( phi.boundaryField()[outlet] )  <<" [kg/s]" <<  endl;
            Info <<  fluidRegions[i].name() <<" EnergyInflow  : " << -1.0* gSum( phi.boundaryField()[inlet]  * ( thermo.he().boundaryField()[inlet]  +  0.5*magSqr(U.boundaryField()[inlet])  ) )   <<" [W]" <<  endl;
            Info <<  fluidRegions[i].name() <<" EnergyOutflow : " <<       gSum( phi.boundaryField()[outlet] * ( thermo.he().boundaryField()[outlet] +  0.5*magSqr(U.boundaryField()[outlet]) ) )   <<" [W]" <<  endl;   
            Info <<  fluidRegions[i].name() <<" EnergyBalance : " <<       gSum( phi.boundaryField()[outlet] * ( thermo.he().boundaryField()[outlet] +  0.5*magSqr(U.boundaryField()[outlet]) ) ) 
                                                                     +1.0* gSum( phi.boundaryField()[inlet]  * ( thermo.he().boundaryField()[inlet]  +  0.5*magSqr(U.boundaryField()[inlet])  ) )   <<" [W]" <<  endl;
                                                         
            Info<<  fluidRegions[i].name() <<"  rho max/avg/min : " << gMax(thermo.rho())    << " " << gAverage(thermo.rho())    << " " << gMin(thermo.rho())   << endl;
            Info<<  fluidRegions[i].name() <<"  T   max/avg/min : " << gMax(thermo.T())      << " " << gAverage(thermo.T())      << " " << gMin(thermo.T())     << endl;
            Info<<  fluidRegions[i].name() <<"  P   max/avg/min : " << gMax(thermo.p())      << " " << gAverage(thermo.p())      << " " << gMin(thermo.p())     << endl;
            Info<<  fluidRegions[i].name() <<"  Prg max/avg/min : " << gMax(p_rgh)           << " " << gAverage(p_rgh)           << " " << gMin(p_rgh)          << endl;
            Info<<  fluidRegions[i].name() <<"  U   max/avg/min : " << gMax(U).component(2)  << " " << gAverage(U).component(2)  << " " << gMin(U).component(2) << endl;

            //Info<<  fluidRegions[i].name() <<"  K average:  " << gAverage(thermo.kappa())  << endl;
            //Info<<  fluidRegions[i].name() <<"  nu average: " << gAverage(thermo.nu())     << endl;  
        }

        forAll(solidRegions, i)
        {
            Info<< "\nSolving for solid region " << solidRegions[i].name() << endl;
            
            #include "setRegionSolidFields.H"
            #include "readSolidMultiRegionSIMPLEControls.H"
            #include "solveSolid.H"

            Info<< "T"<< solidRegions[i].name() <<"  max/avg/min : " << gMax(thermo.T())   << " " << gAverage(thermo.T())   << " " << gMin(thermo.T())   << endl;
        }

        runTime.write();

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s" << "  ClockTime = " << runTime.elapsedClockTime() << " s" << nl << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
