#ifndef PIHM_FUNC_HEADER
#define PIHM_FUNC_HEADER

#define _ARITH_

/* State variables */
#define SURF(i)          i
#define UNSAT(i)         i + nelem
#define GW(i)            i + 2 * nelem
#define RIVSTG(i)        i + 3 * nelem
#define RIVGW(i)         i + 3 * nelem + nriver
#if defined(_FBR_)
# define FBRUNSAT(i)     i + 3 * nelem + 2 * nriver
# define FBRGW(i)        i + 4 * nelem + 2 * nriver
#endif
#if defined(_BGC_) && !defined(_LUMPED_)
# define SURFN(i)        i + 3 * nelem + 2 * nriver
# define SMINN(i)        i + 4 * nelem + 2 * nriver
# define STREAMN(i)      i + 5 * nelem + 2 * nriver
# define RIVBEDN(i)      i + 5 * nelem + 3 * nriver
#else
# define LUMPED_SMINN    3 * nelem + 2 * nriver
#endif

#define RiverArea(...)     _RiverWdthAreaPerim(RIVER_AREA, __VA_ARGS__)
#define RiverEqWid(...)    _RiverWdthAreaPerim(RIVER_WDTH, __VA_ARGS__)
#define RiverPerim(...)    _RiverWdthAreaPerim(RIVER_PERIM, __VA_ARGS__)

#define AvgElev(...)      _WsAreaElev(WS_ZMAX, __VA_ARGS__)
#define AvgZmin(...)      _WsAreaElev(WS_ZMIN, __VA_ARGS__)
#define TotalArea(...)    _WsAreaElev(WS_AREA, __VA_ARGS__)

/* CVode functions */
#if defined(_CVODE_OMP)
# define N_VNew(N)    N_VNew_OpenMP(N, nthreads)
# define NV_DATA      NV_DATA_OMP
# define NV_Ith       NV_Ith_OMP
#else
# define N_VNew(N)    N_VNew_Serial(N)
# define NV_DATA      NV_DATA_S
# define NV_Ith       NV_Ith_S
#endif

/* PIHM system function */
#define PIHMexit(...)               _PIHMexit(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define PIHMprintf(...)             _PIHMprintf(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#if defined(_WIN32) || defined(_WIN64)
# define PIHMmkdir(path)            _mkdir((path))
# define PIHMaccess(path, amode)    _access((path), (amode))
#else
# define PIHMmkdir(path)            mkdir(path, 0755)
# define PIHMaccess(path, amode)    access((path), (amode))
#endif
#if defined(_MSC_VER)
# define timegm                     _mkgmtime
# define strcasecmp                 _stricmp
#endif

#define Cycles_exit      PIHMexit
#define Cycles_printf    PIHMprintf

/*
 * Function Declarations
 */
void            _PIHMexit(const char *, int, const char *, int);
void            _PIHMprintf(const char *, int, const char *, int, const char *,
    ...);
double          _RiverWdthAreaPerim(int, int, double, double);
void            AdjCVodeMaxStep(void *, ctrl_struct *);
void            ApplyBc(forc_struct *, elem_struct *, river_struct *, int);
void            ApplyElemBc(forc_struct *, elem_struct *, int);
#if defined(_NOAH_)
void            ApplyForc(forc_struct *, elem_struct *, int, int,
    const siteinfo_struct *);
#else
void            ApplyForc(forc_struct *, elem_struct *, int);
#endif
void            ApplyLai(forc_struct *, elem_struct *, int);
#if defined(_NOAH_)
void            ApplyMeteoForc(forc_struct *, elem_struct *, int, int,
    const siteinfo_struct *);
#else
void            ApplyMeteoForc(forc_struct *, elem_struct *, int);
#endif
void            ApplyRiverBc(forc_struct *, river_struct *, int);
void            AsciiArt();
double          AvgKv(const soil_struct *, double, double, double);
double          AvgH(double, double, double);
double          AvgHsurf(double, double, double);
void            BackupInput(const char *);
void            BoundFluxElem(int, int, const bc_struct *,
    const wstate_struct *, const topo_struct *, const soil_struct *,
    wflux_struct *);
void            CalcModelStep(ctrl_struct *);
//double          ChanFlowElemToRiver(const elem_struct *, double,
double          ChanFlowElemToRiver(elem_struct *, double,                            // 12.30, new RT change
    const river_struct *, double);
double          ChanFlowRiverToRiver(const river_struct *, const river_struct *,
    int);
double          ChanLeak(const river_wstate_struct *, const river_topo_struct *,
    const shp_struct *, const matl_struct *);
int             CheckCVodeFlag(int);
void            CheckDy(double, const char *, const char *, int, double);
void            CheckFile(const FILE *, const char *);
void            CorrElev(elem_struct *, river_struct *);
int             CountLine(FILE *, char *, int, ...);
int             CountOccurr(FILE *, const char *);
void            CreateOutputDir(char *);
double          DhByDl(const double *, const double *, const double *);
double          EffKh(const soil_struct *, double);
double          EffKinf(const soil_struct *, double, double, double, double,
    double);
double          EffKv(const soil_struct *, double, int);
void            EtExtract(elem_struct *);
double          FieldCapacity(double, double, double, double, double);
void            FindLine(FILE *, const char *, int *, const char *);
void            FreeData(pihm_struct);
void            FrictSlope(const elem_struct *, const river_struct *, int,
    double *, double *);
void            Hydrol(elem_struct *, river_struct *, const ctrl_struct *);
double          Infil(const wstate_struct *, const wstate_struct *,
    const wflux_struct *, const topo_struct *, const soil_struct *, double);
void            InitEFlux(eflux_struct *);
void            InitEState(estate_struct *);
#if defined(_BGC_)
void            InitForc(elem_struct *, forc_struct *, const calib_struct *,
    int, int);
#else
void            InitForc(elem_struct *, forc_struct *, const calib_struct *);
#endif
void            Initialize(pihm_struct, N_Vector, void **);
void            InitLc(elem_struct *, const lctbl_struct *,
    const calib_struct *);
void            InitMesh(elem_struct *, const meshtbl_struct *);
void            InitOutputFile(print_struct *, const char *, int, int);
void            InitPrtVarCtrl(const char *, const char *, int, int, int,
    varctrl_struct *);
//void            InitRiver(river_struct *, elem_struct *, const rivtbl_struct *,
void            InitRiver (pihm_struct, river_struct *, elem_struct *, const rivtbl_struct *,      // 12.30, new RT add
    const shptbl_struct *, const matltbl_struct *, const meshtbl_struct *,
    const calib_struct *);
void            InitRiverWFlux(river_wflux_struct *);
void            InitRiverWState(river_wstate_struct *);
#if defined(_NOAH_)
void            InitSoil(elem_struct *, const soiltbl_struct *,
    const noahtbl_struct *, const calib_struct *);
#else
void            InitSoil(elem_struct *, const soiltbl_struct *,
    const calib_struct *);
#endif
void            InitSurfL(elem_struct *, const river_struct *,
    const meshtbl_struct *);
void            InitTecPrtVarCtrl(const char *, const char *, int, int, int,
    int, int, varctrl_struct *);
void            InitTopo(elem_struct *, const meshtbl_struct *);
void            InitVar(elem_struct *, river_struct *, N_Vector);
void            InitWbFile(char *, char *, FILE *);
void            InitWFlux(wflux_struct *);
void            InitWState(wstate_struct *);
void            IntcpSnowEt(int, double, elem_struct *, const calib_struct *);
void            IntrplForc(tsdata_struct *, int, int);
double          KrFunc(double, double, double);
void            LateralFlow(elem_struct *, const river_struct *, int);
void            MapOutput(const int *, const int *, const elem_struct *,
    const river_struct *, const meshtbl_struct *, const char *, print_struct *);
void            MassBalance(const wstate_struct *, const wstate_struct *,
    wflux_struct *, double *, const soil_struct *, double, double);
double          MonthlyLai(int, int);
double          MonthlyMf(int);
double          MonthlyRl(int, int);
void            NextLine(FILE *, char *, int *);
int             NumStateVar(void);
int             Ode(realtype, N_Vector, N_Vector, void *);
double          OutletFlux(int, const river_wstate_struct *,
    const river_topo_struct *, const shp_struct *, const matl_struct *,
    const river_bc_struct *);
double          OverLandFlow(double, double, double, double, double);
double          OvlFlowElemToElem(const elem_struct *, const elem_struct *, int,
    double, int);
double          OvlFlowElemToRiver(const elem_struct *, const river_struct *);
void            ParseCmdLineParam(int, char *[], char *);
void            PIHM(pihm_struct, void *, N_Vector, int, int, double);
pihm_t_struct   PIHMTime(int);
void            PrintCVodeFinalStats(void *);
void            PrintData(varctrl_struct *, int, int, int, int);
void            PrintDataTecplot(varctrl_struct *, int, int, int);
void            PrintInit(const elem_struct *, const river_struct *,
    const char *, int, int, int, int);
int             PrintNow(int, int, const pihm_t_struct *);
void            PrintPerf(void *, int, int, double, double, double, FILE *);
void            PrintWaterBal(FILE *, int, int, int, const elem_struct *,
    const river_struct *);
double          Psi(double, double, double);
double          PtfAlpha(double, double, double, double, int);
double          PtfBeta(double, double, double, double, int);
double          PtfKv(double, double, double, double, int);
double          PtfThetar(double, double, double, double, int);
double          PtfThetas(double, double, double, double, int);
double          Qtz(int);
int             Readable(const char *);
void            ReadAlloc(pihm_struct);
void            ReadAtt(const char *, atttbl_struct *);
void            ReadBc(const char *, forc_struct *, const atttbl_struct *);
void            ReadCalib(const char *, calib_struct *);
void            ReadForc(const char *, forc_struct *);
void            ReadIc(const char *, elem_struct *, river_struct *);
int             ReadKeyword(const char *, const char *, void *, char,
    const char *, int);
void            ReadLai(const char *, forc_struct *, const atttbl_struct *);
void            ReadLc(const char *, lctbl_struct *);
void            ReadMesh(const char *, meshtbl_struct *);
void            ReadPara(const char *, ctrl_struct *);
int             ReadPrtCtrl(const char *, const char *, const char *, int);
void            ReadRiver(const char *, rivtbl_struct *, shptbl_struct *,
    matltbl_struct *, forc_struct *);
void            ReadSoil(const char *, soiltbl_struct *);
void            ReadTecplot(const char *, ctrl_struct *);
int             ReadTS(const char *, int *, double *, int);
double          Recharge(const wstate_struct *, const wflux_struct *,
    const soil_struct *);
void            RiverFlow(elem_struct *, river_struct *, int);
void            RiverToElem(river_struct *, elem_struct *, elem_struct *);
#if defined(_OPENMP)
void            RunTime(double, double *, double *);
#else
void            RunTime (clock_t, double *, double *);
#endif
void            RelaxIc(elem_struct *, river_struct *);
void            SetCVodeParam(pihm_struct, void *, N_Vector);
int             SoilTex(double, double);
void            SolveCVode(int, int *, int, double, void *, N_Vector);
int             StrTime(const char *);
//double          SubFlowElemToElem(const elem_struct *, const elem_struct *,
double          SubFlowElemToElem(elem_struct *, const elem_struct *,               // 12.30, new RT change
    int);
double          SubFlowElemToRiver(const elem_struct *, double,
    const river_struct *, double, double);
double          SubFlowRiverToRiver(const river_struct *, double,
    const river_struct *, double);
void            Summary(elem_struct *, river_struct *, N_Vector, double);
double          SurfH(double);
void            UpdPrintVar(varctrl_struct *, int, int);
void            UpdPrintVarT(varctrl_struct *, int);
void            VerticalFlow(elem_struct *, double);
double          WiltingPoint(double, double, double, double);

/*
 * Fractured bedrock functions
 */
#if defined(_FBR_)
double          FbrBoundFluxElem(int, int, const bc_struct *,
    const wstate_struct *, const topo_struct *, const geol_struct *);
double          FbrFlowElemToElem(const elem_struct *, const elem_struct *,
    double, double);
double          FbrInfil(const wstate_struct *, const soil_struct *,
    const geol_struct *, const topo_struct *);
double          FbrRecharge(const wstate_struct *, const wflux_struct *,
    const geol_struct *);
void            InitGeol (elem_struct *, const geoltbl_struct *,
        const calib_struct *);
void            ReadBedrock(const char *, atttbl_struct *, meshtbl_struct *,
    ctrl_struct *);
void            ReadGeol(const char *, geoltbl_struct *);
#endif

/*
 * Noah functions
 */
#if defined(_NOAH_)
double          _WsAreaElev(int, const elem_struct *);
void            AlCalc(pstate_struct *, double, int);
void            CalcLatFlx(const pstate_struct *, wflux_struct *, double);
void            CalcSlopeAspect(elem_struct *, const meshtbl_struct *);
void            CalHum(pstate_struct *, estate_struct *);
void            CanRes(const wstate_struct *, const estate_struct *,
    const eflux_struct *, pstate_struct *, const soil_struct *,
    const epconst_struct *);
double          CSnow(double);
void            DefSldpth(double *, int *, double *, double, const double *,
    int);
void            DEvap(const wstate_struct *, wflux_struct *,
    const pstate_struct *, const lc_struct *, const soil_struct *);
# if defined(_CYCLES_)
void            Evapo(const wstate_struct *, wflux_struct *,
    const pstate_struct *, const lc_struct *, const soil_struct *,
    comm_struct *, residue_struct *, const estate_struct *, double);
# else
void            Evapo(const wstate_struct *, wflux_struct *,
    const pstate_struct *, const lc_struct *, const soil_struct *, double);
# endif
int             FindLayer(const double *, int, double);
int             FindWaterTable(const double *, int, double, double *);
double          FrozRain(double, double);
double          GwTransp(double, const double *, int, int);
void            HRT(wstate_struct *, const estate_struct *, eflux_struct *,
    const pstate_struct *, const lc_struct *, const soil_struct *, double *,
    double, double, double, double, double *, double *, double *);
void            InitLsm(elem_struct *, const ctrl_struct *,
    const noahtbl_struct *, const calib_struct *);
void            Noah(elem_struct *, double);
void            NoahHydrol(elem_struct *, double);
# if defined(_CYCLES_)
void            NoPac(wstate_struct *, wflux_struct *, estate_struct *,
    eflux_struct *, pstate_struct *, const lc_struct *, const soil_struct *,
    comm_struct *, residue_struct *, double, double);
#else
void            NoPac(wstate_struct *, wflux_struct *, estate_struct *,
    eflux_struct *, pstate_struct *, const lc_struct *, const soil_struct *,
    double, double);
# endif
void            PcpDrp(wstate_struct *, wflux_struct *, const lc_struct *,
    double, double);
void            Penman(wflux_struct *, const estate_struct *, eflux_struct *,
    pstate_struct *, double *, double, int, int);
double          Pslhs(double);
double          Pslhu(double);
double          Pslms(double);
double          Pslmu(double);
double          Psphs(double);
double          Psphu(double);
double          Pspms(double);
double          Pspmu(double);
void            ReadLsm(const char *, siteinfo_struct *, ctrl_struct *,
    noahtbl_struct *);
void            ReadRad(const char *, forc_struct *);
void            RootDist(const double *, int, int, double *);
void            Rosr12(double *, const double *, const double *, double *,
    const double *, double *, int);
void            SfcDifOff(pstate_struct *, const lc_struct *, double, double,
    int);
# if defined(_CYCLES_)
void            SFlx(wstate_struct *, wflux_struct *, estate_struct *,
    eflux_struct *, pstate_struct *, lc_struct *, epconst_struct *,
    soil_struct *, comm_struct *, residue_struct *, double);
# else
void            SFlx(wstate_struct *, wflux_struct *, estate_struct *,
    eflux_struct *, pstate_struct *, lc_struct *, epconst_struct *,
    soil_struct *, double);
# endif
void            ShFlx(wstate_struct *, estate_struct *, eflux_struct *,
    const pstate_struct *, const lc_struct *, const soil_struct *, double,
    double, double, double);
# if defined(_CYCLES_)
void            SmFlx(wstate_struct *, wflux_struct *, pstate_struct *,
    const soil_struct *, residue_struct *, double);
# else
void            SmFlx(wstate_struct *, wflux_struct *, pstate_struct *,
    const soil_struct *, double);
# endif
double          SnFrac(double, double, double, double);
void            SnkSrc(double *, double, double, double *,
    const soil_struct *, const double *, double, int, double);
# if defined(_CYCLES_)
void            SnoPac(wstate_struct *, wflux_struct *, estate_struct *,
    eflux_struct *, pstate_struct *, const lc_struct *, const soil_struct *,
    comm_struct *, residue_struct *, int, double, double, double, double);
# else
void            SnoPac(wstate_struct *, wflux_struct *, estate_struct *,
    eflux_struct *, pstate_struct *, const lc_struct *, const soil_struct *,
    int, double, double, double, double);
# endif
void            SnowNew(const estate_struct *, double, pstate_struct *);
void            SnowPack(double, double, double *, double *, double, double);
double          Snowz0(double, double, double);
# if defined(_CYCLES_)
void            SRT(wstate_struct *, wflux_struct *, pstate_struct *,
    const soil_struct *, residue_struct *, double, double *, double *, double *,
    double *, double *);
# else
void            SRT(wstate_struct *, wflux_struct *, pstate_struct *,
    const soil_struct *, double *, double *, double *, double *, double *);
# endif
void            SStep(wstate_struct *, wflux_struct *, pstate_struct *,
    const soil_struct *, double *, double *, double *, double *, double *,
    double);
void            SunPos(const siteinfo_struct *, int, spa_data *);
double          TBnd(double, double, const double *, double, int, int);
double          TDfCnd(double, double, double, double, double);
double          TmpAvg(double, double, double, const double *, int);
double          TopoRadn(const topo_struct *, double, double, double, double);
void            Transp(const wstate_struct *, wflux_struct *,
    const pstate_struct *, const lc_struct *, const soil_struct *);
void            WDfCnd(double *, double *, double, double, const soil_struct *);
#endif

#if defined(_DAILY_)
void            DailyVar(int, int, elem_struct *, double);
void            InitDailyStruct(elem_struct *);
#endif

#if defined(_CYCLES_)
void            DailyCycles(int, pihm_struct);
void            FirstDOY(int *, int, int, soilc_struct *, residue_struct *,
    const soil_struct *);
void            ReadCyclesCtrl(char *, agtbl_struct *, ctrl_struct *);
void            ReadCyclesIC(char *, elem_struct *, river_struct *);
void            ReadSoilInit(char *, soiltbl_struct *);
void            ReadCrop(char *, croptbl_struct *);
void            ReadOperation(const agtbl_struct *, mgmttbl_struct *,
    const croptbl_struct *);
int             CropExist(char *, const croptbl_struct *);
void            InitCycles(elem_struct *, river_struct *,
    const ctrl_struct *, const mgmttbl_struct *, const agtbl_struct *,
    const croptbl_struct *, const soiltbl_struct *);
void            InitializeSoil(soil_struct *, const soiltbl_struct *,
    const pstate_struct *, const cyclesic_struct *, int, int);
double          BulkDensity(double, double, double);
void            InitializeResidue(residue_struct *, int);
void            InitializeSoilCarbon(soilc_struct *, int);
void            ComputeFactorComposite(soilc_struct *, int, int, int,
    soil_struct *);
void            ComputeSoilCarbonBalanceMB(soilc_struct *, int,
    residue_struct *, soil_struct *, double *);
void            ComputeSoilCarbonBalance(soilc_struct *, int,
    residue_struct *, soil_struct *, double *);
void            StoreOutput(soilc_struct *, int, int, double *);
double          Aeration(double);
double          Moisture(double);
double          TemperatureFunction(double);
double          MaximumAbgdHumificationFactor(double);
double          MaximumRootHumificationFactor(double);
double          MaximumRhizHumificationFactor(double);
double          MaximumManuHumificationFactor(double);
double          NitrogenMineralization(double, double, double, double);
double          CNdestiny(double, double);
double          PoolNitrogenMineralization(double, double, double, double,
    double);
double          Function_CNnew(double, double);
void            WaterUptake(comm_struct *, soil_struct *, double,
    wflux_struct *, double, double);
double          TemperatureLimitation(double, double, double);
void            CalcRootFraction(double *, soil_struct *, crop_struct *);
int             DOY(int);
int             IsLeapYear(int);
void            DailyOperations(int, int, cropmgmt_struct *, comm_struct *,
    residue_struct *, ctrl_struct *, snow_struct *, soil_struct *,
    soilc_struct *, weather_struct *);
double          Depth_Limitation_To_Evaporation(double);
double          Water_Content_Limitation_To_Evaporation(double, double, double);
void            Evaporation(soil_struct *, const comm_struct *,
    residue_struct *, double, double);
void            LastDOY(int, int, soil_struct *, soilc_struct *,
    residue_struct *);
void            GrowingCrop(int, int, comm_struct *, residue_struct *,
    const ctrl_struct *, soil_struct *, soilc_struct *, cropmgmt_struct *,
    const weather_struct *, const snow_struct *);
void            CropStage(int, comm_struct *, int);
double          FinalHarvestDate(int, int, double, double, double);
void            Phenology(int, int, const weather_struct *, comm_struct *);
double          ThermalTime(double, double, double, double);
void            RadiationInterception(int, int, comm_struct *);
void            Processes(int, int, int, comm_struct *, residue_struct *,
    const weather_struct *, soil_struct *, soilc_struct *);
void            CropNitrogenConcentration(double *, double *, double *,
    double *, double *, double *, double *, double, const crop_struct *);
void            CropNitrogenStress(double, double, double, crop_struct *);
void            CropGrowth(int, int, double *, double, crop_struct *,
    residue_struct *, const weather_struct *);
void            CropNitrogenDemand(double, double, double *, double *,
    double *, double *, crop_struct *);
void            PotentialSoluteUptakeOption2(double *, double *, double, int,
    const double *, const double *, const double *, const double *,
    const double *);
void            CropNitrogenUptake(double *, double *, double *, double *,
    double *, int, double, double, double *, double *, double *,
    comm_struct *, soil_struct *);
void            DistributeRootDetritus(double, double, double, double,
    const soil_struct *, const crop_struct *, residue_struct *, soilc_struct *);
double          ShootBiomassPartitioning(double, double, double, int);
double          TemperatureFunctionGrowth(double, double, double, double);
int             ForcedClipping(int, comm_struct *);
void            GrainHarvest(int, int, crop_struct *, residue_struct *,
    soil_struct *, soilc_struct *);
void            ComputeColdDamage(int, int, crop_struct *,
    const weather_struct *, const snow_struct *, residue_struct *);
double          ColdDamage(double, double, double);
void            ForageAndSeedHarvest(int, int, crop_struct *,
    residue_struct *, soil_struct *, soilc_struct *);
void            HarvestCrop(int, int, crop_struct *, residue_struct *,
    soil_struct *, soilc_struct *);
void            PlantingCrop(comm_struct *, const cropmgmt_struct *, int);
void            AddCrop(crop_struct *);
void            KillCrop(crop_struct *);
void            UpdateCommunity(comm_struct *);
double          ComputeHarvestIndex(double, double, double, double, double);
int             IsOperationToday(int, int, const void *, int, int *, int *,
    int);
void            ApplyFertilizer(const fixfert_struct *, soil_struct *,
    residue_struct *);
void            UpdateOperationStatus(int *, int);
void            FieldOperation(int, int, int, cropmgmt_struct *,
    comm_struct *, soil_struct *, residue_struct *, ctrl_struct *,
    soilc_struct *, weather_struct *);
void            ExecuteTillage(double *, const tillage_struct *, double *,
    soil_struct *, residue_struct *);
void            TillageFactorSettling(double *, int, const double *,
    const double *);
double          Fraction(double, double, double, double, double);
void            ComputeTillageFactor(const tillage_struct *, double *,
    const soil_struct *, const double *, double);
double          ComputeTextureFactor(double);
void            ComputeResidueCover(residue_struct *);
void            ResidueEvaporation(residue_struct *, soil_struct *,
    const comm_struct *, double, double);
void            NitrogenTransformation(int, int, soil_struct *,
    const comm_struct *, const residue_struct *, const weather_struct *,
    const soilc_struct *);
void            Nitrification(double *, double *, soil_struct *,
    const soilc_struct *);
void            Denitrification(double *, double *, soil_struct *,
    const soilc_struct *);
void            Volatilization(int, int, double *, soil_struct *,
    const comm_struct *, const residue_struct *, const weather_struct *);
double          N2OFractionNitrification(double);
double          pHFunction(double);
double          VolatilizationDepthFunction(double);
double          AirMolarDensity(double, double);
double          BoundaryLayerConductance(double, double, double, double);
void            ResidueWetting(residue_struct *, double *);
double          FindIrrigationVolume(int, double, const soil_struct *);
void            SoluteTransport(elem_struct *, river_struct *, double);
void            Adsorption(const double *sldpth, const double *,
    const double *, int, double, solute_struct *);
double          LinearEquilibriumConcentration(double, double, double,
    double, double);
double          LinearEquilibriumSoluteMass(double, double, double, double,
    double);
void            Elem2ElemSolTrnsp(const elem_struct *, const elem_struct *,
    double *, const double *, double, double *, double *);
void            Elem2RiverSolTrnsp(const elem_struct *, const river_struct *,
    double, double *, const double *, double, double, double *, double *);
void            River2RiverSolTrnsp(river_struct *, const river_struct *,
    double *, double, double, double, double *, double *);
void            InitCropSV(crop_struct *);
void            WriteCyclesIC(char *, elem_struct *, river_struct *);
#endif

#if defined(_BGC_)
void            BackgroundLitterfall(const epconst_struct *, epvar_struct *,
    const cstate_struct *, cflux_struct *, nflux_struct *);
void            BgcSpinup(pihm_struct, N_Vector, void *);
void            CanopyCond(const epconst_struct *, epvar_struct *,
    const eflux_struct *, const pstate_struct *, const soil_struct *,
    const daily_struct *);
int             CheckBgcSteadyState(const elem_struct *, double, int, int, int);
void            CheckCarbonBalance(const cstate_struct *, double *);
void            CheckNitrogenBalance(const nstate_struct *, double *);
void            CSummary(const cflux_struct *, const cstate_struct *,
    summary_struct *);
void            DailyAllocation(cflux_struct *, const cstate_struct *,
    nflux_struct *, const nstate_struct *, const epconst_struct *,
    epvar_struct *, ntemp_struct *);
void            DailyBgc(pihm_struct, int);
void            DailyCarbonStateUpdate(cflux_struct *, cstate_struct *, int,
    int, int);
void            DailyNitrogenStateUpdate(nflux_struct *, nstate_struct *,
    solute_struct *, int, int, int);
void            Decomp(double, const epconst_struct *, epvar_struct *,
    const cstate_struct *, cflux_struct *, const nstate_struct *,
    nflux_struct *, ntemp_struct *);
void            EvergreenPhenology(const epconst_struct *, epvar_struct *,
    const cstate_struct *);
void            FirstDay(elem_struct *, river_struct *, const cninit_struct *);
void            FRootLitFall(const epconst_struct *, double, cflux_struct *,
    nflux_struct *);
double          GetCO2(tsdata_struct *, int);
double          GetNdep(tsdata_struct *, int);
void            GrowthResp(const epconst_struct *, cflux_struct *);
void            InitBgc(elem_struct *, const epctbl_struct *);
void            InitBgcVar(elem_struct *, river_struct *, N_Vector);
void            LeafLitFall(const epconst_struct *, double, cflux_struct *,
    nflux_struct *);
void            LivewoodTurnover(const epconst_struct *, epvar_struct *,
    const cstate_struct *, cflux_struct *, const nstate_struct *,
    nflux_struct *);
void            MaintResp(const epconst_struct *, epvar_struct *,
    const cstate_struct *, cflux_struct *, const nstate_struct *,
    const daily_struct *);
void            MakeZeroFluxStruct(cflux_struct *, nflux_struct *);
void            Mortality(const epconst_struct *, cstate_struct *,
    cflux_struct *, nstate_struct *, nflux_struct *);
# if defined(_LEACHING_)
void            NLeaching(elem_struct *);
# elif defined(_LUMPED_)
void            NLeachingLumped(elem_struct *, river_struct *);
# else
void            NTransport(elem_struct *, river_struct *);
# endif
void            OffsetLitterfall(const epconst_struct *, epvar_struct *,
    const cstate_struct *, cflux_struct *, nflux_struct *);
void            OnsetGrowth(const epconst_struct *, const epvar_struct *,
    const cstate_struct *, cflux_struct *, const nstate_struct *,
    nflux_struct *);
void            Phenology(const epconst_struct *, epvar_struct *,
    const cstate_struct *, cflux_struct *, const nstate_struct *,
    nflux_struct *, const daily_struct *);
void            Photosynthesis(psn_struct *);
void            PrecisionControl(cstate_struct *cs, nstate_struct *ns);
void            RadTrans(const cstate_struct *, eflux_struct *,
    pstate_struct *, const epconst_struct *, epvar_struct *,
    const daily_struct *);
void            ReadAnnFile(tsdata_struct *, const char *);
void            ReadBgc(const char *, ctrl_struct *, co2control_struct *,
    ndepcontrol_struct *, cninit_struct *, char *, char *);
void            ReadBgcIc(const char *, elem_struct *, river_struct *);
void            ReadEpc(epctbl_struct *);
void            ResetSpinupStat(elem_struct *);
void            RestartInput(cstate_struct *, nstate_struct *,
    epvar_struct *, const bgcic_struct *);
void            RestartOutput(const cstate_struct *, const nstate_struct *,
    const epvar_struct *, bgcic_struct *);
void            SeasonDecidPhenology(const epconst_struct *, epvar_struct *,
    const daily_struct *);
void            SetAbsTol(double, double, N_Vector);
void            SoilPsi(const soil_struct *, double, double *);
void            TotalPhotosynthesis(const epconst_struct *, epvar_struct *,
    const pstate_struct *, cflux_struct *, psn_struct *, psn_struct *,
    const daily_struct *);
void            WriteBgcIc(const char *, elem_struct *, river_struct *);
void            ZeroSrcSnk(cstate_struct *, nstate_struct *, summary_struct *,
    solute_struct *);
#endif

#endif
