#include "highs_c_api.h"
#include "linprog.h"

extern double time_limit;
extern int verbose;

/* todo: catch ctr-c using callbacks */

int linprog_mip(HighsInt num_vars,
            HighsInt num_constraints_ub,
            HighsInt num_constraints_eq,
            double* obj,
            double* A_ub,
            double* b_lb,
            double* b_ub,
            double* A_eq,
            double* b_eq,
            double* col_lower,
            double* col_upper,
            double* solution,
            double* objective_value,
            HighsInt *integrality
            ) {
    HighsInt i, j, status, num_nz, nz;
    int success = 1;
    HighsInt a_format = 1; /* 1 for column-wise (CSC) format */
    HighsInt sense = 1;    /* 1 for minimization */

    void* highs = Highs_create();

    HighsInt num_constraints = num_constraints_ub + num_constraints_eq;

    /* Initialize constraint matrix in CSC format */
    num_nz = 0;
    if (A_ub != NULL) {
        for (i = 0; i < num_constraints_ub; i++) {
            for (j = 0; j < num_vars; j++)
                if (fabs(A_ub[i * num_vars + j]) <= 1e-8)
                    A_ub[i * num_vars + j] = 0.0;
            for (j = 0; j < num_vars; j++) {
                if (A_ub[i * num_vars + j] != 0.0) num_nz++;
            }
        }
    }
    if (A_eq != NULL) {
        for (i = 0; i < num_constraints_eq; i++) {
            for (j = 0; j < num_vars; j++)
                if (fabs(A_eq[i * num_vars + j]) <= 1e-8)
                    A_eq[i * num_vars + j] = 0.0;
            for (j = 0; j < num_vars; j++) {
                if (A_eq[i * num_vars + j] != 0.0) num_nz++;
            }
        }
    }

    HighsInt* Astart = (HighsInt*)malloc(sizeof(HighsInt) * (num_vars + 1));
    HighsInt* Aindex = (HighsInt*)malloc(sizeof(HighsInt) * num_nz);
    double* Avalue = (double*)malloc(sizeof(double) * num_nz);

    nz = 0;
    for (j = 0; j < num_vars; j++) {
        Astart[j] = nz;
        if (A_ub != NULL) {
            for (i = 0; i < num_constraints_ub; i++) {
                double val = A_ub[i * num_vars + j];
                if (val != 0.0) {
                    Aindex[nz] = i;
                    Avalue[nz] = val;
                    nz++;
                }
            }
        }
        if (A_eq != NULL) {
            for (i = 0; i < num_constraints_eq; i++) {
                double val = A_eq[i * num_vars + j];
                if (val != 0.0) {
                    Aindex[nz] = num_constraints_ub + i;
                    Avalue[nz] = val;
                    nz++;
                }
            }
        }
    }
    Astart[num_vars] = nz;

    /* Initialize row bounds */
    double* row_lower = (double*)malloc(sizeof(double) * num_constraints);
    double* row_upper = (double*)malloc(sizeof(double) * num_constraints);

    /* Inequality constraints (A_ub x ≤ b_ub) */
    for (HighsInt i = 0; i < num_constraints_ub; i++) {
        row_lower[i] = b_lb[i];
        row_upper[i] = b_ub[i];
    }
    /* Equality constraints (A_eq x = b_eq) */
    for (HighsInt i = 0; i < num_constraints_eq; i++) {
        row_lower[num_constraints_ub + i] = b_eq[i];
        row_upper[num_constraints_ub + i] = b_eq[i];
    }

    /* Load the problem (LP or MIP based on integrality) */
    status = Highs_passMip(highs,
                               num_vars,            
                               num_constraints,     
                               num_nz,              
                               a_format,           
                               sense,               
                               0.0,                /* obj offset */
                               obj,                 
                               col_lower,           
                               col_upper,           
                               row_lower,           
                               row_upper,           
                               Astart,              
                               Aindex,              
                               Avalue,              
                               integrality
                               );
    #ifdef DEBUG    
    Highs_setBoolOptionValue(highs, "output_flag", verbose);
    Highs_setBoolOptionValue(highs, "log_to_console", verbose);
    #else
    Highs_setBoolOptionValue(highs, "output_flag", 0);
    Highs_setBoolOptionValue(highs, "log_to_console", 0);
    #endif
    Highs_setDoubleOptionValue(highs, "time_limit", time_limit);

    status = Highs_run(highs);
    if (status != 0) {
        printf("MIP HiGHS error in problem loading: %lld\n", (long long)status);
        Highs_destroy(highs);
        return status;
    }
    else {
        int model_status = Highs_getModelStatus(highs);
        if (model_status == kHighsModelStatusOptimal) {
            #ifdef DEBUG
            printf("--->Solution is feasible and optimal.\n");
            #endif
            success = 0;
        } 
        #ifdef DEBUG
        else if (model_status == kHighsModelStatusInfeasible) {
            printf("--->The model is infeasible.\n");
        } else if (model_status == kHighsModelStatusUnbounded) {
            printf("--->The model is unbounded.\n");
        } else if (model_status == kHighsModelStatusUnboundedOrInfeasible) {
            printf("--->The model might be unbounded or infeasible.\n");
        } else {
            printf("--->Unknown or unset model status: %d\n", model_status);
        }
        #endif
    }
    if (success == 0) {
        Highs_getSolution(highs, solution, NULL, NULL, NULL);
        Highs_getDoubleInfoValue(highs, "objective_function_value", objective_value);
    }
    free(Astart);
    free(Aindex);
    free(Avalue);
    free(row_lower);
    free(row_upper);
    Highs_destroy(highs);
    return success;
}

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
            ) {
    void* highs = Highs_create();

    HighsInt num_constraints = num_constraints_ub + num_constraints_eq;
    HighsInt i, j, nz, num_nz = 0;
    double val;
    int success = 1;

    // Initialize objective function coefficients
    double* obj = (double*)malloc(sizeof(double) * num_vars);
    for (i = 0; i < num_vars; i++) {
        obj[i] = 1.0;
    }

    // Initialize constraint matrix in CSC format
    if (A_ub != NULL) {
        for (i = 0; i < num_constraints_ub; i++) {
            for (j = 0; j < num_vars; j++) {
                if (fabs(A_ub[i * num_vars + j]) <= 1e-8)
                    A_ub[i * num_vars + j] = 0.0;
                if (A_ub[i * num_vars + j] != 0.0) 
                    num_nz++;
            }
        }
    }
    if (A_eq != NULL) {
        for (i = 0; i < num_constraints_eq; i++) {
            for (j = 0; j < num_vars; j++) {
                if (A_eq[i * num_vars + j] != 0.0) num_nz++;
            }
        }
    }

    HighsInt* Astart = (HighsInt*)malloc(sizeof(HighsInt) * (num_vars + 1));
    HighsInt* Aindex = (HighsInt*)malloc(sizeof(HighsInt) * num_nz);
    double* Avalue = (double*)malloc(sizeof(double) * num_nz);

    nz = 0;
    //(*A_ub_out)[i * num_cols + k * num_patterns + j]
    for (j = 0; j < num_vars; j++) {
        Astart[j] = nz;
        if (A_ub != NULL) {
            for (i = 0; i < num_constraints_ub; i++) {
                val = A_ub[i * num_vars + j];
                if (val != 0.0) {
                    Aindex[nz] = i;
                    Avalue[nz] = val;
                    nz++;
                }
            }
        }
        if (A_eq != NULL) {
            for (HighsInt i = 0; i < num_constraints_eq; i++) {
                val = A_eq[i * num_vars + j];
                if (val != 0.0) {
                    Aindex[nz] = num_constraints_ub + i;
                    Avalue[nz] = val;
                    nz++;
                }
            }
        }
    }
    Astart[num_vars] = nz;

    // Initialize row bounds
    double* row_lower = (double*)malloc(sizeof(double) * num_constraints);
    double* row_upper = (double*)malloc(sizeof(double) * num_constraints);

    // Inequality constraints (A_ub x ≤ b_ub)
    for (i = 0; i < num_constraints_ub; i++) {
        if (sign < 0) {
            row_lower[i] = b_ub[i];
            row_upper[i] = HIGHS_CONST_INF;
        } else {
            row_lower[i] = -HIGHS_CONST_INF;
            row_upper[i] = b_ub[i];
        }
    }
    // Equality constraints (A_eq x = b_eq)
    for (i = 0; i < num_constraints_eq; i++) {
        row_lower[num_constraints_ub + i] = b_eq[i];
        row_upper[num_constraints_ub + i] = b_eq[i];
    }

    // Define additional parameters
    HighsInt a_format = 1; // 1 for column-wise (CSC) format
    HighsInt sense = 1;    // 1 for minimization
    double offset = 0.0;   // No offset in objective function
    Highs_setBoolOptionValue(highs, "output_flag", 0);
    Highs_setBoolOptionValue(highs, "log_to_console", 0);
    #ifdef DEBUG
    Highs_setBoolOptionValue(highs, "output_flag", 1);
    #endif
    // Load the problem
    HighsInt status = Highs_passLp(highs,
                                   num_vars,            // num_col
                                   num_constraints,     // num_row
                                   nz,                  // num_nz
                                   a_format,            // a_format
                                   sense,               // sense
                                   offset,              // offset
                                   obj,                 // col_cost
                                   col_lower,           // col_lower
                                   col_upper,           // col_upper
                                   row_lower,           // row_lower
                                   row_upper,           // row_upper
                                   Astart,              // Astart (pointer)
                                   Aindex,              // Aindex (pointer)
                                   Avalue               // Avalue (pointer)
                                   );
    
   // Run the solver
    status = Highs_run(highs);

    if (status != 0) {
        printf("HiGHS error in Highs_passLp: %lld\n", (long long)status);
        Highs_destroy(highs);
        return status;
    }
    else {
        int model_status = Highs_getModelStatus(highs);
        if (model_status == kHighsModelStatusOptimal) {
            #ifdef DEBUG
            printf("====>Solution is feasible and optimal.\n");
            #endif
            success = 0;
        } 
        #ifdef DEBUG
        else if (model_status == kHighsModelStatusInfeasible) {
            printf("====>The model is infeasible.\n");
        } else if (model_status == kHighsModelStatusUnbounded) {
            printf("====>The model is unbounded.\n");
        } else if (model_status == kHighsModelStatusUnboundedOrInfeasible) {
            printf("====>The model might be unbounded or infeasible.\n");
        } else {
            printf("====>Unknown or unset model status: %d\n", model_status);
        }
        #endif
    }

    // Allocate arrays to retrieve duals if not NULL
    double* col_values = solution;

    // Retrieve the solution
    Highs_getSolution(highs, col_values, NULL, NULL, row_duals);
    Highs_getDoubleInfoValue(highs, "objective_function_value", objective_value);

    // Clean up
    free(obj);
    free(Astart);
    free(Aindex);
    free(Avalue);
    free(row_lower);
    free(row_upper);
    Highs_destroy(highs);
    return success; // Success
}
