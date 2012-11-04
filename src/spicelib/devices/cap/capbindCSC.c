/**********
Author: 2012 Francesco Lannutti
**********/

#include "ngspice/ngspice.h"
#include "ngspice/cktdefs.h"
#include "capdefs.h"
#include "ngspice/sperror.h"

#include <stdlib.h>

static
int
BindCompare (const void *a, const void *b)
{
    BindElement *A, *B ;
    A = (BindElement *)a ;
    B = (BindElement *)b ;

    return ((int)(A->Sparse - B->Sparse)) ;
}

int
CAPbindCSC (GENmodel *inModel, CKTcircuit *ckt)
{
    CAPmodel *model = (CAPmodel *)inModel ;
    CAPinstance *here ;
    double *i ;
    BindElement *matched, *BindStruct ;
    size_t nz ;

    BindStruct = ckt->CKTmatrix->CKTbindStruct ;
    nz = (size_t)ckt->CKTmatrix->CKTklunz ;

    /* loop through all the capacitor models */
    for ( ; model != NULL ; model = model->CAPnextModel)
    {
        /* loop through all the instances of the model */
        for (here = model->CAPinstances ; here != NULL ; here = here->CAPnextInstance)
        {
            if ((here->CAPposNode != 0) && (here->CAPposNode != 0))
            {
                i = here->CAPposPosptr ;
                matched = (BindElement *) bsearch (&i, BindStruct, nz, sizeof(BindElement), BindCompare) ;
                here->CAPposPosStructPtr = matched ;
                here->CAPposPosptr = matched->CSC ;
            }

            if ((here->CAPnegNode != 0) && (here->CAPnegNode != 0))
            {
                i = here->CAPnegNegptr ;
                matched = (BindElement *) bsearch (&i, BindStruct, nz, sizeof(BindElement), BindCompare) ;
                here->CAPnegNegStructPtr = matched ;
                here->CAPnegNegptr = matched->CSC ;
            }

            if ((here->CAPposNode != 0) && (here->CAPnegNode != 0))
            {
                i = here->CAPposNegptr ;
                matched = (BindElement *) bsearch (&i, BindStruct, nz, sizeof(BindElement), BindCompare) ;
                here->CAPposNegStructPtr = matched ;
                here->CAPposNegptr = matched->CSC ;
            }

            if ((here->CAPnegNode != 0) && (here->CAPposNode != 0))
            {
                i = here->CAPnegPosptr ;
                matched = (BindElement *) bsearch (&i, BindStruct, nz, sizeof(BindElement), BindCompare) ;
                here->CAPnegPosStructPtr = matched ;
                here->CAPnegPosptr = matched->CSC ;
            }
        }
    }

    return (OK) ;
}

int
CAPbindCSCComplex (GENmodel *inModel, CKTcircuit *ckt)
{
    CAPmodel *model = (CAPmodel *)inModel ;
    CAPinstance *here ;

    NG_IGNORE (ckt) ;

    /* loop through all the capacitor models */
    for ( ; model != NULL ; model = model->CAPnextModel)
    {
        /* loop through all the instances of the model */
        for (here = model->CAPinstances ; here != NULL ; here = here->CAPnextInstance)
        {
            if ((here->CAPposNode != 0) && (here->CAPposNode != 0))
                here->CAPposPosptr = here->CAPposPosStructPtr->CSC_Complex ;

            if ((here->CAPnegNode != 0) && (here->CAPnegNode != 0))
                here->CAPnegNegptr = here->CAPnegNegStructPtr->CSC_Complex ;

            if ((here->CAPposNode != 0) && (here->CAPnegNode != 0))
                here->CAPposNegptr = here->CAPposNegStructPtr->CSC_Complex ;

            if ((here->CAPnegNode != 0) && (here->CAPposNode != 0))
                here->CAPnegPosptr = here->CAPnegPosStructPtr->CSC_Complex ;
        }
    }

    return (OK) ;
}

int
CAPbindCSCComplexToReal (GENmodel *inModel, CKTcircuit *ckt)
{
    CAPmodel *model = (CAPmodel *)inModel ;
    CAPinstance *here ;

    NG_IGNORE (ckt) ;

    /* loop through all the capacitor models */
    for ( ; model != NULL ; model = model->CAPnextModel)
    {
        /* loop through all the instances of the model */
        for (here = model->CAPinstances ; here != NULL ; here = here->CAPnextInstance)
        {
            if ((here->CAPposNode != 0) && (here->CAPposNode != 0))
                here->CAPposPosptr = here->CAPposPosStructPtr->CSC ;

            if ((here->CAPnegNode != 0) && (here->CAPnegNode != 0))
                here->CAPnegNegptr = here->CAPnegNegStructPtr->CSC ;

            if ((here->CAPposNode != 0) && (here->CAPnegNode != 0))
                here->CAPposNegptr = here->CAPposNegStructPtr->CSC ;

            if ((here->CAPnegNode != 0) && (here->CAPposNode != 0))
                here->CAPnegPosptr = here->CAPnegPosStructPtr->CSC ;
        }
    }

    return (OK) ;
}
