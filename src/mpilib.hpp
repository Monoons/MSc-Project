/********************************************************************************
 *                                                                              *
 *          Porting the Essential Dynamics/Molecular Dynamics method            *
 *             for large-scale nucleic acid simulations to ARCHER               *
 *                                                                              *
 *                               Zhuowei Si                                     *
 *              EPCC supervisors: Elena Breitmoser, Iain Bethune                *
 *     External supervisor: Charlie Laughton (The University of Nottingham)     *
 *                                                                              *
 *                 MSc in High Performance Computing, EPCC                      *
 *                      The University of Edinburgh                             *
 *                                                                              *
 *******************************************************************************/

/**
 * File:  mpilib.hpp
 * Brief: Declaration of a class with some specific MPI funcitons
 */

#ifndef projectTest_hpp
#define projectTest_hpp

#include <iostream>
#include <cstddef>
#include "mpi.h"

#include "tetrad.hpp"

// Define the MPI tag for message passing
#define TAG_SIGNAL 1
#define TAG_DATA   2    // For passing parameters (number of tetrads, etc.)
#define TAG_INDEX  3
#define TAG_ED     4    // For message passing of ED forces calculation
#define TAG_NB     7    // For message passing of NB forces calculation
#define TAG_TETRAD 10   // For passing tetrads between master and workers
#define TAG_CLEAN  11
#define TAG_ALL_NB 12

using namespace std;


/**
 * Brief: A class which contains two functions for creating and freeing the MPI data type
 *        for tetrads.
 */
class MPI_Library{
    
public:
    
    /**
     * Function:  Create the MPI_Datatype for tetrads
     *
     * Parameter: MPI_Datatype* MPI_Tetrad -> The MPI data type for tetrad
     *            int num_Tetrads          -> The number of tetrads
     *            Tetrad* tetrad           -> The parameters of all tetrads
     *
     * Return:    None
     */
    static void create_MPI_Tetrad(MPI_Datatype* MPI_Tetrad, int num_Tetrads, Tetrad* tetrad);
    
    /**
     * Function:  Free the MPI_Datatype of tetrads
     *
     * Parameter: MPI_Datatype* MPI_Tetrad -> The MPI data type of tetrads
     *
     * Return:    None
     */
    static void free_MPI_Tetrad(MPI_Datatype* MPI_Tetrad);
    
    /**
     * Function:  Create the MPI_Datatype for tetrads
     *
     * Parameter: MPI_Datatype* MPI_NB -> The MPI data type of NB forces
     *            int num_Tetrads      -> The number of tetrads
     *            Tetrad* tetrad       -> The parameters of all tetrads
     *
     * Return:    None
     */
    static void create_MPI_NB(MPI_Datatype* MPI_NB, int num_Tetrads, Tetrad* tetrad);
    
    /**
     * Function:  Free the MPI_Datatype of tetrads
     *
     * Parameter: MPI_Datatype* MPI_NB -> The MPI data type of NB forces
     *
     * Return:    None
     */
    static void free_MPI_NB(MPI_Datatype* MPI_NB);
    
};


#endif /* projectTest_hpp */

