/*
 * This the header file of the Tetrad class, it contains 
 * some basic charasteristics of tetrads.
 */

#ifndef tetrad_hpp
#define tetrad_hpp

#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

/* Initial parameters of tetrads are read from "prm" file */

typedef struct _Tetrad {
    
    int num_Atoms_In_Tetrad; // The number of atoms in tetrads
    
    int num_Evecs;           // The number of eigenvectors & eigenvalues
    
    float *avg_Structure;   // The average structure (coordinates) of tetrad
    
    float *masses;          // The masses of atoms of tetrads
    
    float *abq;             // The abq
    
    float *eigenvalues;     // The eigenvalues
    
    float **eigenvectors;   // The eigenvectors
    
    float *velocities;      // The velocities of tetrads
    
    float *coordinates;     // The coordinates of tetrads
    
    float *ED_Forces;
    
    float *random_Forces;
    
    float *NB_Forces;
    
}Tetrad;

/*
class Tetrad {
    
public:
    
    int num_Atoms_In_Tetrad; // The number of atoms in tetrads
    
    int num_Evecs;           // The number of eigenvectors & eigenvalues
    
    float *avg_Structure;   // The average structure (coordinates) of tetrad
    
    float *masses;          // The masses of atoms of tetrads
    
    float *abq;             // The abq
    
    float *eigenvalues;     // The eigenvalues
    
    float **eigenvectors;   // The eigenvectors
    
    float *velocities;      // The velocities of tetrads
    
    float *coordinates;     // The coordinates of tetrads
    
    float *ED_Forces;
    
    float *random_Forces;
    
    float *NB_Forces;
    
public:
    
    Tetrad(void);
    
    ~Tetrad(void);
};*/



#endif /* tetrad_hpp */
