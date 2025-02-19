#ifndef LINPROG_H
#define LINPROG_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lp_data/HighsCallbackStruct.h"

// Define infinity as a large number
#define HIGHS_CONST_INF INFINITY

int linprog(HighsInt num_vars,
            HighsInt num_constraints_ub,
            HighsInt num_constraints_eq,
            double* A_ub,
            double* b_ub,
            double* A_eq,
            double* b_eq,
            double* col_lower,
            double* col_upper,
            double* solution,
            double* objective_value,
            double* row_duals,   // Dual variables for constraints
            double sign // A*sign <= b*sign
            );

int linprog_mip(HighsInt num_vars,
            HighsInt num_constraints_ub,
            HighsInt num_constraints_eq,
            double* c,
            double* A_ub,
            double* b_lb,
            double* b_ub,
            double* A_eq,
            double* b_eq,
            double* lb,
            double* ub,
            double* solution,
            double* objective_value,
            HighsInt *integrality
            );

#endif  // LINPROG_H
