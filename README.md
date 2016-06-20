### Project information
##### Project title: Porting the Essential Dynamics/Molecular Dynamics method for large-scale nucleic acid simulations to ARCHER 
##### Student name: Zhuowei(James) Si
##### EPCC supervisor name(s): Elena Breitmoser (EPCC), Iain Bethune (EPCC)
##### External supervisor name(s) and organisation: Charlie Laughton (The University of Nottingham)

### Brief Introduction
Essential Dynamics/Molecular Dynamics (EDMD) is a method developed by the Laughton group in Nottingham for efficient simulation of biomolecules.  Rather than computing forces using a classical force-field on an atomistic level as per conventional Molecular Dynamics, EDMD treats sections of the biomolecule such as DNA tetrads as a single entity (coarse-graining), computes forces on each body independently based on a simplified force-field, and then integrates Newton's equations of motion in a reduced space spanned by the Principal Components of the motion (i.e. capturing the Essential Dynamics rather than the full atomistic Molecular Dynamics).
    
Two implementations of the EDMD method exist:
  * A Python version (12,500 lines of code).  This implements the full EDMD method but is serial, slow due to some unoptimised linear transformation operations and force evaluation code, and is contains a lot of irrelevant code imported from an external framework which handles Proteins as well as DNA.
  * A Fortran 95 + MPI version (2,000 lines of code).  This uses a task-farm parallelisation approach for the evaluation of forces on the DNA tetrads, but suffers from some limitations.  The parallelisation is fixed so that the number of MPI ranks must be equal to the number of tetrads + 1 (the master, which implements the dynamics), and has a less sophisticated integrator and user interface compared to the Python code
    
The aim of the project would be to evaluate the two existing codes, then design and implement a new version which combines the best of both worlds: a flexible parallelisation scheme to achieve good load balance on ARCHER, and the improved integrator and user interface from the Python code. The performance of the new code would then be studied on ARCHER and opportunities for future improvements identified.

### Code Introduction
The code has three folders and the configuartion file "config.txt", the "Makefile" and the "README".


### Reference
1. [The QCP rotation calculation method](http://theobald.brandeis.edu/qcp/) in src/qcprot/
  * Douglas L. Theobald (2005), "Rapid calculation of RMSD using a quaternion-based characteristic polynomial.", Acta Crystallographica A 61(4):478-480.
  * Pu Liu, Dmitris K. Agrafiotis, and Douglas L. Theobald (2009), "Fast determination of the optimal rotational matrix for macromolecular superpositions.", in press, Journal of Computational Chemistry 
    
2. The random number generator in `calculate_Random_Forces(Tetrad* tetrad)` in src/edmd.cpp
  * Adapted from the following Fortran 77 code:
    ALGORITHM 712, COLLECTED ALGORITHMS FROM ACM. THIS WORK PUBLISHED IN TRANSACTIONS ON MATHEMATICAL SOFTWARE, VOL. 18, NO. 4, DECEMBER, 1992, PP. 434-435.
  
    [The function returns a normally distributed pseudo-random number with zero mean and unit variance.
    The algorithm uses the ratio of uniforms method of A.J. Kinderman and J.F. Monahan augmented with quadratic bounding curves.]
