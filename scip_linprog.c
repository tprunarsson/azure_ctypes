#include "scip_linprog.h"
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <stdlib.h>
#include <stdio.h>

extern double time_limit;
extern int verbose;

int scip_linprog_mip(
    int num_vars,
    int num_constraints_ub,
    int num_constraints_eq,
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
    int* integrality
) {
    SCIP* scip = NULL;
    SCIP_RETCODE retcode;
    int success = 1;

    // Initialize SCIP
    retcode = SCIPcreate(&scip);
    if (retcode != SCIP_OKAY) {
        fprintf(stderr, "Error initializing SCIP\n");
        return 1;
    }

    // Include default plugins
    SCIPincludeDefaultPlugins(scip);

    retcode = SCIPsetRealParam(scip, "limits/time", time_limit);
    if (retcode != SCIP_OKAY) {
        fprintf(stderr, "Error setting time limit for SCIP\n");
        return 1;
    }

    // Set the verbosity level to 0 (no output)
    if (verbose == 0) {
        retcode = SCIPsetIntParam(scip, "display/verblevel", 0);
        if (retcode != SCIP_OKAY) {
            fprintf(stderr, "Error setting verbosity level.\n");
            return retcode;
        }
    }

    // Create the problem
    retcode = SCIPcreateProbBasic(scip, "SCIP_MIP");
    if (retcode != SCIP_OKAY) {
        fprintf(stderr, "Error creating SCIP problem\n");
        return 1;
    }


    // Add variables
    SCIP_VAR** vars = (SCIP_VAR**)malloc(num_vars * sizeof(SCIP_VAR*));
    if (!vars) {
        fprintf(stderr, "Memory allocation error for variables.\n");
        return 1;
    }


    for (int i = 0; i < num_vars; ++i) {
        SCIP_VARTYPE vartype = integrality[i] ? SCIP_VARTYPE_INTEGER : SCIP_VARTYPE_CONTINUOUS;
        retcode = SCIPcreateVarBasic(scip, &vars[i], NULL, col_lower[i], col_upper[i], obj[i], vartype);
        if (retcode != SCIP_OKAY) {
            fprintf(stderr, "Error creating variable %d\n", i);
            return 1;
        }
        retcode = SCIPaddVar(scip, vars[i]);
        if (retcode != SCIP_OKAY) {
            fprintf(stderr, "Error adding variable %d to SCIP\n", i);
            return 1;
        }
    }

    // Add inequality constraints (A_ub x <= b_ub)
    for (int i = 0; i < num_constraints_ub; ++i) {
        SCIP_CONS* cons = NULL;
	char name[50];
        snprintf(name, sizeof(name), "constraint_ub_%d", i);
        retcode = SCIPcreateConsBasicLinear(scip, &cons, name, 0, NULL, NULL, b_lb[i], b_ub[i]);
        if (retcode != SCIP_OKAY) {
            fprintf(stderr, "Error creating inequality constraint %d\n", i);
            return 1;
        }
        for (int j = 0; j < num_vars; ++j) {
            if (A_ub[i * num_vars + j] != 0.0) {
                retcode = SCIPaddCoefLinear(scip, cons, vars[j], A_ub[i * num_vars + j]);
                if (retcode != SCIP_OKAY) {
                    fprintf(stderr, "Error adding coefficient to inequality constraint %d\n", i);
                    return 1;
                }
            }
        }
        retcode = SCIPaddCons(scip, cons);
        SCIPreleaseCons(scip, &cons);
    }



    // Add equality constraints (A_eq x = b_eq)
    for (int i = 0; i < num_constraints_eq; ++i) {
        SCIP_CONS* cons = NULL;
	char name[50];
	snprintf(name, sizeof(name), "constraint_eq_%d", i);
        retcode = SCIPcreateConsBasicLinear(scip, &cons, name, 0, NULL, NULL, b_eq[i], b_eq[i]);
        if (retcode != SCIP_OKAY) {
            fprintf(stderr, "Error creating equality constraint %d\n", i);
            return 1;
        }
        for (int j = 0; j < num_vars; ++j) {
            if (A_eq[i * num_vars + j] != 0.0) {
                retcode = SCIPaddCoefLinear(scip, cons, vars[j], A_eq[i * num_vars + j]);
                if (retcode != SCIP_OKAY) {
                    fprintf(stderr, "Error adding coefficient to equality constraint %d\n", i);
                    return 1;
                }
            }
        }
        retcode = SCIPaddCons(scip, cons);
        SCIPreleaseCons(scip, &cons);
    }

    // Solve the problem
    retcode = SCIPsolve(scip);
    if (retcode != SCIP_OKAY) {
        fprintf(stderr, "Error solving SCIP problem\n");
        return 1;
    }

    // Retrieve solution
    SCIP_SOL* sol = SCIPgetBestSol(scip);
    if (sol) {
        for (int i = 0; i < num_vars; ++i) {
            solution[i] = SCIPgetSolVal(scip, sol, vars[i]);
        }
        *objective_value = SCIPgetSolOrigObj(scip, sol);
        success = 0;
    } else {
        fprintf(stderr, "No solution found.\n");
    }

    // Free resources
    for (int i = 0; i < num_vars; ++i) {
        SCIPreleaseVar(scip, &vars[i]);
    }
    free(vars);
    SCIPfree(&scip);

    return success;
}
