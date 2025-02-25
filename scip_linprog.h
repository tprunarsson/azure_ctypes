#ifndef SCIP_LINPROG_H
#define SCIP_LINPROG_H

#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/type_retcode.h>

int scip_linprog_mip(
    int num_vars,
    int num_constraints_ub,
    int num_constraints_eq,
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
    int* integrality
);

#endif  // SCIP_LINPROG_H
