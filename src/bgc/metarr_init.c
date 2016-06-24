#include "pihm.h"

void metarr_init (metarr_struct *metarr, int start_time, int end_time)
{
    /*
     * Generate meteorological forcing array for spin-up
     */
    int             j, k;
    int             length;

    length = (end_time - start_time) / 24 / 3600;

    metarr->tmax = (double *)malloc (length * sizeof (double));
    metarr->tmin = (double *)malloc (length * sizeof (double));
    metarr->prcp = (double *)malloc (length * sizeof (double));
    metarr->vpd = (double *)malloc (length * sizeof (double));
    metarr->q2d = (double *)malloc (length * sizeof (double));
    metarr->swavgfd = (double *)malloc (length * sizeof (double));
    metarr->par = (double *)malloc (length * sizeof (double));
    metarr->dayl = (double *)malloc (length * sizeof (double));
    metarr->prev_dayl = (double *)malloc (length * sizeof (double));
    metarr->tavg = (double *)malloc (length * sizeof (double));
    metarr->tday = (double *)malloc (length * sizeof (double));
    metarr->tnight = (double *)malloc (length * sizeof (double));
    metarr->tsoil = (double *)malloc (length * sizeof (double));
    metarr->swc = (double *)malloc (length * sizeof (double));
    metarr->pa = (double *)malloc (length * sizeof (double));
    metarr->soilw = (double *)malloc (length * sizeof (double));
    metarr->sw_alb = (double *)malloc (length * sizeof (double));
    metarr->gl_bl = (double *)malloc (length * sizeof (double));
    metarr->flag = (int *)malloc (length * sizeof (int));

    for (k = 0; k < 4; k++)
    {
        metarr->latflux[k] = (double *)malloc (length * sizeof (double));
    }

    for (j = 0; j < length; j++)
    {
        metarr->flag[j] = 0;
    }
}

void Save2MetArr (elem_struct *elem, int numele, river_struct *riv, int numriv, int t, int start_time, int end_time)
{
    int             i, k;
    int             ind;
    metarr_struct  *metarr;
    daily_struct   *daily;

    if (t > start_time)
    {
        ind = (t - start_time) / 24 / 3600 - 1; /* t is already the end of day */

        for (i = 0; i < numele; i++)
        {
            metarr = &(elem[i].metarr);
            daily = &(elem[i].daily);

            metarr->dayl[ind] = daily->dayl;
            metarr->prev_dayl[ind] = daily->prev_dayl;

            metarr->tmax[ind] = daily->tmax - 273.15;
            metarr->tmin[ind] = daily->tmin - 273.15;
            metarr->tavg[ind] = daily->sfctmp - 273.15;
            metarr->tday[ind] = daily->tday - 273.15;
            metarr->tnight[ind] = daily->tnight - 273.15;

            metarr->q2d[ind] = daily->q2d;
            metarr->pa[ind] = daily->sfcprs;
            metarr->swavgfd[ind] = daily->solar;
            metarr->par[ind] = metarr->swavgfd[ind] * RAD2PAR;

            metarr->tsoil[ind] = daily->stc[0] - 273.15;
            metarr->swc[ind] = daily->sh2o[0];
            metarr->soilw[ind] = (daily->surf
                + daily->unsat * elem[i].soil.porosity
                + daily->gw * elem[i].soil.porosity)
                * 1000.0;

            metarr->sw_alb[ind] = daily->albedo;
            metarr->gl_bl[ind] = daily->ch;

            for (k = 0; k < 3; k++)
            {
                /* Convert from m3/s to kg/m2/d */
                metarr->latflux[k][ind] = 1000.0
                    * (daily->fluxsub[k] + daily->fluxsurf[k])
                    * 24.0 * 3600.0 / elem[i].topo.area;
            }
            metarr->latflux[3][ind] = 0.0;

            metarr->flag[ind] = 1;
        }

        for (i = 0; i < numriv; i++)
        {
            metarr = &riv[i].metarr;
            daily = &riv[i].daily;

            metarr->soilw[ind] = (daily->surf
                + daily->gw * riv[i].matl.porosity) * 1000.0;

            /* Convert from m3/s to kg/m2/d */
            for (k = 0; k < 4; k++)
            {
                metarr->latflux[k][ind] = 1000.0 * (daily->fluxsurf[k] + daily->fluxsub[k]) * 24.0 * 3600.0 / riv[i].topo.area;
            }

            metarr->flag[ind] = 1;
        }

    }
}
