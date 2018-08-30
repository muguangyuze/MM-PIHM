#include "pihm.h"

void ReadAlloc(pihm_struct pihm)
{
    PIHMprintf(VL_VERBOSE, "\nRead input files:\n");

    /* Set file names of the input files */
    sprintf(pihm->filename.riv,      "input/%s/%s.riv",      project, project);
    sprintf(pihm->filename.mesh,     "input/%s/%s.mesh",     project, project);
    sprintf(pihm->filename.att,      "input/%s/%s.att",      project, project);
    sprintf(pihm->filename.soil,     "input/%s/%s.soil",     project, project);
    sprintf(pihm->filename.lc,       "input/vegprmt.tbl");
    sprintf(pihm->filename.meteo,    "input/%s/%s.meteo",    project, project);
    sprintf(pihm->filename.lai,      "input/%s/%s.lai",      project, project);
    sprintf(pihm->filename.bc,       "input/%s/%s.bc",       project, project);
    sprintf(pihm->filename.para,     "input/%s/%s.para",     project, project);
    sprintf(pihm->filename.calib,    "input/%s/%s.calib",    project, project);
    sprintf(pihm->filename.ic,       "input/%s/%s.ic",       project, project);
    sprintf(pihm->filename.tecplot,  "input/%s/%s.tecplot",  project, project);
#if defined(_FBR_)
    sprintf(pihm->filename.geol,     "input/%s/%s.geol",     project, project);
    sprintf(pihm->filename.bedrock,  "input/%s/%s.bedrock",  project, project);
#endif
#if defined(_NOAH_)
    sprintf(pihm->filename.lsm,      "input/%s/%s.lsm",      project, project);
    sprintf(pihm->filename.rad,      "input/%s/%s.rad",      project, project);
#endif
#if defined(_CYCLES_)
    sprintf(pihm->filename.cycles,   "input/%s/%s.cycles",   project, project);
    sprintf(pihm->filename.soilinit, "input/%s/%s.soilinit", project, project);
    sprintf(pihm->filename.crop,     "input/%s/%s.crop",     project, project);
    sprintf(pihm->filename.cyclesic, "input/%s/%s.cyclesic", project, project);
#endif
#if defined(_BGC_)
    sprintf(pihm->filename.bgc,      "input/%s/%s.bgc",      project, project);
    sprintf(pihm->filename.bgcic,    "input/%s/%s.bgcic",    project, project);
#endif

    /* Read river input file */
    ReadRiver(pihm->filename.riv, &pihm->rivtbl, &pihm->shptbl, &pihm->matltbl,
        &pihm->forc);

    /* Read mesh structure input file */
    ReadMesh(pihm->filename.mesh, &pihm->meshtbl);

    /* Read attribute table input file */
    ReadAtt(pihm->filename.att, &pihm->atttbl);

    /* Read soil input file */
    ReadSoil(pihm->filename.soil, &pihm->soiltbl);

    /* Read land cover input file */
    ReadLc(pihm->filename.lc, &pihm->lctbl);

    /* Read meteorological forcing input file */
    ReadForc(pihm->filename.meteo, &pihm->forc);

    /* Read LAI input file */
    ReadLai(pihm->filename.lai, &pihm->forc, &pihm->atttbl);

    /* Read source and sink input file */
    pihm->forc.nsource = 0;
#if NOT_YET_IMPLEMENTED
    ReadSS ();
#endif

    /* Read model control file */
    ReadPara(pihm->filename.para, &pihm->ctrl);

    /* Read calibration input file */
    ReadCalib(pihm->filename.calib, &pihm->cal);

    if (tecplot)
    {
        ReadTecplot(pihm->filename.tecplot, &pihm->ctrl);
    }

#if defined(_FBR_)
    /* Read geology input file */
    ReadGeol (pihm->filename.geol, &pihm->geoltbl);

    /* Read bedrock control file */
    ReadBedrock(pihm->filename.bedrock, &pihm->atttbl, &pihm->meshtbl,
        &pihm->ctrl);
#endif

    /* Read boundary condition input file
     * Boundary conditions might be needed by fbr thus should be read in after
     * reading bedrock input */
    ReadBc(pihm->filename.bc, &pihm->forc, &pihm->atttbl);

#if defined(_NOAH_)
    /* Read LSM input file */
    ReadLsm(pihm->filename.lsm, &pihm->siteinfo, &pihm->ctrl, &pihm->noahtbl);

    if (pihm->ctrl.rad_mode == TOPO_SOL)
    {
        /* Read radiation input file */
        ReadRad(pihm->filename.rad, &pihm->forc);
    }
#endif

#if defined(_CYCLES_)
    /* Read Cycles simulation control file */
    ReadCyclesCtrl(pihm->filename.cycles, &pihm->agtbl, &pihm->ctrl);

    /* Read soil initialization file */
    ReadSoilInit(pihm->filename.soilinit, &pihm->soiltbl);

    /* Read crop description file */
    ReadCrop(pihm->filename.crop, &pihm->croptbl);

    /* Read operation file */
    ReadOperation(&pihm->agtbl, &pihm->mgmttbl, &pihm->croptbl);
#endif

#if defined(_BGC_)
    ReadBgc(pihm->filename.bgc, &pihm->ctrl, &pihm->co2, &pihm->ndepctrl,
        &pihm->cninit, pihm->filename.co2, pihm->filename.ndep);

    /* Read Biome-BGC epc files */
    ReadEpc(&pihm->epctbl);

    /* Read CO2 and Ndep files */
    pihm->forc.co2 = (tsdata_struct *)malloc(sizeof(tsdata_struct));
    pihm->forc.ndep = (tsdata_struct *)malloc(sizeof(tsdata_struct));

    if (pihm->co2.varco2)
    {
        ReadAnnFile(&pihm->forc.co2[0], pihm->filename.co2);
    }

    if (pihm->ndepctrl.varndep)
    {
        ReadAnnFile(&pihm->forc.ndep[0], pihm->filename.ndep);
    }
#endif
}

void ReadRiver(const char *filename, rivtbl_struct *rivtbl,
    shptbl_struct *shptbl, matltbl_struct *matltbl, forc_struct *forc)
{
    int             i, j;
    FILE           *riv_file;
    char            cmdstr[MAXSTRING];
    int             match;
    int             index;
    int             lno = 0;

    riv_file = fopen(filename, "r");
    CheckFile(riv_file, filename);
    PIHMprintf(VL_VERBOSE, " Reading %s\n", filename);

    /*
     * Read river segment block
     */
    /* Read number of river segments */
    FindLine(riv_file, "BOF", &lno, filename);
    NextLine(riv_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "NUMRIV", &nriver, 'i', filename, lno);

    /* Allocate */
    rivtbl->fromnode = (int *)malloc(nriver * sizeof(int));
    rivtbl->tonode = (int *)malloc(nriver * sizeof(int));
    rivtbl->down = (int *)malloc(nriver * sizeof(int));
    rivtbl->up1 = (int *)malloc(nriver * sizeof(int));        // 01.12 by Wei Zhi
    rivtbl->up2 = (int *)malloc(nriver * sizeof(int));        // 01.12 by Wei Zhi
    rivtbl->leftele = (int *)malloc(nriver * sizeof(int));
    rivtbl->rightele = (int *)malloc(nriver * sizeof(int));
    rivtbl->shp = (int *)malloc(nriver * sizeof(int));
    rivtbl->matl = (int *)malloc(nriver * sizeof(int));
    rivtbl->bc = (int *)malloc(nriver * sizeof(int));
    rivtbl->rsvr = (int *)malloc(nriver * sizeof(int));

    /* Skip header line */
    NextLine(riv_file, cmdstr, &lno);

    /* Read river segment information */
    for (i = 0; i < nriver; i++)
    {
        NextLine(riv_file, cmdstr, &lno);
        // 01.12 by Wei Zhi
        /*
        match = sscanf(cmdstr, "%d %d %d %d %d %d %d %d %d %d",
            &index,
            &rivtbl->fromnode[i], &rivtbl->tonode[i], &rivtbl->down[i],
            &rivtbl->leftele[i], &rivtbl->rightele[i], &rivtbl->shp[i],
            &rivtbl->matl[i], &rivtbl->bc[i], &rivtbl->rsvr[i]);  */
        match = sscanf(cmdstr, "%d %d %d %d %d %d %d %d %d %d %d %d",
            &index,
            &rivtbl->fromnode[i], &rivtbl->tonode[i], &rivtbl->down[i], &rivtbl->up1[i], &rivtbl->up2[i],  // 01.12 by Wei Zhi
            &rivtbl->leftele[i], &rivtbl->rightele[i], &rivtbl->shp[i],
            &rivtbl->matl[i], &rivtbl->bc[i], &rivtbl->rsvr[i]);
        
        // 01.12 by Wei Zhi          
        //if (match != 10 || i != index - 1)
        if (match != 12 || i != index - 1)
        {
            PIHMprintf(VL_ERROR,
                "Error reading river attribute for the %dth segment.\n", i + 1);
            PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n", filename, lno);
            PIHMexit(EXIT_FAILURE);
        }
    }

    /*
     * Read river shape information
     */
    NextLine(riv_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "SHAPE", &shptbl->number, 'i', filename, lno);

    /* Allocate */
    shptbl->depth = (double *)malloc(shptbl->number * sizeof(double));
    shptbl->intrpl_ord = (int *)malloc(shptbl->number * sizeof(int));
    shptbl->coeff = (double *)malloc(shptbl->number * sizeof(double));

    /* Skip header line */
    NextLine(riv_file, cmdstr, &lno);

    for (i = 0; i < shptbl->number; i++)
    {
        NextLine(riv_file, cmdstr, &lno);
        match = sscanf(cmdstr, "%d %lf %d %lf",
            &index,
            &shptbl->depth[i], &shptbl->intrpl_ord[i], &shptbl->coeff[i]);
        if (match != 4 || i != index - 1)
        {
            PIHMprintf(VL_ERROR,
                "Error reading river shape description for the %dth shape.\n",
                i + 1);
            PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n", filename, lno);
            PIHMexit(EXIT_FAILURE);
        }
    }

    /*
     * Read river material information
     */
    NextLine(riv_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "MATERIAL", &matltbl->number, 'i', filename, lno);

    /* Allocate */
    matltbl->rough = (double *)malloc(matltbl->number * sizeof(double));
    matltbl->cwr = (double *)malloc(matltbl->number * sizeof(double));
    matltbl->ksath = (double *)malloc(matltbl->number * sizeof(double));
    matltbl->ksatv = (double *)malloc(matltbl->number * sizeof(double));
    matltbl->bedthick = (double *)malloc(matltbl->number * sizeof(double));

    /* Skip header line */
    NextLine(riv_file, cmdstr, &lno);

    for (i = 0; i < matltbl->number; i++)
    {
        NextLine(riv_file, cmdstr, &lno);
        match = sscanf(cmdstr, "%d %lf %lf %lf %lf %lf",
            &index, &matltbl->rough[i], &matltbl->cwr[i],
            &matltbl->ksath[i], &matltbl->ksatv[i], &matltbl->bedthick[i]);
        if (match != 6 || i != index - 1)
        {
            PIHMprintf(VL_ERROR,
                "Error reading description of the %dth material.\n", i + 1);
            PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n", filename, lno);
            PIHMexit(EXIT_FAILURE);
        }
    }

    /*
     * Read river boundary condition block
     */
    NextLine(riv_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "BC", &forc->nriverbc, 'i', filename, lno);

    if (forc->nriverbc > 0)
    {
        forc->riverbc =
            (tsdata_struct *)malloc(forc->nriverbc * sizeof(tsdata_struct));

        for (i = 0; i < forc->nriverbc; i++)
        {
            NextLine(riv_file, cmdstr, &lno);
            match = sscanf(cmdstr, "%*s %d", &index);
            if (match != 1 || i != index - 1)
            {
                PIHMprintf(VL_ERROR,
                    "Error reading description "
                    "of the %dth river boundary condition.\n", i);
                PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n",
                    filename, lno);
                PIHMexit(EXIT_FAILURE);
            }
            NextLine(riv_file, cmdstr, &lno);
            NextLine(riv_file, cmdstr, &lno);
            forc->riverbc[i].length =
                CountLine(riv_file, cmdstr, 2, "RIV_TS", "RES");
        }

        FindLine(riv_file, "BOF", &lno, filename);
        FindLine(riv_file, "BC", &lno, filename);
        for (i = 0; i < forc->nriverbc; i++)
        {
            NextLine(riv_file, cmdstr, &lno);
            NextLine(riv_file, cmdstr, &lno);
            NextLine(riv_file, cmdstr, &lno);

            forc->riverbc[i].data =
                (double **)malloc((forc->riverbc[i].length) * sizeof(double *));
            forc->riverbc[i].ftime =
                (int *)malloc((forc->riverbc[i].length) * sizeof(int));
            for (j = 0; j < forc->riverbc[i].length; j++)
            {
                forc->riverbc[i].data[j] = (double *)malloc(sizeof(double));
                NextLine(riv_file, cmdstr, &lno);
                if (!ReadTS(cmdstr, &forc->riverbc[i].ftime[j],
                        &forc->riverbc[i].data[j][0], 1))
                {
                    PIHMprintf(VL_ERROR,
                        "Error reading river boundary condition.\n");
                    PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n",
                        filename, lno);
                    PIHMexit(EXIT_FAILURE);
                }
            }
        }
    }

#if NOT_YET_IMPLEMENTED
    /* Read Reservoir information */
#endif

    fclose(riv_file);
}

void ReadMesh(const char *filename, meshtbl_struct *meshtbl)
{
    FILE           *mesh_file;
    int             i;
    char            cmdstr[MAXSTRING];
    int             match;
    int             index;
    int             lno = 0;

    mesh_file = fopen(filename, "r");
    CheckFile(mesh_file, filename);
    PIHMprintf(VL_VERBOSE, " Reading %s\n", filename);

    /*
     * Read element mesh block
     */
    NextLine(mesh_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "NUMELE", &nelem, 'i', filename, lno);

    meshtbl->node = (int **)malloc(nelem * sizeof(int *));
    meshtbl->nabr = (int **)malloc(nelem * sizeof(int *));

    /* Skip header line */
    NextLine(mesh_file, cmdstr, &lno);

    for (i = 0; i < nelem; i++)
    {
        meshtbl->node[i] = (int *)malloc(NUM_EDGE * sizeof(int));
        meshtbl->nabr[i] = (int *)malloc(NUM_EDGE * sizeof(int));

        NextLine(mesh_file, cmdstr, &lno);
        match = sscanf(cmdstr, "%d %d %d %d %d %d %d",
            &index,
            &meshtbl->node[i][0], &meshtbl->node[i][1], &meshtbl->node[i][2],
            &meshtbl->nabr[i][0], &meshtbl->nabr[i][1], &meshtbl->nabr[i][2]);
        if (match != 7 || i != index - 1)
        {
            PIHMprintf(VL_ERROR,
                "Error reading mesh description of the %dth element.\n", i + 1);
            PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n", filename, lno);
            PIHMexit(EXIT_FAILURE);
        }
    }

    /*
     * Read node block
     */
    NextLine(mesh_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "NUMNODE", &meshtbl->numnode, 'i', filename, lno);

    /* Skip header line */
    NextLine(mesh_file, cmdstr, &lno);

    meshtbl->x = (double *)malloc(meshtbl->numnode * sizeof(double));
    meshtbl->y = (double *)malloc(meshtbl->numnode * sizeof(double));
    meshtbl->zmin = (double *)malloc(meshtbl->numnode * sizeof(double));
    meshtbl->zmax = (double *)malloc(meshtbl->numnode * sizeof(double));

    for (i = 0; i < meshtbl->numnode; i++)
    {
        NextLine(mesh_file, cmdstr, &lno);
        match = sscanf(cmdstr, "%d %lf %lf %lf %lf",
            &index,
            &meshtbl->x[i], &meshtbl->y[i],
            &meshtbl->zmin[i], &meshtbl->zmax[i]);
        if (match != 5 || i != index - 1)
        {
            PIHMprintf(VL_ERROR,
                "Error reading description of the %dth node!\n", i + 1);
            PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n", filename, lno);
            PIHMexit(EXIT_FAILURE);
        }
    }

    fclose(mesh_file);
}

void ReadAtt(const char *filename, atttbl_struct *atttbl)
{
    int             i;
    FILE           *att_file;   /* Pointer to .att file */
    char            cmdstr[MAXSTRING];
    int             match;
    int             index;
    int             lno = 0;

    att_file = fopen(filename, "r");
    CheckFile(att_file, filename);
    PIHMprintf(VL_VERBOSE, " Reading %s\n", filename);

    atttbl->soil = (int *)malloc(nelem * sizeof(int));
    atttbl->geol = (int *)malloc(nelem * sizeof(int));
    atttbl->lc = (int *)malloc(nelem * sizeof(int));
    atttbl->bc = (int **)malloc(nelem * sizeof(int *));
    for (i = 0; i < nelem; i++)
    {
        atttbl->bc[i] = (int *)malloc(NUM_EDGE * sizeof(int));
    }
    atttbl->meteo = (int *)malloc(nelem * sizeof(int));
    atttbl->lai = (int *)malloc(nelem * sizeof(int));
    atttbl->source = (int *)malloc(nelem * sizeof(int));

    NextLine(att_file, cmdstr, &lno);
    for (i = 0; i < nelem; i++)
    {
        NextLine(att_file, cmdstr, &lno);
        match = sscanf(cmdstr, "%d %d %d %d %d %d %d %d %d %d",
            &index,
            &atttbl->soil[i], &atttbl->geol[i], &atttbl->lc[i],
            &atttbl->meteo[i], &atttbl->lai[i], &atttbl->source[i],
            &atttbl->bc[i][0], &atttbl->bc[i][1], &atttbl->bc[i][2]);
        if (match != 10)
        {
            PIHMprintf(VL_ERROR,
                "Error reading attribute of the %dth element.\n", i + 1);
            PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n", filename, lno);
            PIHMexit(EXIT_FAILURE);
        }
    }

    fclose(att_file);
}

void ReadSoil(const char *filename, soiltbl_struct *soiltbl)
{
    FILE           *soil_file;
    int             i;
    char            cmdstr[MAXSTRING];
    int             match;
    int             index;
    int             texture;
    const int       TOPSOIL = 1;
    const int       SUBSOIL = 0;
    int             ptf_used = 0;
    int             lno = 0;

    soil_file = fopen(filename, "r");
    CheckFile(soil_file, filename);
    PIHMprintf(VL_VERBOSE, " Reading %s\n", filename);

    /* Start reading soil file */
    NextLine(soil_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "NUMSOIL", &soiltbl->number, 'i', filename, lno);

    soiltbl->silt = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->clay = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->om = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->bd = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->kinfv = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->ksatv = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->ksath = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->smcmax = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->smcmin = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->qtz = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->alpha = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->beta = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->areafh = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->areafv = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->dmac = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->smcref = (double *)malloc(soiltbl->number * sizeof(double));
    soiltbl->smcwlt = (double *)malloc(soiltbl->number * sizeof(double));

    /* Skip header line */
    NextLine(soil_file, cmdstr, &lno);

    for (i = 0; i < soiltbl->number; i++)
    {
        NextLine(soil_file, cmdstr, &lno);
        match = sscanf(cmdstr,
            "%d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
            &index, &soiltbl->silt[i], &soiltbl->clay[i], &soiltbl->om[i],
            &soiltbl->bd[i],
            &soiltbl->kinfv[i], &soiltbl->ksatv[i], &soiltbl->ksath[i],
            &soiltbl->smcmax[i], &soiltbl->smcmin[i],
            &soiltbl->alpha[i], &soiltbl->beta[i],
            &soiltbl->areafh[i], &soiltbl->areafv[i],
            &soiltbl->dmac[i], &soiltbl->qtz[i]);

        if (match != 16 || i != index - 1)
        {
            PIHMprintf(VL_ERROR,
                "Error reading properties of the %dth soil type.\n", i + 1);
            PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n", filename, lno);
            PIHMexit(EXIT_FAILURE);
        }

        /* Fill in missing organic matter and bulk density values */
        soiltbl->om[i] = (soiltbl->om[i] > 0.0) ? soiltbl->om[i] : 2.5;
        soiltbl->bd[i] = (soiltbl->bd[i] > 0.0) ? soiltbl->bd[i] : 1.3;

        /* Fill missing hydraulic properties using PTFs */
        if (soiltbl->kinfv[i] < 0.0)
        {
            soiltbl->kinfv[i] = PtfKv(soiltbl->silt[i], soiltbl->clay[i],
                soiltbl->om[i], soiltbl->bd[i], TOPSOIL);
            ptf_used = 1;
        }
        if (soiltbl->ksatv[i] < 0.0)
        {
            soiltbl->ksatv[i] = PtfKv(soiltbl->silt[i], soiltbl->clay[i],
                soiltbl->om[i], soiltbl->bd[i], SUBSOIL);
            ptf_used = 1;
        }
        if (soiltbl->ksath[i] < 0.0)
        {
            soiltbl->ksath[i] = 10.0 * soiltbl->ksatv[i];
            ptf_used = 1;
        }
        if (soiltbl->smcmax[i] < 0.0)
        {
            soiltbl->smcmax[i] = PtfThetas(soiltbl->silt[i], soiltbl->clay[i],
                soiltbl->om[i], soiltbl->bd[i], SUBSOIL);
            ptf_used = 1;
        }
        if (soiltbl->smcmin[i] < 0.0)
        {
            soiltbl->smcmin[i] = PtfThetar(soiltbl->silt[i], soiltbl->clay[i],
                soiltbl->om[i], soiltbl->bd[i], SUBSOIL);
            ptf_used = 1;
        }
        if (soiltbl->alpha[i] < 0.0)
        {
            soiltbl->alpha[i] = PtfAlpha(soiltbl->silt[i], soiltbl->clay[i],
                soiltbl->om[i], soiltbl->bd[i], SUBSOIL);
            ptf_used = 1;
        }
        if (soiltbl->beta[i] < 0.0)
        {
            soiltbl->beta[i] = PtfBeta(soiltbl->silt[i], soiltbl->clay[i],
                soiltbl->om[i], soiltbl->bd[i], SUBSOIL);
            ptf_used = 1;
        }
        if (soiltbl->qtz[i] < 0.0)
        {
            texture = SoilTex(soiltbl->silt[i], soiltbl->clay[i]);
            soiltbl->qtz[i] = Qtz(texture);
            ptf_used = 1;
        }

        /* Calculate field capacity and wilting point */
        soiltbl->smcref[i] = FieldCapacity(soiltbl->alpha[i], soiltbl->beta[i],
            soiltbl->ksatv[i], soiltbl->smcmax[i], soiltbl->smcmin[i]);
        soiltbl->smcwlt[i] = WiltingPoint(soiltbl->smcmax[i],
            soiltbl->smcmin[i], soiltbl->alpha[i], soiltbl->beta[i]);
    }

    NextLine(soil_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "DINF", &soiltbl->dinf, 'd', filename, lno);

    NextLine(soil_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "KMACV_RO", &soiltbl->kmacv_ro, 'd', filename, lno);

    NextLine(soil_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "KMACH_RO", &soiltbl->kmach_ro, 'd', filename, lno);

    if (ptf_used)
    {
        PIHMprintf(VL_NORMAL,
            "%-7s\t%-15s\t%-15s\t%-15s\t%-7s\t%-7s\t%-7s\t%-7s\t%-7s\n",
            "TYPE", "KINFV", "KSATV", "KSATH", "SMCMAX", "SMCMIN", "ALPHA",
            "BETA", "QTZ");
        for (i = 0; i < soiltbl->number; i++)
        {
            PIHMprintf(VL_NORMAL,
                "%-7d\t%-15.3le\t%-15.3le\t%-15.3le\t%-7.3lf\t%-7.3lf\t"
                "%-7.3lf\t%-7.3lf\t%-7.3lf\n",
                i + 1, soiltbl->kinfv[i], soiltbl->ksatv[i],
                soiltbl->ksath[i], soiltbl->smcmax[i], soiltbl->smcmin[i],
                soiltbl->alpha[i], soiltbl->beta[i], soiltbl->qtz[i]);
        }
    }

    fclose(soil_file);
}

#if defined(_FBR_)
void ReadGeol(const char *filename, geoltbl_struct *geoltbl)
{
    FILE           *geol_file;
    int             i;
    char            cmdstr[MAXSTRING];
    int             match;
    int             index;
    int             lno = 0;

    geol_file = fopen (filename, "r");
    CheckFile (geol_file, filename);
    PIHMprintf (VL_VERBOSE, " Reading %s\n", filename);

    /* Start reading soil file */
    NextLine (geol_file, cmdstr, &lno);
    ReadKeyword (cmdstr, "NUMGEOL", &geoltbl->number, 'i', filename, lno);

    geoltbl->ksatv = (double *)malloc (geoltbl->number * sizeof (double));
    geoltbl->ksath = (double *)malloc (geoltbl->number * sizeof (double));
    geoltbl->smcmax = (double *)malloc (geoltbl->number * sizeof (double));
    geoltbl->smcmin = (double *)malloc (geoltbl->number * sizeof (double));
    geoltbl->alpha = (double *)malloc (geoltbl->number * sizeof (double));
    geoltbl->beta = (double *)malloc (geoltbl->number * sizeof (double));

    /* Skip header line */
    NextLine (geol_file, cmdstr, &lno);

    for (i = 0; i < geoltbl->number; i++)
    {
        NextLine (geol_file, cmdstr, &lno);
        match = sscanf (cmdstr, "%d %lf %lf %lf %lf %lf %lf",
            &index, &geoltbl->ksatv[i], &geoltbl->ksath[i],
            &geoltbl->smcmax[i], &geoltbl->smcmin[i],
            &geoltbl->alpha[i], &geoltbl->beta[i]);

        if (match != 7 || i != index - 1)
        {
            PIHMprintf (VL_ERROR,
                "Error reading properties of the %dth geology type.\n", i + 1);
            PIHMprintf (VL_ERROR, "Error in %s near Line %d.\n", filename, lno);
            PIHMexit (EXIT_FAILURE);
        }
    }

    fclose (geol_file);
}

void ReadBedrock(const char *filename, atttbl_struct *atttbl,
    meshtbl_struct *meshtbl, ctrl_struct *ctrl)
{
    FILE           *br_file;
    int             i;
    char            cmdstr[MAXSTRING];
    int             match;
    int             index;
    int             lno = 0;

    br_file = fopen (filename, "r");
    CheckFile (br_file, filename);
    PIHMprintf (VL_VERBOSE, " Reading %s\n", filename);

    /* Start reading bedrock file */
    /* Read fbr boundary conditions */
    atttbl->fbr_bc = (int **)malloc(nelem * sizeof(int *));
    for (i = 0; i < nelem; i++)
    {
        atttbl->fbr_bc[i] = (int *)malloc(NUM_EDGE * sizeof(int));
    }

    /* Skip header line */
    NextLine (br_file, cmdstr, &lno);
    for (i = 0; i < nelem; i++)
    {
        NextLine (br_file, cmdstr, &lno);
        match = sscanf(cmdstr, "%d %d %d %d",
            &index,
            &atttbl->fbr_bc[i][0], &atttbl->fbr_bc[i][1],
            &atttbl->fbr_bc[i][2]);
        if (match != 4 || i != index - 1)
        {
            PIHMprintf(VL_ERROR,
                "Error reading boundary condition type for fractured bedrock"
                "layer of the %dth element.\n", i + 1);
            PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n", filename, lno);
            PIHMexit(EXIT_FAILURE);
        }
    }

    /* Read bedrock elevations */
    meshtbl->zbed = (double *)malloc (meshtbl->numnode * sizeof (double));

    /* Skip header line */
    NextLine (br_file, cmdstr, &lno);
    for (i = 0; i < meshtbl->numnode; i++)
    {
        NextLine (br_file, cmdstr, &lno);
        match = sscanf (cmdstr, "%d %lf", &index, &meshtbl->zbed[i]);
        if (match != 2 || i != index - 1)
        {
            PIHMprintf (VL_ERROR,
                "Error reading bedrock description of the %dth node.\n", i + 1);
            PIHMprintf (VL_ERROR, "Error in %s near Line %d.\n", filename, lno);
            PIHMexit (EXIT_FAILURE);
        }
    }

    NextLine (br_file, cmdstr, &lno);
    ctrl->prtvrbl[FBRUNSAT_CTRL] =
        ReadPrtCtrl (cmdstr, "FBRUNSAT", filename, lno);

    NextLine (br_file, cmdstr, &lno);
    ctrl->prtvrbl[FBRGW_CTRL] = ReadPrtCtrl (cmdstr, "FBRGW", filename, lno);

    NextLine (br_file, cmdstr, &lno);
    ctrl->prtvrbl[FBRINFIL_CTRL] =
        ReadPrtCtrl(cmdstr, "FBRINFIL", filename, lno);

    NextLine (br_file, cmdstr, &lno);
    ctrl->prtvrbl[FBRRECHG_CTRL] =
        ReadPrtCtrl (cmdstr, "FBRRECHG", filename, lno);

    NextLine (br_file, cmdstr, &lno);
    ctrl->prtvrbl[FBRFLOW_CTRL] =
        ReadPrtCtrl (cmdstr, "FBRFLOW", filename, lno);

    fclose (br_file);
}
#endif

void ReadLc(const char *filename, lctbl_struct *lctbl)
{
    FILE           *lc_file;    /* Pointer to .lc file */
    int             i;
    char            cmdstr[MAXSTRING];
    int             match;
    int             index;
    int             lno = 0;

    lc_file = fopen(filename, "r");
    CheckFile(lc_file, filename);
    PIHMprintf(VL_VERBOSE, " Reading %s\n", filename);

    /* Start reading land cover file */
    NextLine(lc_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "NUMLC", &lctbl->number, 'i', filename, lno);

    lctbl->laimax = (double *)malloc(lctbl->number * sizeof(double));
    lctbl->laimin = (double *)malloc(lctbl->number * sizeof(double));
    lctbl->vegfrac = (double *)malloc(lctbl->number * sizeof(double));
    lctbl->albedomin = (double *)malloc(lctbl->number * sizeof(double));
    lctbl->albedomax = (double *)malloc(lctbl->number * sizeof(double));
    lctbl->emissmin = (double *)malloc(lctbl->number * sizeof(double));
    lctbl->emissmax = (double *)malloc(lctbl->number * sizeof(double));
    lctbl->z0min = (double *)malloc(lctbl->number * sizeof(double));
    lctbl->z0max = (double *)malloc(lctbl->number * sizeof(double));
    lctbl->hs = (double *)malloc(lctbl->number * sizeof(double));
    lctbl->snup = (double *)malloc(lctbl->number * sizeof(double));
    lctbl->rgl = (double *)malloc(lctbl->number * sizeof(double));
    lctbl->rsmin = (double *)malloc(lctbl->number * sizeof(double));
    lctbl->rough = (double *)malloc(lctbl->number * sizeof(double));
    lctbl->rzd = (double *)malloc(lctbl->number * sizeof(double));

    /* Skip header line */
    NextLine(lc_file, cmdstr, &lno);

    for (i = 0; i < lctbl->number; i++)
    {
        NextLine(lc_file, cmdstr, &lno);
        match =
            sscanf(cmdstr,
            "%d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
            &index,
            &lctbl->vegfrac[i], &lctbl->rzd[i], &lctbl->rsmin[i],
            &lctbl->rgl[i], &lctbl->hs[i], &lctbl->snup[i],
            &lctbl->laimin[i], &lctbl->laimax[i],
            &lctbl->emissmin[i], &lctbl->emissmax[i],
            &lctbl->albedomin[i], &lctbl->albedomax[i],
            &lctbl->z0min[i], &lctbl->z0max[i], &lctbl->rough[i]);
        if (match != 16 || i != index - 1)
        {
            PIHMprintf(VL_ERROR,
                "Error reading properties of the %dth landcover type.\n",
                i + 1);
            PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n", filename, lno);
            PIHMexit(EXIT_FAILURE);
        }
    }

    NextLine(lc_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "TOPT_DATA", &lctbl->topt, 'd', filename, lno);

    NextLine(lc_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "CFACTR_DATA", &lctbl->cfactr, 'd', filename, lno);

    NextLine(lc_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "RSMAX_DATA", &lctbl->rsmax, 'd', filename, lno);

    NextLine(lc_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "BARE", &lctbl->bare, 'i', filename, lno);

    NextLine(lc_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "NATURAL", &lctbl->natural, 'i', filename, lno);

    fclose(lc_file);
}

void ReadForc(const char *filename, forc_struct *forc)
{
    FILE           *meteo_file;
    char            cmdstr[MAXSTRING];
    int             i, j;
    int             match;
    int             index;
    int             lno = 0;

    meteo_file = fopen(filename, "r");
    CheckFile(meteo_file, filename);
    PIHMprintf(VL_VERBOSE, " Reading %s\n", filename);

    FindLine(meteo_file, "BOF", &lno, filename);

    forc->nmeteo = CountOccurr(meteo_file, "METEO_TS");

    FindLine(meteo_file, "BOF", &lno, filename);
    if (forc->nmeteo > 0)
    {
        forc->meteo =
            (tsdata_struct *)malloc(forc->nmeteo * sizeof(tsdata_struct));

        NextLine(meteo_file, cmdstr, &lno);
        for (i = 0; i < forc->nmeteo; i++)
        {
            match = sscanf(cmdstr, "%*s %d %*s %lf",
                &index, &forc->meteo[i].zlvl_wind);
            if (match != 2 || i != index - 1)
            {
                PIHMprintf(VL_ERROR,
                    "Error reading the %dth meteorological forcing"
                    " time series.\n", i + 1);
                PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n",
                    filename, lno);
                PIHMexit(EXIT_FAILURE);
            }
            /* Skip header lines */
            NextLine(meteo_file, cmdstr, &lno);
            NextLine(meteo_file, cmdstr, &lno);
            forc->meteo[i].length =
                CountLine(meteo_file, cmdstr, 1, "METEO_TS");
        }

        /* Rewind and read */
        FindLine(meteo_file, "BOF", &lno, filename);
        for (i = 0; i < forc->nmeteo; i++)
        {
            /* Skip header lines */
            NextLine(meteo_file, cmdstr, &lno);
            NextLine(meteo_file, cmdstr, &lno);
            NextLine(meteo_file, cmdstr, &lno);

            forc->meteo[i].ftime =
                (int *)malloc(forc->meteo[i].length * sizeof(int));
            forc->meteo[i].data =
                (double **)malloc(forc->meteo[i].length * sizeof(double *));
            for (j = 0; j < forc->meteo[i].length; j++)
            {
                forc->meteo[i].data[j] =
                    (double *)malloc(NUM_METEO_VAR * sizeof(double));
                NextLine(meteo_file, cmdstr, &lno);
                if (!ReadTS(cmdstr, &forc->meteo[i].ftime[j],
                    &forc->meteo[i].data[j][0], NUM_METEO_VAR))
                {
                    PIHMprintf(VL_ERROR,
                        "Error reading meteorological forcing.");
                    PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n",
                        filename, lno);
                    PIHMexit(EXIT_FAILURE);
                }
            }
        }
    }

    fclose(meteo_file);
}

void ReadLai(const char *filename, forc_struct *forc,
    const atttbl_struct *atttbl)
{
    char            cmdstr[MAXSTRING];
    int             read_lai = 0;
    FILE           *lai_file;
    int             i, j;
    int             index;
    int             lno = 0;

    for (i = 0; i < nelem; i++)
    {
        if (atttbl->lai[i] > 0)
        {
            read_lai = 1;
            break;
        }
    }

    forc->nlai = 0;

    if (read_lai)
    {
        lai_file = fopen(filename, "r");
        CheckFile(lai_file, filename);
        PIHMprintf(VL_VERBOSE, " Reading %s\n", filename);

        /* Start reading lai_file */
        FindLine(lai_file, "BOF", &lno, filename);

        forc->nlai = CountOccurr(lai_file, "LAI_TS");

        FindLine(lai_file, "BOF", &lno, filename);
        if (forc->nlai > 0)
        {
            forc->lai =
                (tsdata_struct *)malloc(forc->nlai * sizeof(tsdata_struct));

            NextLine(lai_file, cmdstr, &lno);
            for (i = 0; i < forc->nlai; i++)
            {
                ReadKeyword(cmdstr, "LAI_TS", &index, 'i', filename, lno);

                if (i != index - 1)
                {
                    PIHMprintf(VL_ERROR,
                        "Error reading the %dth LAI time series.\n", i + 1);
                    PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n",
                        filename, lno);
                    PIHMexit(EXIT_FAILURE);
                }
                /* Skip header lines */
                NextLine(lai_file, cmdstr, &lno);
                NextLine(lai_file, cmdstr, &lno);
                forc->lai[i].length = CountLine(lai_file, cmdstr, 1, "LAI_TS");
            }

            /* Rewind and read */
            FindLine(lai_file, "BOF", &lno, filename);
            for (i = 0; i < forc->nlai; i++)
            {
                /* Skip header lines */
                NextLine(lai_file, cmdstr, &lno);
                NextLine(lai_file, cmdstr, &lno);
                NextLine(lai_file, cmdstr, &lno);

                forc->lai[i].ftime =
                    (int *)malloc(forc->lai[i].length * sizeof(int));
                forc->lai[i].data =
                    (double **)malloc(forc->lai[i].length * sizeof(double *));
                for (j = 0; j < forc->lai[i].length; j++)
                {
                    forc->lai[i].data[j] = (double *)malloc(sizeof(double));
                    NextLine(lai_file, cmdstr, &lno);
                    if (!ReadTS(cmdstr, &forc->lai[i].ftime[j],
                        &forc->lai[i].data[j][0], 1))
                    {
                        PIHMprintf(VL_ERROR, "Error reading LAI forcing.");
                        PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n",
                            filename, lno);
                        PIHMexit(EXIT_FAILURE);
                    }
                }
            }
        }

        fclose(lai_file);
    }
}

void ReadBc(const char *filename, forc_struct *forc,
    const atttbl_struct *atttbl)
{
    int             i, j;
    FILE           *bc_file;    /* Pointer to .ibc file */
    int             read_bc = 0;
    char            cmdstr[MAXSTRING];
    int             match;
    int             index;
    int             lno = 0;

    for (i = 0; i < nelem; i++)
    {
        for (j = 0; j < NUM_EDGE; j++)
        {
            if (atttbl->bc[i][j] != 0)
            {
                read_bc = 1;
                break;
            }
#if defined(_FBR_)
             if (atttbl->fbr_bc[i][j] != 0)
             {
                read_bc = 1;
                break;
            }
#endif
        }
    }

    forc->nbc = 0;

    if (read_bc)
    {
        bc_file = fopen(filename, "r");
        CheckFile(bc_file, filename);
        PIHMprintf(VL_VERBOSE, " Reading %s\n", filename);

        FindLine(bc_file, "BOF", &lno, filename);

        forc->nbc = CountOccurr(bc_file, "BC_TS");

        FindLine(bc_file, "BOF", &lno, filename);
        if (forc->nbc > 0)
        {
            forc->bc = (tsdata_struct *)malloc(forc->nbc * sizeof(tsdata_struct));

            NextLine(bc_file, cmdstr, &lno);
            for (i = 0; i < forc->nbc; i++)
            {
                match = sscanf(cmdstr, "%*s %d", &index);
                if (match != 1 || i != index - 1)
                {
                    PIHMprintf(VL_ERROR,
                        "Error reading the %dth boundary condition time series.\n",
                        i + 1);
                    PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n",
                        filename, lno);
                    PIHMexit(EXIT_FAILURE);
                }
                /* Skip header lines */
                NextLine(bc_file, cmdstr, &lno);
                NextLine(bc_file, cmdstr, &lno);
                forc->bc[i].length = CountLine(bc_file, cmdstr, 1, "BC_TS");
            }

            /* Rewind and read */
            FindLine(bc_file, "BOF", &lno, filename);
            for (i = 0; i < forc->nbc; i++)
            {
                /* Skip header lines */
                NextLine(bc_file, cmdstr, &lno);
                NextLine(bc_file, cmdstr, &lno);
                NextLine(bc_file, cmdstr, &lno);

                forc->bc[i].ftime =
                    (int *)malloc(forc->bc[i].length * sizeof(int));
                forc->bc[i].data =
                    (double **)malloc(forc->bc[i].length * sizeof(double *));
                for (j = 0; j < forc->bc[i].length; j++)
                {
                    forc->bc[i].data[j] = (double *)malloc(sizeof(double));
                    NextLine(bc_file, cmdstr, &lno);
                    if (!ReadTS(cmdstr, &forc->bc[i].ftime[j],
                        &forc->bc[i].data[j][0], 1))
                    {
                        PIHMprintf(VL_ERROR, "Error reading boundary condition.");
                        PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n",
                            filename, lno);
                        PIHMexit(EXIT_FAILURE);
                    }
                }
            }
        }

        fclose(bc_file);
    }
}

void ReadPara(const char *filename, ctrl_struct *ctrl)
{
    FILE           *para_file;    /* Pointer to .para file */
    char            cmdstr[MAXSTRING];
    int             i;
    int             lno = 0;

    for (i = 0; i < MAXPRINT; i++)
    {
        ctrl->prtvrbl[i] = 0;
    }

    para_file = fopen(filename, "r");
    CheckFile(para_file, filename);
    PIHMprintf(VL_VERBOSE, " Reading %s\n", filename);

    /* Start reading para_file */
    /* Read through parameter file to find parameters */
    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "INIT_MODE", &ctrl->init_type, 'i', filename, lno);
    ctrl->init_type = (ctrl->init_type > RELAX) ? RST_FILE : RELAX;

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "ASCII_OUTPUT", &ctrl->ascii, 'i', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "WATBAL_OUTPUT", &ctrl->waterbal, 'i', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "WRITE_IC", &ctrl->write_ic, 'i', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "UNSAT_MODE", &ctrl->unsat_mode, 'i', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "SURF_MODE", &ctrl->surf_mode, 'i', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "RIV_MODE", &ctrl->riv_mode, 'i', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "START", &ctrl->starttime, 't', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "END", &ctrl->endtime, 't', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "MODEL_STEPSIZE", &ctrl->stepsize, 'i', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "LSM_STEP", &ctrl->etstep, 'i', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "ABSTOL", &ctrl->abstol, 'd', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "RELTOL", &ctrl->reltol, 'd', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "INIT_SOLVER_STEP", &ctrl->initstep, 'd', filename,
        lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "NUM_NONCOV_FAIL", &ctrl->nncfn, 'd', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "MAX_NONLIN_ITER", &ctrl->nnimax, 'd', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "MIN_NONLIN_ITER", &ctrl->nnimin, 'd', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "DECR_FACTOR", &ctrl->decr, 'd', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "INCR_FACTOR", &ctrl->incr, 'd', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ReadKeyword(cmdstr, "MIN_MAXSTEP", &ctrl->stmin, 'd', filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[SURF_CTRL] = ReadPrtCtrl(cmdstr, "SURF", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[UNSAT_CTRL] = ReadPrtCtrl(cmdstr, "UNSAT", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[GW_CTRL] = ReadPrtCtrl(cmdstr, "GW", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[RIVSTG_CTRL] = ReadPrtCtrl(cmdstr, "RIVSTG", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[RIVGW_CTRL] = ReadPrtCtrl(cmdstr, "RIVGW", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[SNOW_CTRL] = ReadPrtCtrl(cmdstr, "SNOW", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[CMC_CTRL] = ReadPrtCtrl(cmdstr, "CMC", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[INFIL_CTRL] = ReadPrtCtrl(cmdstr, "INFIL", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[RECHARGE_CTRL] = ReadPrtCtrl(cmdstr, "RECHARGE", filename,
        lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[EC_CTRL] = ReadPrtCtrl(cmdstr, "EC", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[ETT_CTRL] = ReadPrtCtrl(cmdstr, "ETT", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[EDIR_CTRL] = ReadPrtCtrl(cmdstr, "EDIR", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[RIVFLX0_CTRL] = ReadPrtCtrl(cmdstr, "RIVFLX0", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[RIVFLX1_CTRL] = ReadPrtCtrl(cmdstr, "RIVFLX1", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[RIVFLX2_CTRL] = ReadPrtCtrl(cmdstr, "RIVFLX2", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[RIVFLX3_CTRL] = ReadPrtCtrl(cmdstr, "RIVFLX3", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[RIVFLX4_CTRL] = ReadPrtCtrl(cmdstr, "RIVFLX4", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[RIVFLX5_CTRL] = ReadPrtCtrl(cmdstr, "RIVFLX5", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[RIVFLX6_CTRL] = ReadPrtCtrl(cmdstr, "RIVFLX6", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[RIVFLX7_CTRL] = ReadPrtCtrl(cmdstr, "RIVFLX7", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[RIVFLX8_CTRL] = ReadPrtCtrl(cmdstr, "RIVFLX8", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[RIVFLX9_CTRL] = ReadPrtCtrl(cmdstr, "RIVFLX9", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[RIVFLX10_CTRL] = ReadPrtCtrl(cmdstr, "RIVFLX10", filename,
        lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[SUBFLX_CTRL] = ReadPrtCtrl(cmdstr, "SUBFLX", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[SURFFLX_CTRL] = ReadPrtCtrl(cmdstr, "SURFFLX", filename, lno);

    NextLine(para_file, cmdstr, &lno);
    ctrl->prtvrbl[IC_CTRL] = ReadPrtCtrl(cmdstr, "IC", filename, lno);

    fclose(para_file);

    if (ctrl->etstep < ctrl->stepsize || ctrl->etstep % ctrl->stepsize > 0)
    {
        PIHMprintf(VL_ERROR,
            "Error: Land surface model (ET) step size "
            "should be an integral multiple of model step size.\n");
        PIHMprintf(VL_ERROR, "Error in %s near Line %d.\n", filename, lno);
        PIHMexit(EXIT_FAILURE);
    }
}

void ReadCalib(const char *filename, calib_struct *cal)
{
    char            cmdstr[MAXSTRING];
    FILE           *global_calib;   /* Pointer to .calib file */
    int             lno = 0;

    global_calib = fopen(filename, "r");
    CheckFile(global_calib, filename);
    PIHMprintf(VL_VERBOSE, " Reading %s\n", filename);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "KSATH", &cal->ksath, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "KSATV", &cal->ksatv, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "KINF", &cal->kinfv, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "KMACSATH", &cal->kmach, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "KMACSATV", &cal->kmacv, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "DINF", &cal->dinf, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "DROOT", &cal->rzd, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "DMAC", &cal->dmac, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "POROSITY", &cal->porosity, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "ALPHA", &cal->alpha, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "BETA", &cal->beta, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "MACVF", &cal->areafv, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "MACHF", &cal->areafh, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "VEGFRAC", &cal->vegfrac, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "ALBEDO", &cal->albedo, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "ROUGH", &cal->rough, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "EC", &cal->ec, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "ETT", &cal->ett, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "EDIR", &cal->edir, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "ROUGH_RIV", &cal->rivrough, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "KRIVH", &cal->rivksath, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "KRIVV", &cal->rivksatv, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "BEDTHCK", &cal->rivbedthick, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "RIV_DPTH", &cal->rivdepth, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "RIV_WDTH", &cal->rivshpcoeff, 'd', filename, lno);

#if defined(_NOAH_)
    FindLine(global_calib, "LSM_CALIBRATION", &lno, filename);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "DRIP", &cal->drip, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "CMCMAX", &cal->cmcmax, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "RS", &cal->rsmin, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "CZIL", &cal->czil, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "FXEXP", &cal->fxexp, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "CFACTR", &cal->cfactr, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "RGL", &cal->rgl, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "HS", &cal->hs, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "REFSMC", &cal->smcref, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "WLTSMC", &cal->smcwlt, 'd', filename, lno);
#endif

#if defined(_RT_)
    FindLine(global_calib, "RT_CALIBRATION", &lno, filename);

    CS->Cal.PCO2 = 1.0;
    CS->Cal.Keq = 1.0;
    CS->Cal.Site_den = 1.0;
    CS->Cal.SSA = 1.0;
    CS->Cal.Prep_conc = 1.0;
#endif

    /*
     * Scenarios
     */
    FindLine(global_calib, "SCENARIO", &lno, filename);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "PRCP", &cal->prcp, 'd', filename, lno);

    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "SFCTMP", &cal->sfctmp, 'd', filename, lno);
    
    // 02.12 by Wei Zhi
    FindLine(global_calib, "RT", &lno, filename);
    
    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "rate", &cal->rate, 'd', filename, lno);
    printf( "\n cal->rate = %f \n", cal->rate);
    
    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "ssa", &cal->ssa, 'd', filename, lno);
    printf( " cal->ssa = %f \n", cal->ssa);
    
    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "gwinflux", &cal->gwinflux, 'd', filename, lno);
    printf( " cal->gwinflux = %f \n", cal->gwinflux);
    
    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "prcpconc", &cal->prcpconc, 'd', filename, lno);
    printf( " cal->prcpconc = %f \n", cal->prcpconc);
    
    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "initconc", &cal->initconc, 'd', filename, lno);
    printf( " cal->initconc = %f \n", cal->initconc);
    
    // 03.03 by Wei Zhi
    NextLine(global_calib, cmdstr, &lno);
    ReadKeyword(cmdstr, "Xsorption", &cal->Xsorption, 'd', filename, lno);
    printf( " cal->Xsorption = %f \n", cal->Xsorption);

    fclose(global_calib);
}

void ReadIc(const char *filename, elem_struct *elem, river_struct *river)
{
    FILE           *ic_file;
    int             i;
    int             size;

    ic_file = fopen(filename, "rb");
    CheckFile(ic_file, filename);
    PIHMprintf(VL_VERBOSE, " Reading %s\n", filename);

    fseek(ic_file, 0L, SEEK_END);
    size = ftell(ic_file);

    if (size !=
        (int)(sizeof(ic_struct) * nelem + sizeof(river_ic_struct) * nriver))
    {
        PIHMprintf(VL_ERROR,
            "Error in initial condition file %s.\n"
            "The file size does not match requirement.\n", filename);
        PIHMprintf(VL_ERROR, "Please use a correct initial condition file.\n");
        PIHMexit(EXIT_FAILURE);
    }

    fseek(ic_file, 0L, SEEK_SET);

    for (i = 0; i < nelem; i++)
    {
        fread(&elem[i].ic, sizeof(ic_struct), 1, ic_file);
    }

    for (i = 0; i < nriver; i++)
    {
        fread(&river[i].ic, sizeof(river_ic_struct), 1, ic_file);
    }

    fclose(ic_file);
}

void ReadTecplot(const char *filename, ctrl_struct *ctrl)
{
    FILE           *tecplot_file;
    char            cmdstr[MAXSTRING];
    int             i;
    int             lno = 0;

    for (i = 0; i < MAXPRINT; i++)
    {
        ctrl->tpprtvrbl[i] = 0;
    }

    tecplot_file = fopen(filename, "r");
    CheckFile(tecplot_file, filename);
    PIHMprintf(VL_VERBOSE, " Reading %s\n", filename);

    NextLine(tecplot_file, cmdstr, &lno);
    ctrl->tpprtvrbl[SURF_CTRL] = ReadPrtCtrl(cmdstr, "SURF", filename, lno);

    NextLine(tecplot_file, cmdstr, &lno);
    ctrl->tpprtvrbl[UNSAT_CTRL] = ReadPrtCtrl(cmdstr, "UNSAT", filename, lno);

    NextLine(tecplot_file, cmdstr, &lno);
    ctrl->tpprtvrbl[GW_CTRL] = ReadPrtCtrl(cmdstr, "GW", filename, lno);

    NextLine(tecplot_file, cmdstr, &lno);
    ctrl->tpprtvrbl[RIVSTG_CTRL] = ReadPrtCtrl(cmdstr, "RIVSTG", filename, lno);

    NextLine(tecplot_file, cmdstr, &lno);
    ctrl->tpprtvrbl[RIVGW_CTRL] = ReadPrtCtrl(cmdstr, "RIVGW", filename, lno);

    fclose(tecplot_file);
}

void FreeData(pihm_struct pihm)
{
    int             i, j;

    /* Free river input structure */
    free(pihm->rivtbl.fromnode);
    free(pihm->rivtbl.tonode);
    free(pihm->rivtbl.down);
    free(pihm->rivtbl.leftele);
    free(pihm->rivtbl.rightele);
    free(pihm->rivtbl.shp);
    free(pihm->rivtbl.matl);
    free(pihm->rivtbl.bc);
    free(pihm->rivtbl.rsvr);

    free(pihm->shptbl.depth);
    free(pihm->shptbl.intrpl_ord);
    free(pihm->shptbl.coeff);

    free(pihm->matltbl.rough);
    free(pihm->matltbl.cwr);
    free(pihm->matltbl.ksath);
    free(pihm->matltbl.ksatv);
    free(pihm->matltbl.bedthick);

    /* Free mesh input structure */
    for (i = 0; i < nelem; i++)
    {
        free(pihm->meshtbl.node[i]);
        free(pihm->meshtbl.nabr[i]);
    }
    free(pihm->meshtbl.node);
    free(pihm->meshtbl.nabr);
    free(pihm->meshtbl.x);
    free(pihm->meshtbl.y);
    free(pihm->meshtbl.zmin);
    free(pihm->meshtbl.zmax);
#if defined(_FBR_)
    free(pihm->meshtbl.zbed);
#endif

    /* Free attribute input structure */
    for (i = 0; i < nelem; i++)
    {
        free(pihm->atttbl.bc[i]);
#if defined(_FBR_)
        free(pihm->atttbl.fbr_bc[i]);
#endif
    }
    free(pihm->atttbl.bc);
#if defined(_FBR_)
    free(pihm->atttbl.fbr_bc);
#endif
    free(pihm->atttbl.soil);
    free(pihm->atttbl.geol);
    free(pihm->atttbl.lc);
    free(pihm->atttbl.meteo);
    free(pihm->atttbl.lai);
    free(pihm->atttbl.source);

    /* Free soil input structure */
    free(pihm->soiltbl.silt);
    free(pihm->soiltbl.clay);
    free(pihm->soiltbl.om);
    free(pihm->soiltbl.bd);
    free(pihm->soiltbl.kinfv);
    free(pihm->soiltbl.ksatv);
    free(pihm->soiltbl.ksath);
    free(pihm->soiltbl.smcmax);
    free(pihm->soiltbl.smcmin);
    free(pihm->soiltbl.qtz);
    free(pihm->soiltbl.alpha);
    free(pihm->soiltbl.beta);
    free(pihm->soiltbl.areafh);
    free(pihm->soiltbl.areafv);
    free(pihm->soiltbl.dmac);
    free(pihm->soiltbl.smcref);
    free(pihm->soiltbl.smcwlt);

#if defined(_FBR_)
    /* Free geol input structure */
    free (pihm->geoltbl.ksatv);
    free (pihm->geoltbl.ksath);
    free (pihm->geoltbl.smcmax);
    free (pihm->geoltbl.smcmin);
    free (pihm->geoltbl.alpha);
    free (pihm->geoltbl.beta);
#endif

    /* Free landcover input structure */
    free(pihm->lctbl.laimax);
    free(pihm->lctbl.laimin);
    free(pihm->lctbl.vegfrac);
    free(pihm->lctbl.albedomin);
    free(pihm->lctbl.albedomax);
    free(pihm->lctbl.emissmin);
    free(pihm->lctbl.emissmax);
    free(pihm->lctbl.z0min);
    free(pihm->lctbl.z0max);
    free(pihm->lctbl.hs);
    free(pihm->lctbl.snup);
    free(pihm->lctbl.rgl);
    free(pihm->lctbl.rsmin);
    free(pihm->lctbl.rough);
    free(pihm->lctbl.rzd);

    /* Free forcing input structure */
    if (pihm->forc.nriverbc > 0)
    {
        for (i = 0; i < pihm->forc.nriverbc; i++)
        {
            for (j = 0; j < pihm->forc.riverbc[i].length; j++)
            {
                free(pihm->forc.riverbc[i].data[j]);
            }
            free(pihm->forc.riverbc[i].ftime);
            free(pihm->forc.riverbc[i].data);
        }
        free(pihm->forc.riverbc);
    }

    if (pihm->forc.nmeteo > 0)
    {
        for (i = 0; i < pihm->forc.nmeteo; i++)
        {
            for (j = 0; j < pihm->forc.meteo[i].length; j++)
            {
                free(pihm->forc.meteo[i].data[j]);
            }
            free(pihm->forc.meteo[i].ftime);
            free(pihm->forc.meteo[i].data);
            free(pihm->forc.meteo[i].value);
        }
        free(pihm->forc.meteo);
    }

    if (pihm->forc.nlai > 0)
    {
        for (i = 0; i < pihm->forc.nlai; i++)
        {
            for (j = 0; j < pihm->forc.lai[i].length; j++)
            {
                free(pihm->forc.lai[i].data[j]);
            }
            free(pihm->forc.lai[i].ftime);
            free(pihm->forc.lai[i].data);
            free(pihm->forc.lai[i].value);
        }
        free(pihm->forc.lai);
    }

    if (pihm->forc.nbc > 0)
    {
        for (i = 0; i < pihm->forc.nbc; i++)
        {
            for (j = 0; j < pihm->forc.bc[i].length; j++)
            {
                free(pihm->forc.bc[i].data[j]);
            }
            free(pihm->forc.bc[i].ftime);
            free(pihm->forc.bc[i].data);
            free(pihm->forc.bc[i].value);
        }
        free(pihm->forc.bc);
    }

#if defined(_NOAH_)
    if (pihm->forc.nrad > 0)
    {
        for (i = 0; i < pihm->forc.nrad; i++)
        {
            for (j = 0; j < pihm->forc.rad[i].length; j++)
            {
                free(pihm->forc.rad[i].data[j]);
            }
            free(pihm->forc.rad[i].ftime);
            free(pihm->forc.rad[i].data);
            free(pihm->forc.rad[i].value);
        }
        free(pihm->forc.rad);
    }
#endif

#if defined(_BGC_)
    free(pihm->epctbl.woody);
    free(pihm->epctbl.evergreen);
    free(pihm->epctbl.c3_flag);
    free(pihm->epctbl.phenology_flag);
    free(pihm->epctbl.onday);
    free(pihm->epctbl.offday);
    free(pihm->epctbl.transfer_days);
    free(pihm->epctbl.litfall_days);
    free(pihm->epctbl.leaf_turnover);
    free(pihm->epctbl.froot_turnover);
    free(pihm->epctbl.livewood_turnover);
    free(pihm->epctbl.daily_mortality_turnover);
    free(pihm->epctbl.daily_fire_turnover);
    free(pihm->epctbl.alloc_frootc_leafc);
    free(pihm->epctbl.alloc_newstemc_newleafc);
    free(pihm->epctbl.alloc_newlivewoodc_newwoodc);
    free(pihm->epctbl.alloc_crootc_stemc);
    free(pihm->epctbl.alloc_prop_curgrowth);
    free(pihm->epctbl.avg_proj_sla);
    free(pihm->epctbl.sla_ratio);
    free(pihm->epctbl.lai_ratio);
    free(pihm->epctbl.ext_coef);
    free(pihm->epctbl.flnr);
    free(pihm->epctbl.psi_open);
    free(pihm->epctbl.psi_close);
    free(pihm->epctbl.vpd_open);
    free(pihm->epctbl.vpd_close);
    free(pihm->epctbl.froot_cn);
    free(pihm->epctbl.leaf_cn);
    free(pihm->epctbl.livewood_cn);
    free(pihm->epctbl.deadwood_cn);
    free(pihm->epctbl.leaflitr_cn);
    free(pihm->epctbl.leaflitr_flab);
    free(pihm->epctbl.leaflitr_fucel);
    free(pihm->epctbl.leaflitr_fscel);
    free(pihm->epctbl.leaflitr_flig);
    free(pihm->epctbl.frootlitr_flab);
    free(pihm->epctbl.frootlitr_fucel);
    free(pihm->epctbl.frootlitr_fscel);
    free(pihm->epctbl.frootlitr_flig);
    free(pihm->epctbl.deadwood_fucel);
    free(pihm->epctbl.deadwood_fscel);
    free(pihm->epctbl.deadwood_flig);

    if (pihm->co2.varco2 > 0)
    {
        for (j = 0; j < pihm->forc.co2[0].length; j++)
        {
            free(pihm->forc.co2[0].data[j]);
        }
        free(pihm->forc.co2[0].ftime);
        free(pihm->forc.co2[0].data);
        free(pihm->forc.co2[0].value);
    }
    free(pihm->forc.co2);

    if (pihm->ndepctrl.varndep > 0)
    {
        for (j = 0; j < pihm->forc.ndep[0].length; j++)
        {
            free(pihm->forc.ndep[0].data[j]);
        }
        free(pihm->forc.ndep[0].ftime);
        free(pihm->forc.ndep[0].data);
        free(pihm->forc.ndep[0].value);
    }
    free(pihm->forc.ndep);
#endif

    free(pihm->ctrl.tout);

    /*
     * Close files
     */
    if (pihm->ctrl.waterbal)
    {
        fclose(pihm->print.watbal_file);
    }
    if (debug_mode)
    {
        fclose(pihm->print.cvodeperf_file);
    }
    for (i = 0; i < pihm->print.nprint; i++)
    {
        free(pihm->print.varctrl[i].var);
        free(pihm->print.varctrl[i].buffer);
        fclose(pihm->print.varctrl[i].datfile);
        if (pihm->ctrl.ascii)
        {
            fclose(pihm->print.varctrl[i].txtfile);
        }
    }
    if (tecplot)
    {
        for (i = 0; i < pihm->print.ntpprint; i++)
        {
            fclose(pihm->print.tp_varctrl[i].datfile);
        }
    }
    free(pihm->elem);
    free(pihm->river);
}
