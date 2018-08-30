#include "pihm.h"
#include "optparse.h"

void ParseCmdLineParam(int argc, char *argv[], char *outputdir)
{
    int             option;
    struct optparse options;
    struct optparse_long longopts[] = {
        {"output", 'o', OPTPARSE_REQUIRED},
        {"elevation_correction", 'c', OPTPARSE_NONE},
        {"debug", 'd', OPTPARSE_NONE},
        {"tecplot", 't', OPTPARSE_NONE},
        {"verbose", 'v', OPTPARSE_NONE},
        {"print_version", 'V', OPTPARSE_NONE},
        {0, 0, 0}
    };

    optparse_init(&options, argv);

    while ((option = optparse_long(&options, longopts, NULL)) != -1)
    {
        switch (option)
        {
            case 'o':
                /* Specify output directory */
                sprintf(outputdir, "output/%s/", options.optarg);
                break;
            case 'c':
                /* Surface elevation correction mode */
                corr_mode = 1;
                printf("\tSurface elevation correction mode turned on.\n");
                break;
            case 'd':
                /* Debug mode */
                debug_mode = 1;
                printf("\tDebug mode turned on.\n");
                break;
            case 't':
                /* Tecplot output */
                tecplot = 1;
                printf("\tTecplot output turned on.\n");
                break;
            case 'v':
                /* Verbose mode */
                verbose_mode = 1;
                printf("\tVerbose mode turned on.\n");
                break;
            case 'V':
                /* Print version number */
                printf("\nMM-PIHM Version %s.\n", VERSION);
                PIHMexit(EXIT_SUCCESS);
                break;
            case '?':
                printf("Option not recognisable %s\n", options.errmsg);
                break;
            default:
                break;
        }

        fflush(stdout);
    }

    if (options.optind >= argc)
    {
        fprintf(stderr, "Error:You must specify the name of project!\n");
        fprintf(stderr,
            "Usage: ./pihm [-o output_dir] [-c] [-d] [-v] [-V]"
            " <project name>\n");
        fprintf(stderr, "\t-o Specify output directory\n");
        fprintf(stderr, "\t-c Correct surface elevation\n");
        fprintf(stderr, "\t-d Debug mode\n");
        fprintf(stderr, "\t-v Verbose mode\n");
        fprintf(stderr, "\t-V Version number\n");
        PIHMexit(EXIT_FAILURE);
    }
    else
    {
        /* Parse remaining arguments */
        strcpy(project, optparse_arg(&options));
    }
}

void CreateOutputDir(char *outputdir)
{
    time_t          rawtime;
    struct tm      *timestamp;
    char            str[11];
    char            icdir[MAXSTRING];

    if (0 == (PIHMmkdir("output")))
    {
        PIHMprintf(VL_NORMAL, " Output directory was created.\n\n");
    }

    if (outputdir[0] == '\0')
    {
        /* Create default output directory name based on project and time */
        time(&rawtime);
        timestamp = localtime(&rawtime);
        strftime(str, 11, "%y%m%d%H%M", timestamp);
        //sprintf(outputdir, "output/%s.%s/", project, str);
        sprintf(outputdir, "output/%s/", project);
    }

    if (PIHMmkdir(outputdir) != 0)
    {
        if (errno != EEXIST)
        {
            PIHMprintf(VL_ERROR,
                "Error creating output directory %s\n", outputdir);
            PIHMexit(EXIT_FAILURE);
        }
        else
        {
            PIHMprintf(VL_NORMAL,
                "Output directory %s already exists. Overwriting.\n",
                outputdir);
        }
    }
    else
    {
        PIHMprintf(VL_NORMAL, "Output directory %s was created.\n", outputdir);
    }

    sprintf(icdir, "%srestart/", outputdir);
    if (PIHMmkdir(icdir) != 0 && errno != EEXIST)
    {
        PIHMprintf(VL_ERROR,
            "Error creating restart directory %s\n", outputdir);
        PIHMexit(EXIT_FAILURE);
    }
}

void BackupInput(const char *outputdir)
{
    char            system_cmd[MAXSTRING];
    char            source_file[MAXSTRING];

#if OBSOLETE
    strcpy(tempname, project);
    if (strstr(tempname, ".") != 0)
    {
        token = strtok(tempname, ".");
        strcpy(project, token);
    }
    else
    {
        strcpy(project, project);
    }
#endif

    /* Save input files into output directory */
    sprintf(source_file, "input/%s/%s.para", project, project);
    if (PIHMaccess(source_file, F_OK) != -1)
    {
        sprintf(system_cmd, "cp %s ./%s/%s.para.bak", source_file, outputdir,
            project);
        system(system_cmd);
    }
    sprintf(source_file, "input/%s/%s.calib", project, project);
    if (PIHMaccess(source_file, F_OK) != -1)
    {
        sprintf(system_cmd, "cp %s ./%s/%s.calib.bak", source_file,
            outputdir, project);
        system(system_cmd);
    }
    sprintf(source_file, "input/%s/%s.init", project, project);
    if (PIHMaccess(source_file, F_OK) != -1)
    {

        sprintf(system_cmd, "cp %s ./%s/%s.init.bak", source_file, outputdir,
            project);
        system(system_cmd);
    }
}

int CheckCVodeFlag(int cv_flag)
{
    if (cv_flag < 0)
    {
        PIHMprintf(VL_ERROR, "CVODE error %d\n", cv_flag);
        return 0;
    }
    else
    {
        return 1;
    }
}

void _PIHMexit(const char *fn, int lineno, const char *func, int error)
{
    PIHMprintf(VL_ERROR, "\n");
    PIHMprintf(VL_ERROR, "Exiting from %s", func);
    if (debug_mode)
    {
        PIHMprintf(VL_ERROR, " (%s, Line %d)", fn, lineno);
    }
    PIHMprintf(VL_ERROR, "...\n\n");

    exit(error);
}
