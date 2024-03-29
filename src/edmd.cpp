/********************************************************************************
 *                                                                              *
 *          Porting the Essential Dynamics/Molecular Dynamics method            *
 *             for large-scale nucleic acid simulations to ARCHER               *
 *                                                                              *
 *                               Zhuowei Si                                     *
 *              EPCC supervisors: Elena Breitmoser, Iain Bethune                *
 *     External supervisor: Charlie Laughton (The University of Nottingham)     *
 *                                                                              *
 *                  MSc in High Performance Computing, EPCC                     *
 *                       The University of Edinburgh                            *
 *                                                                              *
 *******************************************************************************/

/**
 * File:  edmd.cpp
 * Brief: The implementation of the EDMD class functions for ED/MD simulation
 */

#include "edmd.hpp"


EDMD::EDMD(void) {
    
    constants.Boltzmann = 0.002;
    constants.timefac  = 20.455;
    
    dt  = 0.002;
    gamma = 2.0;
    tautp = 0.2;
    
    // Convert time-related parameters to internal units
    dt    *= constants.timefac;
    gamma /= constants.timefac;
    tautp *= constants.timefac;
    
    temperature = 300.0;
    scaled = constants.Boltzmann * temperature;
    
    mole_Cutoff = 30.0;
    atom_Cutoff = 10.0;
    mole_Least  =  5.0;
}



EDMD::~EDMD(void) {}



void EDMD::initialise(double _dt, double _gamma, double _tautp, double _temperature, double _scaled, double _mole_Cutoff, double _atom_Cutoff, double _mole_Least) {
    dt          = _dt;
    gamma       = _gamma;
    tautp       = _tautp;
    temperature = _temperature;
    scaled      = _scaled;
    mole_Cutoff = _mole_Cutoff;
    atom_Cutoff = _atom_Cutoff;
    mole_Least  = _mole_Least ;
}



void EDMD::calculate_ED_Forces(Tetrad* tetrad) {
    
    int i, j;
    double rotmat[9], v[3];
    
    // Allocate memory for temp arrays
    double * temp_Crds = new double [3 * tetrad->num_Atoms];
    double * proj      = new double [tetrad->num_Evecs];
    double ** temp_Frs = Array::allocate_2D_Double_Array(tetrad->num_Atoms, 3);
    double ** avg_Crds = Array::allocate_2D_Double_Array(3, tetrad->num_Atoms);
    double ** crds     = Array::allocate_2D_Double_Array(3, tetrad->num_Atoms);
    
    // Copy average structure & coordinates from tetrads
    for (i = 0; i < tetrad->num_Atoms; i++) {
        avg_Crds[0][i] = tetrad->avg[3 * i];
        avg_Crds[1][i] = tetrad->avg[3*i+1];
        avg_Crds[2][i] = tetrad->avg[3*i+2];
        
        crds[0][i] = tetrad->coordinates[3 * i];
        crds[1][i] = tetrad->coordinates[3*i+1];
        crds[2][i] = tetrad->coordinates[3*i+2];
    }
    
    // Step 1: rotate x into the pcz frame of reference & remove average structure
    CalcRMSDRotationalMatrix((double **) avg_Crds, (double **) crds, tetrad->num_Atoms, rotmat, NULL); // Call QCP functions
    for (i = 0; i < tetrad->num_Atoms; i++) {
        temp_Crds[3 * i] = rotmat[0] * crds[0][i] + rotmat[1] * crds[1][i] + rotmat[2] * crds[2][i] - avg_Crds[0][i];
        temp_Crds[3*i+1] = rotmat[3] * crds[0][i] + rotmat[4] * crds[1][i] + rotmat[5] * crds[2][i] - avg_Crds[1][i];
        temp_Crds[3*i+2] = rotmat[6] * crds[0][i] + rotmat[7] * crds[1][i] + rotmat[8] * crds[2][i]  - avg_Crds[2][i];
        
    }
    
    // Calculate the offset vector of tetrads
    for (v[0] = v[1] = v[2] = 0.0, i = 0; i < tetrad->num_Atoms; i++) {
        v[0] += (tetrad->avg[3 * i] - (rotmat[0] * tetrad->coordinates[3*i] + rotmat[1] * tetrad->coordinates[3*i+1] + rotmat[2] * tetrad->coordinates[3*i+2]));
        v[1] += (tetrad->avg[3*i+1] - (rotmat[3] * tetrad->coordinates[3*i] + rotmat[4] * tetrad->coordinates[3*i+1] + rotmat[5] * tetrad->coordinates[3*i+2]));
        v[2] += (tetrad->avg[3*i+2] - (rotmat[6] * tetrad->coordinates[3*i] + rotmat[7] * tetrad->coordinates[3*i+1] + rotmat[8] * tetrad->coordinates[3*i+2]));
    }
    v[0] /= tetrad->num_Atoms; v[1] /= tetrad->num_Atoms; v[2] /= tetrad->num_Atoms;
    
    
    // Step 2: calculate projections
    for(i = 0; i < tetrad->num_Evecs; i++) {
        for(proj[i] = 0.0, j = 0; j < 3 * tetrad->num_Atoms; j++) {
            proj[i] += tetrad->eigenvectors[i][j] * temp_Crds[j];
        }
    }
    
    // Step 3 & Step 4 done in the same loop
    for (i = 0; i < 3 * tetrad->num_Atoms; i++) {
        tetrad->ED_Forces[i] = 0.0; temp_Crds[i] = tetrad->avg[i];
    }
    
    for (i = 0; i < 3 * tetrad->num_Atoms; i++) {
        for (j = 0; j < tetrad->num_Evecs; j++) {
            // Step 3: re-embed the input coordinates in PC space - a sort of 'shake' procedure.
            //         Ideally this step is not needed, as stuff above should ensure all moves
            //         remain in PC subspace...
            temp_Crds[i] += tetrad->eigenvectors[j][i] * proj[j];
            
            // Step 4: calculate ED forces
            tetrad->ED_Forces[i] -= (tetrad->eigenvectors[j][i] * proj[j] * scaled / tetrad->eigenvalues[j]);
        }
    }
    
    // Step 5 & Step 6 done in a single loop
    for (i = 0; i < tetrad->num_Atoms; i++) {
        
        // Step 5: rotate 'shaken' coordinates back into right frame
        temp_Crds[3*i] -= v[0]; temp_Crds[3*i+1] -= v[1]; temp_Crds[3*i+2] -= v[2];
        tetrad->coordinates[3 * i] = rotmat[0] * temp_Crds[3*i] + rotmat[3] * temp_Crds[3*i+1] + rotmat[6] * temp_Crds[3*i+2];
        tetrad->coordinates[3*i+1] = rotmat[1] * temp_Crds[3*i] + rotmat[4] * temp_Crds[3*i+1] + rotmat[7] * temp_Crds[3*i+2];
        tetrad->coordinates[3*i+2] = rotmat[2] * temp_Crds[3*i] + rotmat[5] * temp_Crds[3*i+1] + rotmat[8] * temp_Crds[3*i+2];
        
        // Step 6: rotate forces back to original orientation of coordinates
        temp_Frs[i][0] = rotmat[0] * tetrad->ED_Forces[3*i] + rotmat[3] * tetrad->ED_Forces[3*i+1] + rotmat[6] * tetrad->ED_Forces[3*i+2];
        temp_Frs[i][1] = rotmat[1] * tetrad->ED_Forces[3*i] + rotmat[4] * tetrad->ED_Forces[3*i+1] + rotmat[7] * tetrad->ED_Forces[3*i+2];
        temp_Frs[i][2] = rotmat[2] * tetrad->ED_Forces[3*i] + rotmat[5] * tetrad->ED_Forces[3*i+1] + rotmat[8] * tetrad->ED_Forces[3*i+2];

    }
    
    // Assign the ED forces from the temporary arrays to tetrads
    for (i = 0; i < tetrad->num_Atoms; i++) {
        tetrad->ED_Forces[3 * i] = temp_Frs[i][0];
        tetrad->ED_Forces[3*i+1] = temp_Frs[i][1];
        tetrad->ED_Forces[3*i+2] = temp_Frs[i][2];
    }

    // Step 7: calculate the 'potential energy' (in units of kT), stroed in last entry of the ED force array of tetrad
    tetrad->ED_Forces[3 * tetrad->num_Atoms] = 0.0;
    for (i = 0; i < tetrad->num_Evecs; i++) {
        tetrad->ED_Forces[3 * tetrad->num_Atoms] += (proj[i] * proj[i] / tetrad->eigenvalues[i]);
    }
    tetrad->ED_Forces[3 * tetrad->num_Atoms] *= 0.5 * scaled; // ED Energy
    
    // Deallocate memory of temp arrays
    Array::deallocate_2D_Double_Array(temp_Frs);
    Array::deallocate_2D_Double_Array(avg_Crds);
    Array::deallocate_2D_Double_Array(crds);
    delete [] temp_Crds;
    delete [] proj;
    
}



void EDMD::calculate_Random_Terms(Tetrad* tetrad, int rank) {
    
    int i;
    static unsigned int RNG_Seed = 13579;
    double random, s = 0.449871, t = -0.386595, a = 0.19600, b = 0.25472;
    double half = 0.5, r1 = 0.27597, r2 = 0.27846, u, v, x, y, q;
    double * noise_Factor = new double[3 * tetrad->num_Atoms];

    // Set the seed for random number generator
    srand((unsigned)((RNG_Seed++) + rank * rank * rank + time(NULL)));
    if (RNG_Seed > 50000000) RNG_Seed = 13579;
    
    // Noise factors, sum(noise_Factor) = 3594.75 when gmma = 2.0
    for (i = 0; i < 3 * tetrad->num_Atoms; i++) {
        noise_Factor[i] = sqrt(2.0 * gamma * scaled * tetrad->masses[i] / dt);
        tetrad->random_Terms[i] = 0.0;
    }
    
    // Calculate random terms;
    for (i = 0; i < 3 * tetrad->num_Atoms; i++) {
        /*
         ! Adapted from the following Fortran 77 code
         !      ALGORITHM 712, COLLECTED ALGORITHMS FROM ACM.
         !      THIS WORK PUBLISHED IN TRANSACTIONS ON MATHEMATICAL SOFTWARE,
         !      VOL. 18, NO. 4, DECEMBER, 1992, PP. 434-435.
         
         !  The function returns a normally distributed pseudo-random
         !  number with zero mean and unit variance.
         
         !  The algorithm uses the ratio of uniforms method of A.J. Kinderman
         !  and J.F. Monahan augmented with quadratic bounding curves.
         */
        {
            // Generate P = (u,v) uniform in rectangle enclosing acceptance region
            while (1) {
                
                u = (double)(rand()/(double)RAND_MAX);
                v = (double)(rand()/(double)RAND_MAX);
                v = 1.7156 * (v - half);
                
                x = u - s;
                y = abs(v) - t;
                q = x*x + y * (a*y - b*x);
                
                if (q < r1) { break; }    // Accept P if inside inner ellipse
                if (q > r2) { continue; } // Reject P if outside outer ellipse
                if (v*v < -4.0 * log(u) * (u*u)) { break; } // Reject P if outside region
            }
            
            // Return ratio of P's coordinates as the normal deviate
            random = v/u;
        }
        
        tetrad->random_Terms[i] = random * noise_Factor[i];
    }
    
    delete []noise_Factor;
    
}



void EDMD::calculate_NB_Forces(Tetrad* t1, Tetrad* t2) {
    
    int i, j;
    double dx, dy, dz, sqdist;
    double a, pair_Force;
    double krep = 100.0; // krep: soft repulsion constant
    double q;            // q: num_atoms vectors of charges
    
    // qfac: electrostatics factor, set up for dd-dielectric constant of 4r, qfac=332.064/4.0,
    //no electrostatics...
    double qfac = 0.0;
    
    // Initialise the NB forces & energies to 0
    for (i = 0 ; i < 3 * t1->num_Atoms + 2; i++) { t1->NB_Forces[i] = 0.0; }
    for (i = 0 ; i < 3 * t2->num_Atoms + 2; i++) { t2->NB_Forces[i] = 0.0; }
    
    for (i = 0; i < t1->num_Atoms; i++) {
        for (j = 0; j < t2->num_Atoms; j++) {
            
            dx = t1->coordinates[3 * i] - t2->coordinates[3 * j];
            dy = t1->coordinates[3*i+1] - t2->coordinates[3*j+1];
            dz = t1->coordinates[3*i+2] - t2->coordinates[3*j+2];
            
            // Avoid div0 (full atom overlap, almost impossible)
            sqdist = max(dx*dx + dy*dy + dz*dz, (double) 1e-9);

            if (sqdist < (atom_Cutoff * atom_Cutoff)) {

                a = max(0.0, (2.0 - sqdist));
                q = t1->abq[3*i+2] * t2->abq[3*j+2];
                
                // NB Energy & Electrostatic Energy
                t1->NB_Forces[3 * t1->num_Atoms]     += 0.25 * krep * a * a;
                t1->NB_Forces[3 * t1->num_Atoms + 1] += 0.5 * qfac * q * sqdist;
                
                // NB forces
                pair_Force = -2.0 * krep * a - 2.0 * qfac * q / (sqdist * sqdist);
                t1->NB_Forces[3 * i] -= dx * pair_Force;
                t1->NB_Forces[3*i+1] -= dy * pair_Force;
                t1->NB_Forces[3*i+2] -= dz * pair_Force;
                
                t2->NB_Forces[3 * j] += dx * pair_Force;
                t2->NB_Forces[3*j+1] += dy * pair_Force;
                t2->NB_Forces[3*j+2] += dz * pair_Force;
            }
            
        }
    }
    
    // NB Energy & Electrostatic Energy, two tetrads are the same.
    t2->NB_Forces[3 * t2->num_Atoms]     = t1->NB_Forces[3 * t1->num_Atoms];
    t2->NB_Forces[3 * t2->num_Atoms + 1] = t1->NB_Forces[3 * t1->num_Atoms + 1];
    
}



void EDMD::update_Velocities(Tetrad* tetrad) {
    
    int i;
    double kentic_Energy = 0.0;
    double target_KE; // The target kinetic energy
    double tscal;     // The Berendsen T-coupling factor
    double gamfac = 1.0 / (1.0 + gamma * dt); // Velocity scale factor
    
    for (i = 0; i < 3 * tetrad->num_Atoms; i++) {
        // Simple Langevin dynamics, gamfac = 0.9960
        tetrad->velocities[i] = (tetrad->velocities[i] + tetrad->ED_Forces[i] * dt + (tetrad->random_Terms[i] + tetrad->NB_Forces[i]) * dt / tetrad->masses[i]) * gamfac;

        // Berendsen temperature control. Calculate the actual kentic energy
        kentic_Energy += 0.5 * tetrad->masses[i] * tetrad->velocities[i] * tetrad->velocities[i];
    }
    
    target_KE = 0.5 * scaled * 3 * tetrad->num_Atoms;
    tscal = sqrt(1.0 + (dt/tautp) * ((target_KE/kentic_Energy) - 1.0));
    
    // Calculate temperature of tetrad
    tetrad->temperature = kentic_Energy * 2 / (constants.Boltzmann * 3 * tetrad->num_Atoms);
    tetrad->temperature *= tscal * tscal;
    
    // Update velocities
    for (i = 0; i < 3 * tetrad->num_Atoms; i++) {
        tetrad->velocities[i] *= tscal;
    }

}




void EDMD::update_Coordinates(Tetrad* tetrad) {
    
    for (int i = 0; i < 3 * tetrad->num_Atoms; i++) {
        tetrad->coordinates[i] += tetrad->velocities[i] * dt;
    }
}

