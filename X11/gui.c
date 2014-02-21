/* Standard C header files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>

/* Motif header files */
#include <Xm/MainW.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/ToggleB.h>
#include <Xm/BulletinB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/Notebook.h>
#include <Xm/RowColumn.h>
#include <Xm/Frame.h>
#include <Xm/ComboBox.h>
#include <Xm/FileSB.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/CascadeB.h>
#include <Xm/Protocols.h>

/* Local header files */
#include "cluster.h"   /* The C clustering library */
#include "data.h"      /* Includes data handling and file reading/writing */
#include "gui.h"       /* Declaration of the guimain function */

#define CMD_FILE_OPEN 0
#define CMD_FILE_SAVE 1
#define CMD_FILE_QUIT 2
#define CMD_HELP_HTMLHELP 0
#define CMD_HELP_MANUAL 1
#define CMD_HELP_DOWNLOAD 2
#define CMD_HELP_FILEFORMAT 3
#define CMD_HELP_ABOUT 4
#define ID_FILEMANAGER_INIT 101
#define ID_FILEMANAGER_SET_FILEMEMO 102
#define ID_FILEMANAGER_SET_JOBNAME 103
#define ID_FILEMANAGER_UPDATE_ROWS_COLUMNS 104
#define ID_FILTER_INIT 301
#define ID_FILTER_APPLY 311
#define ID_FILTER_ACCEPT 313
#define ID_FILTER_RESET 314
#define ID_FILTER_FREE 315
#define ID_ADJUST_INIT 401
#define ID_ADJUST_EXECUTE 402
#define ID_HIERARCHICAL_INIT 501
#define ID_HIERARCHICAL_CENTROID 513
#define ID_HIERARCHICAL_SINGLE 514
#define ID_HIERARCHICAL_COMPLETE 515
#define ID_HIERARCHICAL_AVERAGE 516
#define ID_KMEANS_INIT 601
#define ID_KMEANS_EXECUTE 602
#define ID_SOM_INIT 701
#define ID_SOM_EXECUTE 702
#define ID_SOM_UPDATE 703
#define ID_PCA_INIT 801
#define ID_PCA_EXECUTE 802

/*============================================================================*/
/* GUI utilities                                                              */
/*============================================================================*/

static void
CreateMetricComboBox(Widget parent, int x, int y, char* name, char initial)
{   XmString xms;
    Widget label, combo;
    Arg args[5];
    int n = 0;
    XmStringTable metrics = (XmStringTable)XtMalloc(8*sizeof(XmString*));
    metrics[0] = XmStringCreateSimple("Correlation (uncentered)");
    metrics[1] = XmStringCreateSimple("Correlation (centered)");
    metrics[2] = XmStringCreateSimple("Absolute Correlation (uncentered)");
    metrics[3] = XmStringCreateSimple("Absolute Correlation (centered)");
    metrics[4] = XmStringCreateSimple("Spearman Rank Correlation");
    metrics[5] = XmStringCreateSimple("Kendall's tau");
    metrics[6] = XmStringCreateSimple("Euclidean distance");
    metrics[7] = XmStringCreateSimple("City-block distance");
    XtSetArg(args[n], XmNx, x+65); n++;
    XtSetArg(args[n], XmNy, y); n++;
    xms = XmStringCreateSimple("Similarity Metric");
    XtSetArg(args[n], XmNlabelString, xms); n++;
    label = XmCreateLabel(parent, "", args, n);
    XtManageChild(label);
    XmStringFree(xms);
    n = 0;
    XtSetArg(args[n], XmNx, x); n++;
    XtSetArg(args[n], XmNy, y+20); n++;
    XtSetArg(args[n], XmNitems, metrics); n++;
    XtSetArg(args[n], XmNitemCount, 8); n++;
    switch (initial)
    {   case 'u': XtSetArg(args[n], XmNselectedPosition, 0); n++; break;
        case 'c': XtSetArg(args[n], XmNselectedPosition, 1); n++; break;
        case 'a': XtSetArg(args[n], XmNselectedPosition, 2); n++; break;
        case 'x': XtSetArg(args[n], XmNselectedPosition, 3); n++; break;
        case 's': XtSetArg(args[n], XmNselectedPosition, 4); n++; break;
        case 'k': XtSetArg(args[n], XmNselectedPosition, 5); n++; break;
        case 'e': XtSetArg(args[n], XmNselectedPosition, 6); n++; break;
        case 'b': XtSetArg(args[n], XmNselectedPosition, 7); n++; break;
        default : XtSetArg(args[n], XmNselectedPosition, 0); n++; break;
    }
    combo = XmCreateDropDownList(parent, name, args, n);
    XtManageChild(combo);
    for (n = 0; n < 8; n++) XmStringFree(metrics[n]);
    XtFree((char*)metrics);
}

static char GetMetric(Widget w)
{ int n = 0;
  Arg args[1];
  int index;
  XtSetArg(args[n], XmNselectedPosition, &index); n++;
  XtGetValues(w, args, n);
  switch (index)
  { case 0: return 'u'; break; /* Uncentered correlation */
    case 1: return 'c'; break; /* Centered correlation */
    case 2: return 'x'; break; /* Absolute uncentered correlation */
    case 3: return 'a'; break; /* Absolute centered correlation */
    case 4: return 's'; break; /* Spearman rank correlation */
    case 5: return 'k'; break; /* Kendall's tau */
    case 6: return 'e'; break; /* Euclidean distance */
    case 7: return 'b'; break; /* City-block distance */
    /* The code will never get here. */
    default: return 'e'; /* Euclidean distance is default. */
  }
}

static int GetWidgetItemInt(Widget w, const char item[])
{   int result;
    char* text;
    Widget textfield = XtNameToWidget(w, item);
    if (textfield==0) return 0;
    text = XmTextGetString(textfield);
    result = strtol(text, NULL, 0); /* returns 0 if failed */
    XtFree(text);
    return result;
}

/*============================================================================*/
/* Status bar                                                                 */
/*============================================================================*/

static Widget Statusbar(Widget w, char* message)
{
    static Widget statusbar = NULL;
    if (!statusbar)
    {   int n = 0;
        Arg args[2];
        XtSetArg(args[n], XmNeditable, False); n++;
        XtSetArg(args[n], XmNmaxLength, 200); n++;
        statusbar = XmCreateTextField(w, "message", args, n);
        XtManageChild(statusbar);
        return statusbar;
    }
    else
    {   XmTextSetString(statusbar, message);
        XmUpdateDisplay(statusbar);
        return NULL;
    }
}

/*============================================================================*/
/* Error messages                                                             */
/*============================================================================*/

static void ShowError(Widget parent, const char* message, const char* title)
{   Widget dialog;
    Arg args[2];
    XmString message_string = XmStringCreateSimple((char*)message);
    /* Note: error is a const char*. XmStringCreateSimple wants a char* (without
     * the const), although it does not modify the string. So cast it to char*.
     */
    XmString title_string = title ? XmStringCreateSimple((char*)title)
                                  : XmStringCreateSimple("Error");
    int n = 0;
    XtSetArg(args[n], XmNmessageString, message_string); n++;
    XtSetArg(args[n], XmNdialogTitle, title_string); n++;
    dialog = XmCreateMessageDialog(parent, "", args, n);
    XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
    XtManageChild(dialog);
    XmStringFree(message_string);
    XmStringFree(title_string);
}

/*============================================================================*/
/* File manager functions                                                     */
/*============================================================================*/

static char* GetBaseName(Widget work)
{   Widget widget;
    char* jobname;
    char* directory;
    char* index;
    char* fullpath;
    int n;
    widget = XtNameToWidget(work,"Jobname");
    jobname = XmTextGetString(widget);
    widget = XtNameToWidget(work,"FileMemo");
    directory = XmTextGetString(widget);
    index = strrchr(directory,'/');
    *(index+1) = '\0';
    n = strlen(directory) + strlen(jobname) + 1;
    fullpath = malloc(n*sizeof(char));
    if (fullpath)
    {   strcpy(fullpath, directory);
        strcat(fullpath, jobname);
    }
    XtFree(directory);
    XtFree(jobname);
    return fullpath;
}

/*============================================================================*/
/* Callback functions --- Tab pages                                           */
/*============================================================================*/

static void SOM(Widget w, XtPointer client_data, XtPointer call_data)
{   
    static Widget page = NULL;
    int* which = (int*) client_data;
    switch (*which)
    {   case ID_SOM_INIT:
        {   static int command = ID_SOM_EXECUTE;
            Arg args[5];
            int n;
            Widget widget, frame;
            XmString xms;

            page = w;
            n = 0;
            XtSetArg(args[n], XmNx, 180); n++;
            widget = XmCreateLabel(page, "Calculate a Self-Organizing Map", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 50); n++;
            widget = XmCreateToggleButton(page, "Organize genes", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 80); n++;
            XtSetArg(args[n], XmNwidth, 60); n++;
            XtSetArg(args[n], XmNvalue, "4"); n++;
            widget = XmCreateText(page, "SOMGeneXDim", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 85); n++;
            XtSetArg(args[n], XmNy, 86); n++;
            widget = XmCreateLabel(page, "XDim", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 120); n++;
            XtSetArg(args[n], XmNwidth, 60); n++;
            XtSetArg(args[n], XmNvalue, "4"); n++;
            widget = XmCreateText(page, "SOMGeneYDim", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 85); n++;
            XtSetArg(args[n], XmNy, 126); n++;
            widget = XmCreateLabel(page, "YDim", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 160); n++;
            XtSetArg(args[n], XmNwidth, 100); n++;
            XtSetArg(args[n], XmNvalue, "100000"); n++;
            widget = XmCreateText(page, "SOMGeneIters", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 130); n++;
            XtSetArg(args[n], XmNy, 166); n++;
            widget = XmCreateLabel(page, "Number of iterations", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 200); n++;
            XtSetArg(args[n], XmNwidth, 80); n++;
            XtSetArg(args[n], XmNvalue, "0.02"); n++;
            widget = XmCreateText(page, "SOMGeneTau", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 110); n++;
            XtSetArg(args[n], XmNy, 206); n++;
            widget = XmCreateLabel(page, "Initial tau", args, n);
            XtManageChild(widget);
            CreateMetricComboBox(page, 20, 240, "GeneMetric", 'e');
            n = 0;
            XtSetArg(args[n], XmNy, 30); n++;
            XtSetArg(args[n], XmNwidth, 255); n++;
            XtSetArg(args[n], XmNheight, 275); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "Frame", args, n);
            XtManageChild(frame);
            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            xms = XmStringCreateSimple("Genes");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(frame, NULL, args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNx, 290); n++;
            XtSetArg(args[n], XmNy, 50); n++;
            widget = XmCreateToggleButton(page, "Organize arrays", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 290); n++;
            XtSetArg(args[n], XmNy, 80); n++;
            XtSetArg(args[n], XmNwidth, 60); n++;
            XtSetArg(args[n], XmNvalue, "4"); n++;
            widget = XmCreateText(page, "SOMArrayXDim", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 355); n++;
            XtSetArg(args[n], XmNy, 86); n++;
            widget = XmCreateLabel(page, "XDim", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 290); n++;
            XtSetArg(args[n], XmNy, 120); n++;
            XtSetArg(args[n], XmNwidth, 60); n++;
            XtSetArg(args[n], XmNvalue, "4"); n++;
            widget = XmCreateText(page, "SOMArrayYDim", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 355); n++;
            XtSetArg(args[n], XmNy, 126); n++;
            widget = XmCreateLabel(page, "YDim", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 290); n++;
            XtSetArg(args[n], XmNy, 160); n++;
            XtSetArg(args[n], XmNwidth, 100); n++;
            XtSetArg(args[n], XmNvalue, "20000"); n++;
            widget = XmCreateText(page, "SOMArrayIters", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 400); n++;
            XtSetArg(args[n], XmNy, 166); n++;
            widget = XmCreateLabel(page, "Number of iterations", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 290); n++;
            XtSetArg(args[n], XmNy, 200); n++;
            XtSetArg(args[n], XmNwidth, 80); n++;
            XtSetArg(args[n], XmNvalue, "0.02"); n++;
            widget = XmCreateText(page, "SOMArrayTau", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 380); n++;
            XtSetArg(args[n], XmNy, 206); n++;
            widget = XmCreateLabel(page, "Initial tau", args, n);
            XtManageChild(widget);
            CreateMetricComboBox(page, 290, 240, "ArrayMetric", 'e');
            n = 0;
            XtSetArg(args[n], XmNx, 280); n++;
            XtSetArg(args[n], XmNy, 30); n++;
            XtSetArg(args[n], XmNwidth, 255); n++;
            XtSetArg(args[n], XmNheight, 275); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "Frame", args, n);
            XtManageChild(frame);
            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            xms = XmStringCreateSimple("Arrays");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(frame, NULL, args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNx, 240); n++;
            XtSetArg(args[n], XmNy, 315); n++;
            widget = XmCreatePushButton(page, "Make SOM", args, n);
            XtManageChild(widget);
            XtAddCallback(widget,
                          XmNactivateCallback,
                          SOM,
                          (XtPointer)&command);
            break;
        }
        case ID_SOM_EXECUTE:
        {   const int Rows = GetRows();
            const int Columns = GetColumns();

            Widget button;

            Boolean ClusterGenes;
            Boolean ClusterArrays;
            int GeneXDim = GetWidgetItemInt(page, "SOMGeneXDim");
            int GeneYDim = GetWidgetItemInt(page, "SOMGeneYDim");
            int ArrayXDim = GetWidgetItemInt(page, "SOMArrayXDim");
            int ArrayYDim = GetWidgetItemInt(page, "SOMArrayYDim");
            int GeneIters = 0;
            int ArrayIters = 0;
            double GeneTau = 0.0;
            double ArrayTau = 0.0;
            char GeneMetric = 'e';
            char ArrayMetric = 'e';

            int n;
            char* path;
            char* filetag;
            FILE* GeneFile = NULL;
            FILE* ArrayFile = NULL;
            FILE* DataFile = NULL;

            int ok;

            Widget notebook = XtParent(page);
            Widget work = XtParent(notebook);

            if (Rows==0 || Columns==0)
            {   Statusbar(NULL, "No data available");
                return;
            }

            path = GetBaseName(work);
            if (!path)
            {   Statusbar(NULL, "Memory allocation failure");
                return;
            }

            button = XtNameToWidget(page,"Organize genes");
            ClusterGenes = XmToggleButtonGetState(button);
            button = XtNameToWidget(page,"Organize arrays");
            ClusterArrays = XmToggleButtonGetState(button);

            n = strlen(path) + strlen("_SOM") + strlen(".ext") + 1;
            if (ClusterGenes)
            {   int dummy;
                dummy = GeneXDim;
                do n++; while (dummy/=10);
                dummy = GeneYDim;
                do n++; while (dummy/=10);
                n+=strlen("_G");
                n++; /* For the '-' */
            }
            if (ClusterArrays)
            {   int dummy;
                dummy = ArrayXDim;
                do n++; while (dummy/=10);
                dummy = ArrayYDim;
                do n++; while (dummy/=10);
                n+=strlen("_A");
                n++; /* For the '-' */
            }
    
            path = realloc(path, n*sizeof(char));
            strcat(path, "_SOM");
            filetag = strchr(path, '\0');
            if (ClusterGenes)
                filetag += sprintf(filetag, "_G%d-%d", GeneXDim, GeneYDim);
            if (ClusterArrays)
                filetag += sprintf(filetag, "_A%d-%d", ArrayXDim, ArrayYDim);

            sprintf(filetag, ".txt");
            DataFile = fopen(path, "wt");
            if (!DataFile) 
            {   free(path);
                Statusbar(NULL, "Error: Unable to open the output file");
                return;
            }
 
            if (ClusterGenes)
            {   double value;
                char* error;
                Widget widget = XtNameToWidget(page,"SOMGeneTau");
                char* text = XmTextGetString(widget);
                value = strtod(text, &error);
                GeneTau = error ? value : 0;
                XtFree(text);
                widget = XtNameToWidget(page,"GeneMetric");
                GeneMetric = GetMetric(widget);
                GeneIters = GetWidgetItemInt(page, "SOMGeneIters");
    
                if ((GeneIters==0)||(GeneTau==0)||(GeneXDim==0)||(GeneYDim==0))
                {   Statusbar(NULL, "Error starting SOM: Check options");
                    fclose(DataFile);
                    free(path);
                    return;
                }
                sprintf(filetag, ".gnf");
                GeneFile = fopen(path, "wt");
                if (!GeneFile) 
                {   Statusbar(NULL, "Error: Unable to open the output file");
                    fclose(DataFile);
                    free(path);
                    return;
                }
            }

            if (ClusterArrays)
            {   double value;
                char* error;
                Widget widget = XtNameToWidget(page,"SOMArrayTau");
                char* text = XmTextGetString(widget);
                value = strtod(text, &error);
                ArrayTau = error ? value : 0;
                XtFree(text);
                widget = XtNameToWidget(page,"ArrayMetric");
                ArrayMetric = GetMetric(widget);
                ArrayIters = GetWidgetItemInt(page, "SOMArrayIters");

                if ((ArrayIters==0)||(ArrayTau==0)||(ArrayXDim==0)||(ArrayYDim==0))
                {   Statusbar(NULL, "Error starting SOM: Check options");
                    if (GeneFile) fclose(GeneFile);
                    fclose(DataFile);
                    free(path);
                    return;
                }

                sprintf(filetag, ".anf");
                ArrayFile = fopen(path, "wt");
                if (!ArrayFile) 
                {   Statusbar(NULL, "Error: Unable to open the output file");
                    if (GeneFile) fclose(GeneFile);
                    fclose(DataFile);
                    free(path);
                    return;
                }
            }

            free(path);

            Statusbar(NULL, "Calculating Self-Organizing Map");

            ok = PerformSOM(GeneFile, GeneXDim, GeneYDim, GeneIters, GeneTau,
                           GeneMetric, ArrayFile, ArrayXDim, ArrayYDim,
                           ArrayIters, ArrayTau, ArrayMetric);
            if (GeneFile) fclose(GeneFile);
            if (ArrayFile) fclose(ArrayFile);
            if (!ok)
            {   ShowError(w, "Memory allocation error", "Insufficient memory");
                Statusbar(NULL, "Memory allocation error");
                break;
            }
            ok = Save(DataFile, 0, 0);
            fclose(DataFile);
            if (!ok)
            { ShowError(w, "Error saving file", "Insufficient memory");
              Statusbar(NULL, "Error saving to file");
              break;
            }
            Statusbar(NULL, "Done making SOM");
            break;
        }
        case ID_SOM_UPDATE:
        {   const int Rows = GetRows();
            const int Columns = GetColumns();
            int n;
            int dim;
            Arg args[1];
            char buffer[32];
            Widget widget;

            /* Update SOM defaults to reflect new number of rows */
            dim = 1 + (int)sqrt(sqrt(Rows)); /* pow causes a crash on AIX */
            n = 0;
            sprintf(buffer,"%d",dim);
            XtSetArg(args[n], XmNvalue, buffer); n++;
            widget = XtNameToWidget(page,"SOMGeneXDim");
            XtSetValues(widget, args, n);
            widget = XtNameToWidget(page,"SOMGeneYDim");
            XtSetValues(widget, args, n);
            dim = 1 + (int)sqrt(sqrt(Columns)); /* pow causes a crash on AIX */
            n = 0;
            sprintf(buffer,"%d",dim);
            XtSetArg(args[n], XmNvalue, buffer); n++;
            widget = XtNameToWidget(page,"SOMArrayXDim");
            XtSetValues(widget, args, n);
            widget = XtNameToWidget(page,"SOMArrayYDim");
            XtSetValues(widget, args, n);
            break;
        }
    }
}

static void FileManager(Widget w, XtPointer client_data, XtPointer call_data)
{   static Widget work = NULL;
    int* which = (int*) client_data;
    switch (*which)
    {   case ID_FILEMANAGER_INIT:
        {   Widget widget;
            Arg args[5];
            int n = 0;
            XmString xmsempty = XmStringCreateSimple("");
            XmString xmsmax = XmStringCreateSimple("xxxxxxxxx");
            /* to calculate the size */

            work = w;

            n = 0;
            XtSetArg(args[n], XmNx, 10); n++;
            XtSetArg(args[n], XmNy, 10); n++;
            widget = XmCreateLabel(work, "File loaded", args, n);
            XtManageChild(widget);

            n = 0;
            XtSetArg(args[n], XmNx, 10); n++;
            XtSetArg(args[n], XmNy, 100); n++;
            widget = XmCreateLabel(work, "Job name", args, n);
            XtManageChild(widget);

            n = 0;
            XtSetArg(args[n], XmNx, 10); n++;
            XtSetArg(args[n], XmNy, 140); n++;
            widget = XmCreateLabel(work, "Data set has", args, n);
            XtManageChild(widget);

            n = 0;
            XtSetArg(args[n], XmNx, 180); n++;
            XtSetArg(args[n], XmNy, 130); n++;
            XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
            XtSetArg(args[n], XmNrecomputeSize, False); n++;
            XtSetArg(args[n], XmNlabelString, xmsmax); n++;
            widget = XmCreateLabel(work, "rows", args, n);

            n = 0;
            XtSetArg(args[n], XmNlabelString, xmsempty); n++;
            XtSetValues(widget, args, n);
            XtManageChild(widget);

            n = 0;
            XtSetArg(args[n], XmNx, 240); n++;
            XtSetArg(args[n], XmNy, 130); n++;
            widget = XmCreateLabel(work, "Rows", args, n);
            XtManageChild(widget);

            n = 0;
            XtSetArg(args[n], XmNx, 180); n++;
            XtSetArg(args[n], XmNy, 150); n++;
            XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
            XtSetArg(args[n], XmNrecomputeSize, False); n++;
            XtSetArg(args[n], XmNlabelString, xmsmax); n++;
            widget = XmCreateLabel(work, "columns", args, n);

            n = 0;
            XtSetArg(args[n], XmNlabelString, xmsempty); n++;
            XtSetValues(widget, args, n);
            XtManageChild(widget);

            n = 0;
            XtSetArg(args[n], XmNx, 240); n++;
            XtSetArg(args[n], XmNy, 150); n++;
            widget = XmCreateLabel(work, "Columns", args, n);
            XtManageChild(widget);

            n = 0;
            XtSetArg(args[n], XmNx, 160); n++;
            XtSetArg(args[n], XmNy, 10); n++;
            XtSetArg(args[n], XmNwidth, 400); n++;
            XtSetArg(args[n], XmNheight, 80); n++;
            XtSetArg(args[n], XmNeditable, False); n++;
            widget = XmCreateText(work, "FileMemo", args, n);
            XtManageChild(widget);

            n = 0;
            XtSetArg(args[n], XmNx, 160); n++;
            XtSetArg(args[n], XmNy, 95); n++;
            XtSetArg(args[n], XmNwidth, 400); n++;
            XtSetArg(args[n], XmNeditable, True); n++;
            widget = XmCreateText(work, "Jobname", args, n);
            XtManageChild(widget);

            XmStringFree(xmsmax);
            XmStringFree(xmsempty);
            break;
        }
        case ID_FILEMANAGER_UPDATE_ROWS_COLUMNS:
        {   int n;
            Arg args[1];
            Widget widget;
            char buffer[32];
            XmString xms;
            const int rows = GetRows();
            const int columns = GetColumns();
            widget = XtNameToWidget(work, "rows");
            sprintf(buffer,"%d", rows);
            xms = XmStringCreateSimple(buffer);
            n = 0;
            XtSetArg(args[n], XmNlabelString, xms); n++;
            XtSetValues(widget, args, n);
            XmStringFree(xms);
            n = 0;
            widget = XtNameToWidget(work, "columns");
            sprintf(buffer,"%d", columns);
            xms = XmStringCreateSimple(buffer);
            XtSetArg(args[n], XmNlabelString, xms); n++;
            XtSetValues(widget, args, n);
            XmStringFree(xms);
            break;
        }
        case ID_FILEMANAGER_SET_JOBNAME:
        {   int n = 0;
            Arg args[1];
            char* jobname = (char*)call_data; 
            Widget widget = XtNameToWidget(work, "Jobname");
            XtSetArg(args[n], XmNvalue, jobname); n++;
            XtSetValues(widget, args, n);
            break;
        }
        case ID_FILEMANAGER_SET_FILEMEMO:
        {   int n = 0;
            Arg args[1];
            char* filememo = (char*)call_data; 
            Widget widget = XtNameToWidget(work, "FileMemo");
            XtSetArg(args[n], XmNvalue, filememo); n++;
            XtSetValues(widget, args, n);
            break;
        }
    }
}

static void Filter(Widget w, XtPointer client_data, XtPointer call_data)
{   
    int* which = (int*) client_data;
    static int* use = NULL;
    static int useRows = 0;
    static Widget page = NULL;
    switch (*which)
    {   case ID_FILTER_INIT:
        {   Arg args[6];
            int n;
            Widget widget, frame;
            XmString xms;
            static int apply_command = ID_FILTER_APPLY;
            static int accept_command = ID_FILTER_ACCEPT;

            page = w;
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 33); n++;
            widget = XmCreateToggleButton(page, "% Present >=", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 160); n++;
            XtSetArg(args[n], XmNy, 30); n++;
            XtSetArg(args[n], XmNwidth, 80); n++;
            XtSetArg(args[n], XmNvalue, "80"); n++;
            widget = XmCreateText(page, "FilterPercent", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 73); n++;
            widget = XmCreateToggleButton(page, "SD (Gene Vector)", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 160); n++;
            XtSetArg(args[n], XmNy, 70); n++;
            XtSetArg(args[n], XmNwidth, 80); n++;
            XtSetArg(args[n], XmNvalue, "2.0"); n++;
            widget = XmCreateText(page, "FilterStd", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 113); n++;
            widget = XmCreateToggleButton(page, "At least", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 97); n++;
            XtSetArg(args[n], XmNy, 110); n++;
            XtSetArg(args[n], XmNwidth, 50); n++;
            XtSetArg(args[n], XmNvalue, "1"); n++;
            widget = XmCreateText(page, "FilterNumber", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 150); n++;
            XtSetArg(args[n], XmNy, 117); n++;
            widget = XmCreateLabel(page, "observations with abs(Val) >=", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 340); n++;
            XtSetArg(args[n], XmNy, 110); n++;
            XtSetArg(args[n], XmNwidth, 80); n++;
            XtSetArg(args[n], XmNvalue, "2.0"); n++;
            widget = XmCreateText(page, "FilterObservationValue", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 153); n++;
            widget = XmCreateToggleButton(page, "MaxVal - MinVal >=", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 160); n++;
            XtSetArg(args[n], XmNy, 150); n++;
            XtSetArg(args[n], XmNwidth, 80); n++;
            XtSetArg(args[n], XmNvalue, "2.0"); n++;
            widget = XmCreateText(page, "FilterMaxMin", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 240); n++;
            XtSetArg(args[n], XmNy, 190); n++;
            widget = XmCreatePushButton(page, "Apply Filter", args, n);
            XtManageChild(widget);
            XtAddCallback(widget, XmNactivateCallback, Filter, (XtPointer)&apply_command);
            n = 0;
            XtSetArg(args[n], XmNx, 237); n++;
            XtSetArg(args[n], XmNy, 245); n++;
            XtSetArg(args[n], XmNsensitive, False); n++;
            widget = XmCreatePushButton(page, "Accept Filter", args, n);
            XtManageChild(widget);
            XtAddCallback(widget, XmNactivateCallback, Filter, (XtPointer)&accept_command);
            n = 0;
            XtSetArg(args[n], XmNx, 80); n++;
            XtSetArg(args[n], XmNy, 220); n++;
            XtSetArg(args[n], XmNwidth, 400); n++;
            XtSetArg(args[n], XmNheight, 20); n++;
            xms = XmStringCreateSimple("");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            XtSetArg(args[n], XmNrecomputeSize, False); n++;
            widget = XmCreateLabel(page, "FilterResult", args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNwidth, 500); n++;
            XtSetArg(args[n], XmNheight, 270); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "Frame", args, n);
            XtManageChild(frame);
            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            xms = XmStringCreateSimple("Filter Genes");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(frame, NULL, args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            break;
        }
        case ID_FILTER_APPLY:
        {   /* Filter data. Apply user selected criteria to flag (for subsequent
             * removal) rows that fail to pass tests. Note that filters are
             * assessed here and applied separately so the user can adjust
             * parameters to get appropriate number of rows passing
             */
            Boolean bStd, bPercent, bAbsVal, bMaxMin;
            Widget label, button, textfield;
            Arg args[1];
            int n = 0;
            const int Rows = GetRows();
            char buffer[128];
            char* text;
            char* errorchar;
            double value;
            double absVal, percent, std;
            int numberAbs;
            double maxmin;
            int Row;
            int command = ID_FILTER_RESET;

            XmString xms = XmStringGenerate("result", XmFONTLIST_DEFAULT_TAG, XmCHARSET_TEXT, NULL);
            label = XtNameToWidget(page,"FilterResult");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            XtSetValues(label, args, n);
            XmStringFree(xms);
            button = XtNameToWidget(page,"Accept Filter");
            XtSetSensitive(button, True);
            button = XtNameToWidget(page,"SD (Gene Vector)");
            bStd = XmToggleButtonGetState(button);
            button = XtNameToWidget(page,"% Present >=");
            bPercent = XmToggleButtonGetState(button);
            button = XtNameToWidget(page,"At least");
            bAbsVal = XmToggleButtonGetState(button);
            button = XtNameToWidget(page,"MaxVal - MinVal >=");
            bMaxMin = XmToggleButtonGetState(button);

            /* Read information from the edit boxes */
            textfield = XtNameToWidget(page,"FilterObservationValue");
            text = XmTextGetString(textfield);
            value = strtod(text, &errorchar);
            absVal = (*errorchar==0) ? value : 0;
            XtFree(text);
            textfield = XtNameToWidget(page,"FilterPercent");
            text = XmTextGetString(textfield);
            value = strtod(text, &errorchar);
            percent = (*errorchar==0) ? value : 0;
            XtFree(text);
            textfield = XtNameToWidget(page,"FilterStd");
            text = XmTextGetString(textfield);
            value = strtod(text, &errorchar);
            std = (*errorchar==0) ? value : 0;
            XtFree(text);
            numberAbs = GetWidgetItemInt(page, "FilterNumber");
            textfield = XtNameToWidget(page,"FilterMaxMin");
            text = XmTextGetString(textfield);
            value = strtod(text, &errorchar);
            maxmin = (*errorchar==0) ? value : 0;
            XtFree(text);

            Filter(page, (XtPointer)&command, NULL);

            /* Store results in boolean use */
            if (use) free(use);
            use = malloc(Rows*sizeof(int));
            if (!use)
            {   Statusbar(NULL, "Memory allocation failure");
                return;
            }
 
            useRows = 0;

            for (Row = 0; Row < Rows; Row++)
            {   sprintf(buffer, "Assessing filters for gene %d", Row);
                Statusbar(NULL, buffer);
                use[Row] = FilterRow(Row, bStd, bPercent, bAbsVal, bMaxMin,
                                     absVal, percent, std, numberAbs, maxmin);
                /* Count how many passed */
                if (use[Row]) useRows++;
            }

            /* Tell user how many rows passed */
            sprintf(buffer, "%d passed out of %d", useRows, Rows);
            xms = XmStringCreateSimple(buffer);
            label = XtNameToWidget(page, "FilterResult");
            n = 0;
            XtSetArg(args[n], XmNlabelString, xms); n++;
            XtSetValues(label, args, n);
            XmStringFree(xms);

            n = 0;
            XtSetArg(args[n], XmNsensitive, True); n++;
            button = XtNameToWidget(page, "Accept Filter");
            XtSetValues(button, args, n);
            Statusbar(NULL, "Done Analyzing Filters");
            break;
        }
        case ID_FILTER_ACCEPT:
        {   /* Accept results of last filtering */
            int update;
            int ok;
            XtSetSensitive(w, False);
            ok = SelectSubset(useRows, use);
            if (!ok)
            {   ShowError(w,
                          "Insufficient memory",
                          "Failed to apply filtering");
                Statusbar(NULL, "Filtering failed");
                return;
            }
            update = ID_FILEMANAGER_UPDATE_ROWS_COLUMNS;
            FileManager(NULL, (XtPointer)&update, NULL);
            update = ID_SOM_UPDATE;
            SOM(w, (XtPointer)&update, NULL);
            break;
        }
        case ID_FILTER_RESET:
        {   Arg args[1];
            int n;
            Widget widget;
            XmString xms;
            n = 0;
            xms = XmStringCreateSimple("");
            widget = XtNameToWidget(page, "FilterResult");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            XtSetValues(widget, args, n);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNsensitive, False); n++;
            widget = XtNameToWidget(page, "Accept Filter");
            XtSetValues(widget, args, n);
            break;
        }
        case ID_FILTER_FREE:
        {   if (use) free(use);
            break;
        }
    }
}

static void
SwitchEnableDisable(Widget w, XtPointer client_data, XtPointer call_data)
{
    const char* name = XtName(w);
    Widget box = NULL;
    Widget mean = NULL;
    Widget median = NULL;
    Widget page = XtParent(w);
    XmToggleButtonCallbackStruct* tbs =
        (XmToggleButtonCallbackStruct*) call_data;
    if (strcmp(name,"CenterGenes")==0)
    {   box = XtNameToWidget(page,"CenterGenesBox");
        mean = XtNameToWidget(box, "AdjustMeanGenes");
        median = XtNameToWidget(box, "AdjustMedianGenes");
    }
    else if (strcmp(name,"CenterArrays")==0)
    {   box = XtNameToWidget(page,"CenterArraysBox");
        mean = XtNameToWidget(box, "AdjustMeanArrays");
        median = XtNameToWidget(box, "AdjustMedianArrays");
    }
    else return; /* never get here */
    if (tbs->set == XmUNSET)
    {   XtSetSensitive(box, True);
        if (!XmToggleButtonGetState(mean) && !XmToggleButtonGetState(median))
            XmToggleButtonSetState(mean,True,False);
    }
    else XtSetSensitive(box, False);
}

static void Adjust(Widget w, XtPointer client_data, XtPointer call_data)
{   
    static Widget page = 0;
    int* which = (int*) client_data;
    switch (*which)
    {   case ID_ADJUST_INIT:
        {   Arg args[6];
            int n;
            Widget widget, frame, box;
            static int command = ID_ADJUST_EXECUTE;

            XmString xms;
            page = w;

            n = 0;
            XtSetArg(args[n], XmNx, 30); n++;
            XtSetArg(args[n], XmNy, 45); n++;
            widget = XmCreateToggleButton(page, "Log transform data", args, n);
            XtManageChild(widget);

            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 43); n++;
            XtSetArg(args[n], XmNwidth, 420); n++;
            XtSetArg(args[n], XmNheight, 28); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "Frame", args, n);
            XtManageChild(frame);

            n = 0;
            xms = XmStringCreateSimple("Center genes");
            XtSetArg(args[n], XmNx, 30); n++;
            XtSetArg(args[n], XmNy, 87); n++;
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateToggleButton(page, "CenterGenes", args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            XtAddCallback(widget, XmNarmCallback, SwitchEnableDisable, NULL);

            n = 0;
            XtSetArg(args[n], XmNx, 40); n++;
            XtSetArg(args[n], XmNy, 110); n++;
            XtSetArg(args[n], XmNsensitive, False); n++;
            box = XmCreateRadioBox(page, "CenterGenesBox", args, n);
            XtManageChild(box);

            n = 0;
            xms = XmStringCreateSimple("Mean");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateToggleButton(box, "AdjustMeanGenes", args, n);
            XtManageChild(widget);
            XmStringFree(xms);

            n = 0;
            xms = XmStringCreateSimple("Median");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateToggleButton(box, "AdjustMedianGenes", args, n);
            XtManageChild(widget);
            XmStringFree(xms);

            n = 0;
            xms = XmStringCreateSimple("Normalize genes");
            XtSetArg(args[n], XmNx,  30); n++;
            XtSetArg(args[n], XmNy, 169); n++;
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateToggleButton(page, "AdjustNormalizeGenes", args, n);
            XtManageChild(widget);
            XmStringFree(xms);

            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 85); n++;
            XtSetArg(args[n], XmNwidth,  200); n++;
            XtSetArg(args[n], XmNheight, 110); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "", args, n);
            XtManageChild(frame);

            n = 0;
            xms = XmStringCreateSimple("Center arrays");
            XtSetArg(args[n], XmNx,250); n++;
            XtSetArg(args[n], XmNy, 87); n++;
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateToggleButton(page, "CenterArrays", args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            XtAddCallback(widget, XmNarmCallback, SwitchEnableDisable, NULL);

            n = 0;
            XtSetArg(args[n], XmNx,260); n++;
            XtSetArg(args[n], XmNy, 110); n++;
            XtSetArg(args[n], XmNsensitive, False); n++;
            box = XmCreateRadioBox(page, "CenterArraysBox", args, n);
            XtManageChild(box);

            n = 0;
            xms = XmStringCreateSimple("Mean");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateToggleButton(box, "AdjustMeanArrays", args, n);
            XtManageChild(widget);
            XmStringFree(xms);

            n = 0;
            xms = XmStringCreateSimple("Median");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateToggleButton(box, "AdjustMedianArrays", args, n);
            XtManageChild(widget);

            XmStringFree(xms);
            n = 0;
            xms = XmStringCreateSimple("Normalize arrays");
            XtSetArg(args[n], XmNx, 250); n++;
            XtSetArg(args[n], XmNy, 169); n++;
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateToggleButton(page, "AdjustNormalizeArrays", args, n);
            XtManageChild(widget);
            XmStringFree(xms);

            n = 0;
            XtSetArg(args[n], XmNx,240); n++;
            XtSetArg(args[n], XmNy, 85); n++;
            XtSetArg(args[n], XmNwidth,  200); n++;
            XtSetArg(args[n], XmNheight, 110); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "", args, n);
            XtManageChild(frame);

            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 210); n++;
            XtSetArg(args[n], XmNwidth, 200); n++;
            XtSetArg(args[n], XmNheight, 110); n++;
            XtSetArg(args[n], XmNeditable, False); n++;
            XtSetArg(args[n], XmNvalue, "Order of Operations\n\nLog Transform\nCenter Genes\nNormalize Genes\nCenter Arrays\nNormalize Arrays"); n++;
            widget = XmCreateText(page, "", args, n);
            XtManageChild(widget);

            n = 0;
            XtSetArg(args[n], XmNx, 340); n++;
            XtSetArg(args[n], XmNy, 210); n++;
            widget = XmCreatePushButton(page, "Apply", args, n);
            XtManageChild(widget);
            XtAddCallback(widget, XmNactivateCallback, Adjust, (XtPointer)&command);

            n = 0;
            XtSetArg(args[n], XmNwidth, 440); n++;
            XtSetArg(args[n], XmNheight, 320); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "Frame", args, n);
            XtManageChild(frame);

            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            xms = XmStringCreateSimple("Adjust Data");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(frame, NULL, args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            break;
        }
        case ID_ADJUST_EXECUTE:
        {   int ok;
            int bLogTransform;
            int GeneMeanCenter = False;
            int GeneMedianCenter = False;
            int GeneNormalize;
            int ArrayMeanCenter = False;
            int ArrayMedianCenter = False;
            int ArrayNormalize;
            Widget button;

            Statusbar(NULL, "Adjusting data");

            button = XtNameToWidget(page,"Log transform data");
            bLogTransform = XmToggleButtonGetState(button);
            if (bLogTransform) LogTransform();

            button = XtNameToWidget(page, "CenterGenes");
            if (XmToggleButtonGetState(button))
            {   Widget box = XtNameToWidget(page, "CenterGenesBox");
                button = XtNameToWidget(box, "AdjustMeanGenes");
                GeneMeanCenter = XmToggleButtonGetState(button);
                button = XtNameToWidget(box, "AdjustMedianGenes");
                GeneMedianCenter = XmToggleButtonGetState(button);
            }
            button = XtNameToWidget(page,"AdjustNormalizeGenes");
            GeneNormalize = XmToggleButtonGetState(button);
            ok = AdjustGenes(GeneMeanCenter,
                             GeneMedianCenter,
                             GeneNormalize);
            if (!ok)
            {   ShowError(w, "Memory allocation error", "Insufficient memory");
                Statusbar(NULL, "Memory allocation error");
                return;
            }

            button = XtNameToWidget(page, "CenterArrays");
            if (XmToggleButtonGetState(button))
            {   Widget box = XtNameToWidget(page, "CenterArraysBox");
                button = XtNameToWidget(box, "AdjustMeanArrays");
                ArrayMeanCenter = XmToggleButtonGetState(button);
                button = XtNameToWidget(box, "AdjustMedianArrays");
                ArrayMedianCenter = XmToggleButtonGetState(button);
            }
            button = XtNameToWidget(page, "AdjustNormalizeArrays");
            ArrayNormalize = XmToggleButtonGetState(button);
            ok = AdjustArrays(ArrayMeanCenter,
                              ArrayMedianCenter, 
                              ArrayNormalize);
            if (!ok)
            {   ShowError(w, "Memory allocation error", "Insufficient memory");
                Statusbar(NULL, "Memory allocation error");
                return;
            }

            Statusbar(NULL, "Done adjusting data");
            break;
        }
    }
}

static void
SwitchGeneWeight(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget page = XtParent(w);
    XmToggleButtonCallbackStruct* tbs =
        (XmToggleButtonCallbackStruct*) call_data;
    Widget widget = XtNameToWidget(page,"GeneWeight");
    if (tbs->set == XmUNSET) XtManageChild(widget);
    else XtUnmanageChild(widget);
}

static void
SwitchArrayWeight(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget page = XtParent(w);
    XmToggleButtonCallbackStruct* tbs =
        (XmToggleButtonCallbackStruct*) call_data;
    Widget widget = XtNameToWidget(page,"ArrayWeight");
    if (tbs->set == XmUNSET) XtManageChild(widget);
    else XtUnmanageChild(widget);
}

static void KMeans(Widget w, XtPointer client_data, XtPointer call_data)
{   
    static Widget page = NULL;
    int* which = (int*) client_data;
    switch (*which)
    {   case ID_KMEANS_INIT:
        {   Arg args[5];
            int n;
            Widget widget, frame, GeneMethod, ArrayMethod;
            XmString xms;
            static int command = ID_KMEANS_EXECUTE;

            page = w;
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 40); n++;
            widget = XmCreateToggleButton(page, "Organize genes", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 70); n++;
            XtSetArg(args[n], XmNwidth, 80); n++;
            XtSetArg(args[n], XmNvalue, "10"); n++;
            widget = XmCreateText(page, "KMeansGeneK", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 105); n++;
            XtSetArg(args[n], XmNy, 76); n++;
            widget = XmCreateLabel(page, "number of clusters (k)", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 145); n++;
            XtSetArg(args[n], XmNy, 116); n++;
            widget = XmCreateLabel(page, "number of runs", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 110); n++;
            XtSetArg(args[n], XmNwidth, 120); n++;
            XtSetArg(args[n], XmNvalue, "100"); n++;
            widget = XmCreateText(page, "KMeansGeneRuns", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 150); n++;
            XtSetArg(args[n], XmNwidth, 130); n++;
            XtSetArg(args[n], XmNheight, 80); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "GeneMethodFrame", args, n);
            XtManageChild(frame);
            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            widget = XmCreateLabel(frame, "Method", args, n);
            XtManageChild(widget);
            n = 0;
            GeneMethod = XmCreateRadioBox(frame, "GeneMethod", args, n);
            XtManageChild(GeneMethod);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 20); n++;
            XtSetArg(args[n], XmNset, XmSET); n++;
            widget = XmCreateToggleButton(GeneMethod, "k-Means", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 40); n++;
            widget = XmCreateToggleButton(GeneMethod, "k-Medians", args, n);
            XtManageChild(widget);
            CreateMetricComboBox(page, 20, 240, "GeneMetric", 'e');
            n = 0;
            XtSetArg(args[n], XmNwidth, 255); n++;
            XtSetArg(args[n], XmNheight, 295); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "Frame", args, n);
            XtManageChild(frame);
            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            xms = XmStringCreateSimple("Genes");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(frame, NULL, args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNx, 290); n++;
            XtSetArg(args[n], XmNy, 40); n++;
            widget = XmCreateToggleButton(page, "Organize arrays", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 290); n++;
            XtSetArg(args[n], XmNy, 70); n++;
            XtSetArg(args[n], XmNwidth, 80); n++;
            XtSetArg(args[n], XmNvalue, "10"); n++;
            widget = XmCreateText(page, "KMeansArrayK", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 375); n++;
            XtSetArg(args[n], XmNy, 76); n++;
            widget = XmCreateLabel(page, "number of clusters (k)", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 415); n++;
            XtSetArg(args[n], XmNy, 116); n++;
            widget = XmCreateLabel(page, "number of runs", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 290); n++;
            XtSetArg(args[n], XmNy, 110); n++;
            XtSetArg(args[n], XmNwidth, 120); n++;
            XtSetArg(args[n], XmNvalue, "100"); n++;
            widget = XmCreateText(page, "KMeansArrayRuns", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 290); n++;
            XtSetArg(args[n], XmNy, 150); n++;
            XtSetArg(args[n], XmNwidth, 130); n++;
            XtSetArg(args[n], XmNheight, 80); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "ArrayMethodFrame", args, n);
            XtManageChild(frame);
            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            widget = XmCreateLabel(frame, "Method", args, n);
            XtManageChild(widget);
            n = 0;
            ArrayMethod = XmCreateRadioBox(frame, "ArrayMethod", args, n);
            XtManageChild(ArrayMethod);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 20); n++;
            XtSetArg(args[n], XmNset, XmSET); n++;
            widget = XmCreateToggleButton(ArrayMethod, "k-Means", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 40); n++;
            widget = XmCreateToggleButton(ArrayMethod, "k-Medians", args, n);
            XtManageChild(widget);
            CreateMetricComboBox(page, 290, 240, "ArrayMetric", 'e');
            n = 0;
            XtSetArg(args[n], XmNx, 280); n++;
            XtSetArg(args[n], XmNwidth, 255); n++;
            XtSetArg(args[n], XmNheight, 295); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "Frame", args, n);
            XtManageChild(frame);
            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            xms = XmStringCreateSimple("Arrays");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(frame, NULL, args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNx, 245); n++;
            XtSetArg(args[n], XmNy, 315); n++;
            widget = XmCreatePushButton(page, "Execute", args, n);
            XtManageChild(widget);
            XtAddCallback(widget, XmNactivateCallback, KMeans, (XtPointer)&command);
            break;
        }
        case ID_KMEANS_EXECUTE:
        {   const int Rows = GetRows();
            const int Columns = GetColumns();
            int n;
            int ok;
            char* path;
            char* filetag;
            FILE* outputfile;

            Boolean ClusterGenes;
            Boolean ClusterArrays;

            int kGenes = 0;
            int kArrays = 0;

            Widget widget, button;
            Widget notebook = XtParent(page);
            Widget work = XtParent(notebook);

            if (Rows==0 || Columns==0)
            {   Statusbar(NULL, "No data available");
                return;
            }

            button = XtNameToWidget(page,"Organize genes");
            ClusterGenes = XmToggleButtonGetState(button);
            button = XtNameToWidget(page,"Organize arrays");
            ClusterArrays = XmToggleButtonGetState(button);
            if (!ClusterGenes && !ClusterArrays) return;
            /* Nothing to do */

            path = GetBaseName(work);
            if (!path)
            {   Statusbar(NULL, "Memory allocation failure");
                return;
            }
            n = strlen(path) + strlen("_K") + strlen(".ext") + 1;
            /* .ext represents the extension (kgg, kag, or cdt) */

            if (ClusterGenes)
            {   int dummy;
                kGenes = GetWidgetItemInt(page, "KMeansGeneK");
                if (kGenes==0)
                {   Statusbar(NULL, "Choose a nonzero number of clusters");
                    return;
                }
                if (Rows < kGenes)
                {   Statusbar(NULL, "More clusters than genes available");
                    return;
                }
                n += strlen("_G");
                dummy = kGenes;
                do n++; while (dummy/=10);
            }

            if (ClusterArrays)
            {   int dummy;
                kArrays = GetWidgetItemInt(page, "KMeansArrayK");
                if (kArrays==0)
                {   Statusbar(NULL, "Choose a nonzero number of clusters");
                    return;
                }
                if (Columns < kArrays)
                {   Statusbar(NULL, "More clusters than arrays available");
                    return;
                }
                n += strlen("_A");
                dummy = kArrays;
                do n++; while (dummy/=10);
            }

            path = realloc(path, n*sizeof(char));
            filetag = strchr(path,'\0');

            Statusbar(NULL, "Executing k-means clustering");

            if (ClusterGenes)
            {   char method;
                char dist;
                int* NodeMap;
                int nTrials;
                int ifound;

                char buffer[256];

                Widget frame = XtNameToWidget(page,"GeneMethodFrame");
                widget = XtNameToWidget(frame, "GeneMethod");
                button = XtNameToWidget(widget, "k-Means");
                method = XmToggleButtonGetState(button) ? 'a' : 'm';
                /* 'a' is average (mean), 'm' is median */

                widget = XtNameToWidget(page,"GeneMetric");
                dist = GetMetric(widget);
                NodeMap = malloc(Rows*sizeof(int));
                if (!NodeMap)
                {   Statusbar(NULL, "Memory allocation failure");
                    free(path);
                    return;
                }
                nTrials = GetWidgetItemInt(page, "KMeansGeneRuns");

                ifound = GeneKCluster(kGenes, nTrials, method, dist, NodeMap);
                if (ifound < 0)
                {   Statusbar(NULL, "Memory allocation failure");
                    free(path);
                    return;
                }
                sprintf(buffer, "Solution was found %d times", ifound);
                Statusbar(NULL, buffer);

                sprintf(filetag, "_K_G%d.kgg", kGenes);
                outputfile = fopen(path, "wt");
                if (!outputfile)
                {   Statusbar(NULL, "Error: Unable to open the output file");
                    free(NodeMap);
                    free(path);
                    return;
                }

                ok = SaveGeneKCluster(outputfile, kGenes, NodeMap);
                fclose(outputfile);
                free(NodeMap);
                if (!ok)
                {   Statusbar(NULL, "Error: Failed to allocate memory while saving");
                    free(path);
                    return;
                }
            }

            if (ClusterArrays)
            {   char method;
                char dist;
                int* NodeMap;
                int nTrials;
                int ifound;

                char buffer[256];
 
                Widget frame = XtNameToWidget(page,"ArrayMethodFrame");
                widget = XtNameToWidget(frame, "ArrayMethod");
                button = XtNameToWidget(widget, "k-Means");
                method = XmToggleButtonGetState(button) ? 'a' : 'm';
                /* 'a' is average, 'm' is median */

                widget = XtNameToWidget(page,"ArrayMetric");
                dist = GetMetric(widget);
                NodeMap = malloc(Columns*sizeof(int));
                if (!NodeMap)
                {   Statusbar(NULL, "Memory allocation failure");
                    free(path);
                    return;
                }
                nTrials = GetWidgetItemInt(page, "KMeansArrayRuns");
                ifound = ArrayKCluster(kArrays, nTrials, method, dist, NodeMap);
                if (ifound < 0)
                {   Statusbar(NULL, "Memory allocation failure");
                    free(path);
                    return;
                }
                sprintf(buffer, "Solution was found %d times", ifound);
                Statusbar(NULL, buffer);

                sprintf(filetag, "_K_A%d.kag", kArrays);
                outputfile = fopen(path, "wt");
                if (!outputfile)
                {   Statusbar(NULL, "Error: Unable to open the output file");
                    free(NodeMap);
                    free(path);
                    return;
                }

                ok = SaveArrayKCluster(outputfile, kArrays, NodeMap);
                fclose(outputfile);
                free(NodeMap);
                if (!ok)
                {   Statusbar(NULL, "Error: Failed to allocate memory while saving");
                    free(path);
                    return;
                }
            }

            /* Now write the data file */
            filetag += sprintf(filetag, "_K");
            if (ClusterGenes) filetag += sprintf(filetag, "_G%d", kGenes);
            if (ClusterArrays) filetag += sprintf(filetag, "_A%d", kArrays);
            sprintf(filetag,".cdt");

            outputfile = fopen(path, "wt");
            free(path);
            if (!outputfile)
            {   Statusbar(NULL, "Error: Unable to open the output file");
                return;
            }
            ok = Save(outputfile, 0, 0);
            fclose(outputfile);
            if (ok) Statusbar(NULL, "Done clustering");
            else
            { ShowError(w, "Error saving file", "Insufficient memory");
              Statusbar(NULL, "Error saving to file");
            }
            break;
        }
    }
}

static void PCA(Widget w, XtPointer client_data, XtPointer call_data)
{   
    static Widget page = NULL;
    int* which = (int*) client_data;
    switch (*which)
    {   case ID_PCA_INIT:
        {   Arg args[5];
            int n;
            Widget widget, frame;
            XmString xms;
            static int command = ID_PCA_EXECUTE;

            page = w;
            n = 0;
            XtSetArg(args[n], XmNx, 180); n++;
            widget = XmCreateLabel(page, "Principal Component Analysis", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 70); n++;
            widget = XmCreateToggleButton(page, "Apply PCA to genes", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNy, 30); n++;
            XtSetArg(args[n], XmNwidth, 255); n++;
            XtSetArg(args[n], XmNheight, 275); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "Frame", args, n);
            XtManageChild(frame);
            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            xms = XmStringCreateSimple("Genes");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(frame, NULL, args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNx, 290); n++;
            XtSetArg(args[n], XmNy, 70); n++;
            widget = XmCreateToggleButton(page, "Apply PCA to arrays", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 280); n++;
            XtSetArg(args[n], XmNy, 30); n++;
            XtSetArg(args[n], XmNwidth, 255); n++;
            XtSetArg(args[n], XmNheight, 275); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "Frame", args, n);
            XtManageChild(frame);
            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            xms = XmStringCreateSimple("Arrays");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(frame, NULL, args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNx, 240); n++;
            XtSetArg(args[n], XmNy, 315); n++;
            widget = XmCreatePushButton(page, "Execute", args, n);
            XtManageChild(widget);
            XtAddCallback(widget, XmNactivateCallback, PCA, (XtPointer)&command);
            break;
        }
        case ID_PCA_EXECUTE:
        {   Boolean DoGenePCA;
            Boolean DoArrayPCA;
            Widget button;

            const char* error;
            const int Rows = GetRows();
            const int Columns = GetColumns();
            char* base;
            char* path = NULL;
            char* extension;
            FILE* coordinatefile;
            FILE* pcfile;
            Widget notebook = XtParent(page);
            Widget work = XtParent(notebook);

            button = XtNameToWidget(page, "Apply PCA to genes");
            DoGenePCA = XmToggleButtonGetState(button);
            button = XtNameToWidget(page, "Apply PCA to arrays");
            DoArrayPCA = XmToggleButtonGetState(button);

            if (Rows==0 || Columns==0)
            {   Statusbar(NULL, "No data available");
                return;
            }

            base = GetBaseName(work);
            if (base)
                path = realloc(base, strlen(base)+strlen("_pca_array.coords.txt"));
            if (!path)
            {   Statusbar(NULL, "Memory allocation failure");
                if (base) free(base);
                return;
            }
            extension = strchr(path, '\0');
 
            if (DoGenePCA)
            {
                Statusbar(NULL, "Calculating PCA");
                sprintf(extension, "_pca_gene.coords.txt");
                coordinatefile = fopen(path, "wt");
                sprintf(extension, "_pca_gene.pc.txt");
                pcfile = fopen(path, "wt");
                if (!coordinatefile || !pcfile)
                { Statusbar(NULL, "Error: Unable to open the output file");
                  if (coordinatefile) fclose(coordinatefile);
                  if (pcfile) fclose(pcfile);
                  free(path);
                  return;
                }
                error = PerformGenePCA(coordinatefile, pcfile);
                fclose(coordinatefile);
                fclose(pcfile);
                if (error)
                { ShowError(w, error, "Error");
                  free(path);
                  return;
                }
                Statusbar(NULL, "Finished Principal Component Analysis");
            }
            if (DoArrayPCA)
            {
                Statusbar(NULL, "Calculating PCA");
                sprintf(extension, "_pca_array.coords.txt");
                coordinatefile = fopen(path, "wt");
                sprintf(extension, "_pca_array.pc.txt");
                pcfile = fopen(path, "wt");
                if (!coordinatefile || !pcfile)
                { Statusbar(NULL, "Error: Unable to open the output file");
                  if (coordinatefile) fclose(coordinatefile);
                  if (pcfile) fclose(pcfile);
                  free(path);
                  return;
                }
                error = PerformArrayPCA(coordinatefile, pcfile);
                fclose(coordinatefile);
                fclose(pcfile);
                if (error)
                { ShowError(w, error, "Error");
                  free(path);
                  return;
                }
                Statusbar(NULL, "Finished Principal Component Analysis");
            }
            free(path);
            break;
        }
    }
}

/*============================================================================*/
/* Callback functions --- Main dialog window                                  */
/*============================================================================*/

static void OpenFile(Widget w, XtPointer client_data, XtPointer call_data)
{   char* filename = NULL;
    char** directory = (char**)client_data;
    char* result;
    struct stat filestat;
    FILE* inputfile;
    XmFileSelectionBoxCallbackStruct* cbs;
    XtUnmanageChild(w);
    cbs = (XmFileSelectionBoxCallbackStruct*) call_data;
    if (!cbs) return;
    filename = (char*)XmStringUnparse(cbs->value,
                                      NULL,
                                      XmCHARSET_TEXT,
                                      XmCHARSET_TEXT,
                                      NULL,
                                      0,
                                      XmOUTPUT_ALL);
    if (!filename) return;
    if (stat(filename, &filestat) || S_ISDIR(filestat.st_mode))
    {   XtFree(filename);
        return;
    }
    /* Save the directory name based on the file name */
    if (*directory) free(*directory);
    *directory = (char*)XmStringUnparse(cbs->dir,
                                        NULL,
                                        XmCHARSET_TEXT,
                                        XmCHARSET_TEXT,
                                        NULL,
                                        0,
                                        XmOUTPUT_ALL);
    /* Open the file */
    inputfile = fopen(filename, "rt");
    if (!inputfile) Statusbar(NULL,"Error opening file");
    /* Read file */
    result = Load(inputfile);
    fclose(inputfile);
    if (result && strcmp(result, "ok")==0)
    {   /* Extract job name from file name */
        int command;
        char* jobname = strrchr(filename,'/') + 1;
        char* extension = strrchr(jobname,'.');
        command = ID_FILEMANAGER_SET_FILEMEMO;
        FileManager(NULL, (XtPointer)&command, filename);
        if (extension) *extension = '\0';
        command = ID_FILEMANAGER_SET_JOBNAME;
        FileManager(NULL, (XtPointer)&command, jobname);
        command = ID_FILEMANAGER_UPDATE_ROWS_COLUMNS;
        FileManager(NULL, (XtPointer)&command, NULL);
        command = ID_FILTER_RESET;
        Filter(NULL, (XtPointer)&command, NULL);
        command = ID_SOM_UPDATE;
        SOM(w, (XtPointer)&command, NULL);
        Statusbar(NULL, "Done loading data");
    }
    else
    {   char buffer[256];
        int command;
        if (result)
        {   ShowError(w, result, "Error in data file");
            free(result);
        }
        else ShowError(w, "Insufficient memory", "Error reading file");
        sprintf(buffer, "Error reading file %s", filename);
        Statusbar(NULL, buffer);
        command = ID_FILEMANAGER_SET_FILEMEMO;
        FileManager(NULL, (XtPointer)&command, "");
        command = ID_FILEMANAGER_SET_JOBNAME;
        FileManager(NULL, (XtPointer)&command, "");
        command = ID_FILEMANAGER_UPDATE_ROWS_COLUMNS;
        FileManager(NULL, (XtPointer)&command, NULL);
    }
    XtFree(filename);
}

static void SaveFile(Widget w, XtPointer client_data, XtPointer call_data)
{   int ok;
    char* filename = NULL;
    struct stat filestat;
    XmFileSelectionBoxCallbackStruct* cbs = NULL;
    FILE* outputfile;
    char** directory = (char**)client_data;

    XtUnmanageChild(w);
    cbs = (XmFileSelectionBoxCallbackStruct*) call_data;
    if (!cbs) return;
    filename = (char*)XmStringUnparse(cbs->value,
                                      NULL,
                                      XmCHARSET_TEXT,
                                      XmCHARSET_TEXT,
                                      NULL,
                                      0,
                                      XmOUTPUT_ALL);
    if (*directory) free(*directory);
    *directory = (char*)XmStringUnparse(cbs->dir,
                                        NULL,
                                        XmCHARSET_TEXT,
                                        XmCHARSET_TEXT,
                                        NULL,
                                        0,
                                        XmOUTPUT_ALL);
    if (!stat(filename, &filestat) && S_ISDIR(filestat.st_mode))
    {   Statusbar(NULL, "Error saving file: Directory selected");
        return;
    }
    /* Save the data to file */
    outputfile = fopen(filename, "wt");
    free(filename);
    if (!outputfile)
    {   Statusbar(NULL, "Error: Unable to open the output file");
        return;
    }
    ok = Save(outputfile, 0, 0);
    fclose(outputfile);
    if (ok) Statusbar(NULL, "Finished saving file");
    else
    {   ShowError(w, "Insufficient memory", "Error saving file"),
        Statusbar(NULL, "Error saving to file");
    }
}

static void Cancel(Widget w, XtPointer client_data, XtPointer call_data)
{   Statusbar(NULL, "Cancelled");
    XtUnmanageChild(w);
}

/*============================================================================*/
/* Callback functions --- Menu                                                */
/*============================================================================*/

static void MenuFile(Widget w, XtPointer client_data, XtPointer call_data)
{   static char* directory = NULL;
    int* which = (int*)client_data;
    switch (*which)
    {   case CMD_FILE_OPEN:
        /* User will select a data file (*.txt) */
        {   Widget dialog, parent, widget;
            Arg args[4];
            int n;

            XmString mask = XmStringCreateSimple("*.txt");
            XmString title = XmStringCreateSimple("Select data file to open");
            XmString initdir = XmStringCreateSimple(directory);

            n = 0;
            XtSetArg(args[n], XmNwidth, 300); n++;
            XtSetArg(args[n], XmNdirMask, mask); n++;
            XtSetArg(args[n], XmNdialogTitle, title); n++;
            XtSetArg(args[n], XmNdirectory, initdir); n++;

            parent = XtParent(w);
            dialog = XmCreateFileSelectionDialog(parent, "FileOpen", args, n);
            widget = XmSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON);
            XtUnmanageChild(widget);

            XmStringFree(mask);
            XmStringFree(title);
            XtAddCallback(dialog, XmNokCallback, OpenFile, &directory);
            XtAddCallback(dialog, XmNcancelCallback, Cancel, NULL);
            XtManageChild(dialog);
            Statusbar(NULL, "Opening file");
            break;
        }
        case CMD_FILE_SAVE:
        {   static Widget dialog = NULL;
            Arg args[4];
            int n;
            char buffer[256];
            XmString filename;
            Widget menubar = XtParent(XtParent(XtParent(w)));
            Widget main_w = XtParent(menubar);
            Widget work = XtNameToWidget(main_w,"work");
            Widget widget = XtNameToWidget(work,"Jobname");
            char* jobname = XmTextGetString(widget);
            sprintf(buffer,"%s.txt",jobname);
            XtFree(jobname);
            filename = XmStringCreateSimple(buffer);
            if (!dialog)
            {   XmString mask = XmStringCreateSimple("*.txt");
                XmString title = XmStringCreateSimple("Select file name to save to");
                XmString initdir = XmStringCreateSimple(directory);
                n = 0;
                XtSetArg(args[n], XmNwidth, 300); n++;
                XtSetArg(args[n], XmNdirMask, mask); n++;
                XtSetArg(args[n], XmNdialogTitle, title); n++;
                XtSetArg(args[n], XmNdirectory, initdir); n++;

                dialog = XmCreateFileSelectionDialog(XtParent(w), "FileSave", args, n);
                XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
                XmStringFree(mask);
                XmStringFree(title);
                XmStringFree(initdir);
                XtAddCallback(dialog, XmNokCallback, SaveFile, (XtPointer)&directory);
                XtAddCallback(dialog, XmNcancelCallback, Cancel, NULL);
            }
            n = 0;
            XtSetArg(args[n], XmNdirSpec, filename); n++;
            XtSetValues(dialog, args, n);
            XmStringFree(filename);
            XtManageChild(dialog);
            Statusbar(NULL, "Saving data to file");
            break;
        }
        case CMD_FILE_QUIT:
        {   int command;
            if (directory) free(directory);
            Free();
            command = ID_FILTER_FREE;
            Filter(NULL, (XtPointer)&command, NULL);
            exit(0);
            break;
        }
    }
    return;
}

static void MenuHelp(Widget w, XtPointer client_data, XtPointer call_data)
/* Note: In this function, PREFIX should be defined by a command line
 * definition.
 */
{   int* item_no = (int*) client_data;
    switch (*item_no)
    {   case CMD_HELP_HTMLHELP:
        {   system("firefox "PREFIX"/cluster/html/index.html &");
            break;
        }
        case CMD_HELP_MANUAL:
        {   system("acroread "PREFIX"/cluster/doc/cluster3.pdf &");
            break;
        }
        case CMD_HELP_DOWNLOAD:
        {   system("firefox http://bonsai.ims.u-tokyo.ac.jp/~mdehoon/software/cluster/manual/index.html &");
            break;
        }
        case CMD_HELP_FILEFORMAT:
        {   Arg args[6];
            int n = 0;
            Widget dialog, widget;
            Pixmap pixmap;
            /* Text is split up for ANSI compliance */
            char* helplines[12];
            char* helptext;
            int nchars = 1; /* One for the final \0 */
            XmString xms;
            helplines[0] = \
"The input for the clustering program is a tab-delimited text file.\n";
            helplines[1] = \
"An example is shown below.\n\n";
            helplines[2] = \
"The cells in red must appear in the file, although they can be any string.\n";
            helplines[3] = \
"The cells in bold are headers for optional columns/rows.\n\n";
            helplines[4] = \
"UNIQID: (string/number)\nThis column should contain unique identifiers for each gene.\n\n";
            helplines[5] = \
"NAME: (string)\nA text description of each gene which will be used in display.\n\n";
            helplines[6] = \
"EWEIGHT: (real number)\nA weight for each experiment that can be used to count certain experiments\nmore than others.\n\n";
            helplines[7] = \
"GWEIGHT: (real number)\nA similar weight for each gene can be used when clustering arrays.\n\n";
            helplines[8] = \
"GORDER: (real number)\nA value to be used for ordering nodes in display program\n\n";
            helplines[9] = \
"EXPID: (string, e.g. EXP1, EXP2,...)\nA text description of each experiment that will be used in the display.\n\n";
            helplines[10] = \
"DATA: (real number)\nData for a single gene in a single experiment. Any desired numerical transform\n";
            helplines[11] = \
"(e.g. log) should be applied before clustering. Missing values are acceptable.";
            for (n=0; n<12; n++) nchars += strlen(helplines[n]);
            helptext = malloc(nchars*sizeof(char));
            if (!helptext)
            {   Statusbar(NULL, "Memory allocation failuere");
                return;
            }

            helptext[0] = '\0';
            for (n=0; n<12; n++) strcat(helptext, helplines[n]);
            n = 0;
            xms = XmStringCreateSimple("File Format");
            XtSetArg(args[n], XmNdialogTitle, xms); n++;
            w = XtParent(w);
            dialog = XmCreateBulletinBoardDialog(w, "FileFormat", args, n);
            XtManageChild(dialog);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNx, 60); n++;
            XtSetArg(args[n], XmNy, 10); n++;
            XtSetArg(args[n], XmNwidth, 500); n++;
            XtSetArg(args[n], XmNheight, 390); n++;
            XtSetArg(args[n], XmNeditable, False); n++;
            XtSetArg(args[n], XmNvalue, helptext); n++;
            widget = XmCreateText(dialog, "", args, n);
            XtManageChild(widget);
            free(helptext);
            n = 0;
            pixmap = XmGetPixmap(XtScreen(dialog),PREFIX"/cluster/format.xpm",0,0);
            XtSetArg(args[n], XmNx, 10); n++;
            XtSetArg(args[n], XmNy, 410); n++;
            XtSetArg(args[n],XmNlabelType, XmPIXMAP); n++;
            XtSetArg(args[n],XmNlabelPixmap, pixmap); n++;
            widget = XmCreateLabel(dialog, "LabelPixmap", args, n);
            XtManageChild(widget);
            break;
        }
        case CMD_HELP_ABOUT:
        {   Arg args[3];
            int n = 0;
            Widget dialog, widget;
            XmString xms = XmStringCreateSimple("About Cluster");
            XtSetArg(args[n], XmNdialogTitle, xms); n++;
            w = XtParent(w);
            dialog = XmCreateBulletinBoardDialog(w, "About", args, n);
            XtManageChild(dialog);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNx, 10); n++;
            XtSetArg(args[n], XmNy, 10); n++;
            XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
            widget = XmCreateLabel(dialog, "Cluster 3.0\nusing the C Clustering Library version " CLUSTERVERSION ".\n\nCluster was originally written by Michael Eisen\n(eisen 'AT' rana.lbl.gov)\nCopyright 1998-99 Stanford University\n\nCluster version 3.0 for X11/Motif was created\nby Michiel de Hoon (mdehoon 'AT' gsc.riken.jp),\ntogether with Seiya Imoto and Satoru Miyano.\n\nType 'cluster --help' for information about\nrunning Cluster 3.0 as a command-line program.\n\nUniversity of Tokyo, Human Genome Center\nJune 2002", args, n);
            XtManageChild(widget);
            break;
        }
    }
}

static void Hierarchical(Widget w, XtPointer client_data, XtPointer call_data)
{
    static Widget page = 0;
    int* which = (int*) client_data;
    switch (*which)
    {   case ID_HIERARCHICAL_INIT:
        {   Arg args[5];
            int n;
            Widget widget, frame, geneweight, arrayweight;
            Widget GeneWeightPage, ArrayWeightPage;
            XmString xms;
            static int single_command = ID_HIERARCHICAL_SINGLE;
            static int complete_command = ID_HIERARCHICAL_COMPLETE;
            static int average_command = ID_HIERARCHICAL_AVERAGE;
            static int centroid_command = ID_HIERARCHICAL_CENTROID;

            page = w;
            n = 0;
            xms = XmStringCreateSimple("Cluster");
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 40); n++;
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateToggleButton(page, "ClusterGenes", args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            xms = XmStringCreateLocalized("Calculate\nweights");
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 85); n++;
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateToggleButton(page, "CalculateGeneWeights", args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            XtAddCallback(widget, XmNarmCallback, SwitchArrayWeight, NULL);
            CreateMetricComboBox(page, 20, 140, "GeneMetric", 'u');
            n = 0;
            XtSetArg(args[n], XmNx, 120); n++;
            XtSetArg(args[n], XmNy, 30); n++;
            XtSetArg(args[n], XmNwidth, 130); n++;
            XtSetArg(args[n], XmNheight, 100); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            geneweight = XmCreateFrame(page, "GeneWeight", args, n);
            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            xms = XmStringCreateSimple("Weight Options");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(geneweight,NULL,args,n);
            XtManageChild(widget);
            XmStringFree(xms);
            GeneWeightPage = XtCreateManagedWidget("GeneWeightPage",xmBulletinBoardWidgetClass,geneweight,NULL,0);
            n = 0;
            XtSetArg(args[n], XmNx, 10); n++;
            XtSetArg(args[n], XmNy, 16); n++;
            xms = XmStringCreateSimple("Cutoff");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(GeneWeightPage, "", args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNx, 55); n++;
            XtSetArg(args[n], XmNy, 10); n++;
            XtSetArg(args[n], XmNwidth, 65); n++;
            XtSetArg(args[n], XmNvalue, "0.1"); n++;
            widget = XmCreateText(GeneWeightPage, "GeneWeightCutoff", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 10); n++;
            XtSetArg(args[n], XmNy, 51); n++;
            xms = XmStringCreateSimple("Exponent");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(GeneWeightPage, "", args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNx, 70); n++;
            XtSetArg(args[n], XmNy, 45); n++;
            XtSetArg(args[n], XmNwidth, 50); n++;
            XtSetArg(args[n], XmNvalue, "1"); n++;
            widget = XmCreateText(GeneWeightPage, "GeneWeightExp", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNwidth, 255); n++;
            XtSetArg(args[n], XmNheight, 195); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "Frame", args, n);
            XtManageChild(frame);
            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            xms = XmStringCreateSimple("Genes");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(frame,NULL,args,n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            xms = XmStringCreateSimple("Cluster");
            XtSetArg(args[n], XmNx, 290); n++;
            XtSetArg(args[n], XmNy, 40); n++;
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateToggleButton(page, "ClusterArrays", args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            xms = XmStringCreateLocalized("Calculate\nweights");
            XtSetArg(args[n], XmNx, 290); n++;
            XtSetArg(args[n], XmNy, 85); n++;
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateToggleButton(page, "CalculateArrayWeights", args, n);
            XtAddCallback(widget, XmNarmCallback, SwitchGeneWeight, NULL);
            XmStringFree(xms);
            XtManageChild(widget);
            CreateMetricComboBox(page, 290, 140, "ArrayMetric", 'u');
            n = 0;
            XtSetArg(args[n], XmNx, 390); n++;
            XtSetArg(args[n], XmNy, 30); n++;
            XtSetArg(args[n], XmNwidth, 130); n++;
            XtSetArg(args[n], XmNheight, 100); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            arrayweight = XmCreateFrame(page, "ArrayWeight", args, n);
            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            xms = XmStringCreateSimple("Weight Options");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(arrayweight,NULL,args,n);
            XtManageChild(widget);
            XmStringFree(xms);
            ArrayWeightPage = XtCreateManagedWidget("ArrayWeightPage",xmBulletinBoardWidgetClass,arrayweight,NULL,0);
            n = 0;
            XtSetArg(args[n], XmNx, 10); n++;
            XtSetArg(args[n], XmNy, 16); n++;
            xms = XmStringCreateSimple("Cutoff");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(ArrayWeightPage, "", args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNx, 55); n++;
            XtSetArg(args[n], XmNy, 10); n++;
            XtSetArg(args[n], XmNwidth, 65); n++;
            XtSetArg(args[n], XmNvalue, "0.1"); n++;
            widget = XmCreateText(ArrayWeightPage, "ArrayWeightCutoff", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNx, 10); n++;
            XtSetArg(args[n], XmNy, 51); n++;
            xms = XmStringCreateSimple("Exponent");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(ArrayWeightPage, "", args, n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNx, 70); n++;
            XtSetArg(args[n], XmNy, 45); n++;
            XtSetArg(args[n], XmNwidth, 50); n++;
            XtSetArg(args[n], XmNvalue, "1"); n++;
            widget = XmCreateText(ArrayWeightPage, "ArrayWeightExp", args, n);
            XtManageChild(widget);
            n = 0;
            XtSetArg(args[n], XmNwidth, 255); n++;
            XtSetArg(args[n], XmNheight, 195); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "Frame", args, n);
            XtManageChild(frame);
            n = 0;
            XtSetArg(args[n], XmNx, 280); n++;
            XtSetArg(args[n], XmNwidth, 255); n++;
            XtSetArg(args[n], XmNheight, 195); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "Frame", args, n);
            XtManageChild(frame);
            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            xms = XmStringCreateSimple("Arrays");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(frame,NULL,args,n);
            XtManageChild(widget);
            XmStringFree(xms);
            n = 0;
            XtSetArg(args[n], XmNx, 20); n++;
            XtSetArg(args[n], XmNy, 243); n++;
            widget = XmCreatePushButton(page, "Centroid linkage", args, n);
            XtManageChild(widget);
            XtAddCallback(widget, XmNactivateCallback, Hierarchical, (XtPointer)&centroid_command);
            n = 0;
            XtSetArg(args[n], XmNx, 160); n++;
            XtSetArg(args[n], XmNy, 243); n++;
            widget = XmCreatePushButton(page, "Single linkage", args, n);
            XtManageChild(widget);
            XtAddCallback(widget, XmNactivateCallback, Hierarchical, (XtPointer)&single_command);
            n = 0;
            XtSetArg(args[n], XmNx, 290); n++;
            XtSetArg(args[n], XmNy, 243); n++;
            widget = XmCreatePushButton(page, "Complete Linkage", args, n);
            XtManageChild(widget);
            XtAddCallback(widget, XmNactivateCallback, Hierarchical, (XtPointer)&complete_command);
            n = 0;
            XtSetArg(args[n], XmNx, 425); n++;
            XtSetArg(args[n], XmNy, 243); n++;
            widget = XmCreatePushButton(page, "Average Linkage", args, n);
            XtManageChild(widget);
            XtAddCallback(widget, XmNactivateCallback, Hierarchical, (XtPointer)&average_command);
            n = 0;
            XtSetArg(args[n], XmNx, 10); n++;
            XtSetArg(args[n], XmNy, 215); n++;
            XtSetArg(args[n], XmNwidth, 525); n++;
            XtSetArg(args[n], XmNheight, 70); n++;
            XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
            frame = XmCreateFrame(page, "Frame", args, n);
            XtManageChild(frame);
            n = 0;
            XtSetArg(args[n], XmNchildType, XmFRAME_TITLE_CHILD); n++;
            xms = XmStringCreateSimple("Clustering method");
            XtSetArg(args[n], XmNlabelString, xms); n++;
            widget = XmCreateLabel(frame,NULL,args,n);
            XtManageChild(widget);
            XmStringFree(xms);
            break;
        }
        case ID_HIERARCHICAL_CENTROID:
        case ID_HIERARCHICAL_SINGLE:
        case ID_HIERARCHICAL_COMPLETE:
        case ID_HIERARCHICAL_AVERAGE:
        {   Widget widget, button;
            char method = 'e';
            /* So the compiler won't complain about method
             * not being initialized */
            int ok;
            const int Rows = GetRows();
            const int Columns = GetColumns();
            Boolean ClusterGenes;
            Boolean ClusterArrays;
            Boolean bCalculateGeneWeights;
            Boolean bCalculateArrayWeights;
            char genemetric;
            char arraymetric;
            char* path;
            char* extension;
            FILE* outputfile;

            Widget notebook = XtParent(page);
            Widget work = XtParent(notebook);

            if (Rows==0 || Columns==0)
            {   Statusbar(NULL, "No data available");
                return;
            }

            /* Find out what we need to do here */
            button = XtNameToWidget(page,"ClusterGenes");
            ClusterGenes = XmToggleButtonGetState(button);
            button = XtNameToWidget(page,"ClusterArrays");
            ClusterArrays = XmToggleButtonGetState(button);

            if (!ClusterGenes && !ClusterArrays) return;
            /* Nothing to do here */

            button = XtNameToWidget(page,"CalculateGeneWeights");
            bCalculateGeneWeights = XmToggleButtonGetState(button);
            button = XtNameToWidget(page,"CalculateArrayWeights");
            bCalculateArrayWeights = XmToggleButtonGetState(button);

            /* Find out which metrics to use */
            widget = XtNameToWidget(page,"GeneMetric");
            genemetric = GetMetric(widget);
            widget = XtNameToWidget(page,"ArrayMetric");
            arraymetric = GetMetric(widget);

            path = GetBaseName(work);
            if (path)
            {   char* p = realloc(path, strlen(path)+strlen(".ext")+1);
                if (!p) free(path);
                path = p;
            }
            if (!path)
            {   Statusbar(NULL, "Memory allocation failure");
                return;
            }
            extension = strchr(path, '\0');

            if (bCalculateGeneWeights || bCalculateArrayWeights)
            /* This means calculating array weights, which will be used when
             * clustering genes */
            {   const char* error;
                double gene_cutoff = 0.0;
                double gene_exponent = 0.0;
                double array_cutoff = 0.0;
                double array_exponent = 0.0;
                Statusbar(NULL, "Calculating weights");
                if (bCalculateGeneWeights)
                {   char* text;
                    Widget textfield;
                    Widget arrayweight = XtNameToWidget(page,"ArrayWeight");
                    Widget arrayweightpage = XtNameToWidget(arrayweight,"ArrayWeightPage");
                    textfield = XtNameToWidget(arrayweightpage,"ArrayWeightCutoff");
                    text = XmTextGetString(textfield);
                    array_cutoff = strtod(text, NULL);
                    XtFree(text);
                    textfield = XtNameToWidget(arrayweightpage,"ArrayWeightExp");
                    text = XmTextGetString(textfield);
                    array_exponent = strtod(text, NULL);
                    XtFree(text);
                }
                if (bCalculateArrayWeights)
                {   char* text;
                    Widget textfield;
                    Widget geneweight = XtNameToWidget(page,"GeneWeight");
                    Widget geneweightpage = XtNameToWidget(geneweight,"GeneWeightPage");
                    textfield = XtNameToWidget(geneweightpage,"GeneWeightCutoff");
                    text = XmTextGetString(textfield);
                    gene_cutoff = strtod(text, NULL);
                    XtFree(text);
                    textfield = XtNameToWidget(geneweightpage,"GeneWeightExp");
                    text = XmTextGetString(textfield);
                    gene_exponent = strtod(text, NULL);
                    XtFree(text);
                }
                error = CalculateWeights(gene_cutoff, gene_exponent, genemetric,
                          array_cutoff, array_exponent, arraymetric);
                if (error)
                {   free(path);
                    ShowError(w, error, "Error");
                    return;
                }
            }

            switch (*which)
            {   case ID_HIERARCHICAL_CENTROID:
                {   method = 'c';
                    Statusbar(NULL, "Performing centroid linkage hierarchical clustering");
                    break;
                }
                case ID_HIERARCHICAL_SINGLE:
                {   method = 's';
                    Statusbar(NULL, "Performing single linkage hierarchical clustering");
                    break;
                }
                case ID_HIERARCHICAL_COMPLETE:
                {   method = 'm';
                    Statusbar(NULL, "Performing complete linkage hierarchical clustering");
                    break;
                }
                case ID_HIERARCHICAL_AVERAGE:
                {   method = 'a';
                    Statusbar(NULL, "Performing average linkage hierarchical clustering");
                    break;
                }
            }
            if (ClusterGenes)
            {   sprintf(extension, ".gtr");
                outputfile = fopen(path, "wt");
                if (!outputfile)
                {   free(path);
                    Statusbar(NULL, "Error: Unable to open the output file");
                    return;
                }
                ok = HierarchicalCluster(outputfile, genemetric, False, method);
                fclose(outputfile);
                if (!ok)
                {   free(path);
                    Statusbar(NULL, "Error: Insufficient memory");
                    return;
                }
            }

            if (ClusterArrays)
            {   sprintf(extension, ".atr");
                outputfile = fopen(path, "wt");
                if (!outputfile)
                {   free(path);
                    Statusbar(NULL, "Error: Unable to open the output file");
                    return;
                }
                ok = HierarchicalCluster(outputfile, arraymetric, True, method);
                fclose(outputfile);
                if (!ok)
                {   free(path);
                    Statusbar(NULL, "Error: Insufficient memory");
                    return;
                }
            }

            Statusbar(NULL, "Saving the clustering result");

            /* Now make output .cdt file */
            sprintf(extension, ".cdt");
            outputfile = fopen(path, "wt");
            free(path);
            if (!outputfile)
            {   Statusbar(NULL, "Error: Unable to open the output file");
                return;
            }
            Save(outputfile, ClusterGenes, ClusterArrays);
            ok = fclose(outputfile);
            if (ok) Statusbar(NULL, "Done clustering");
            else
            {   ShowError(w, "Insufficient memory", "Error saving to file");
                Statusbar(NULL, "Error saving to file");
            }
            break;
        }
    }
}

static void InitTabpages(Widget work)
{   Widget notebook, page, scroller;
    Arg args[8];
    int n = 0;
    int i;
    char* labels[] = {"Filter Data","Adjust Data", "Hierarchical", "k-Means", "SOMs", "PCA"};
    char* names[] = {"FilterTab","AdjustTab", "HierarchicalTab", "KMeansTab", "SOMTab", "PCATab"};

    XtSetArg(args[n], XmNx, 10); n++;
    XtSetArg(args[n], XmNy, 180); n++;
    XtSetArg(args[n], XmNwidth, 570); n++;
    XtSetArg(args[n], XmNheight, 380); n++;
    XtSetArg(args[n], XmNbindingType, XmNONE); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNbackPagePlacement, XmTOP_RIGHT); n++;
    XtSetArg(args[n], XmNbackPageSize, 0); n++;
    notebook = XmCreateNotebook(work,"tabs",args,n);

    /* Create the "pages" */
    for (i = 0; i < 6; i++) {
        int command;
        Widget tab;
        page = XtCreateManagedWidget(names[i],xmBulletinBoardWidgetClass,notebook,NULL,0);
        switch (i) {
            case 0:
                command = ID_FILTER_INIT;
                Filter(page, (XtPointer)&command, NULL);
                break;
            case 1:
                command = ID_ADJUST_INIT;
                Adjust(page, (XtPointer)&command, NULL);
                break;
            case 2:
                command = ID_HIERARCHICAL_INIT;
                Hierarchical(page, (XtPointer)&command, NULL);
                break;
            case 3:
                command = ID_KMEANS_INIT;
                KMeans(page, (XtPointer)&command, NULL);
                break;
            case 4:
                command = ID_SOM_INIT;
                SOM(page, (XtPointer)&command, NULL);
                break;
            case 5:
                command = ID_PCA_INIT;
                PCA(page, (XtPointer)&command, NULL);
                break;
        }
        n = 0;
        XtSetArg(args[n], XmNnotebookChildType, XmMAJOR_TAB); n++;
        tab = XmCreatePushButton(notebook, labels[i], args, n);
        XtManageChild(tab);
        XtManageChild(page);
    }

    scroller = XtNameToWidget(notebook, "PageScroller");
    XtUnmanageChild(scroller);
    XtManageChild(notebook);
}

static Widget CreateMenu(Widget main_w)
/* The menu can be created more easily by using XmVaCreateSimplePulldownMenu.
 * However, XmVaCreateSimplePulldownMenu will cause client_data in the callback
 * function to be an int instead of a pointer to an int, which we use here.
 * Using an int directly may cause problems on 64-bit platforms, where the size
 * of XtPointer usually is not equal to the size of an int.
 */
{
    int n;
    Arg args[3];
    Widget pulldown;
    Widget cascade;
    Widget menuitem;
    Atom atom;

    static int open_command = CMD_FILE_OPEN;
    static int save_command = CMD_FILE_SAVE;
    static int quit_command = CMD_FILE_QUIT;
    static int htmlhelp_command = CMD_HELP_HTMLHELP;
    static int manual_command = CMD_HELP_MANUAL;
    static int download_command = CMD_HELP_DOWNLOAD;
    static int fileformat_command = CMD_HELP_FILEFORMAT;
    static int about_command = CMD_HELP_ABOUT;

    XmString file_str = XmStringCreateLocalized("File");
    XmString help_str = XmStringCreateLocalized("Help");

    Widget top = XtParent(main_w);
    Widget menubar = XmVaCreateSimpleMenuBar(main_w, "menubar", NULL, 0);

    /* File Menu */
    pulldown = XmCreatePulldownMenu(menubar, "file_menu", NULL, 0);

    n = 0;
    XtSetArg(args[n], XmNsubMenuId, pulldown); n++;
    XtSetArg(args[n], XmNlabelString, file_str); n++;
    XtSetArg(args[n], XmNmnemonic, 'F'); n++;
    cascade = XmCreateCascadeButton(menubar, "file_menu", args, n);
    XtManageChild(cascade);
    XmStringFree(file_str);

    menuitem = XtVaCreateManagedWidget("Open data file",
                                       xmPushButtonGadgetClass, 
                                       pulldown,
                                       NULL);
    XtVaSetValues(menuitem, XmNmnemonic, 'O', NULL);
    XtAddCallback(menuitem, XmNactivateCallback, MenuFile, &open_command);

    menuitem = XtVaCreateManagedWidget("Save data file",
                                       xmPushButtonGadgetClass, 
                                       pulldown,
                                       NULL);
    XtVaSetValues(menuitem, XmNmnemonic, 'S', NULL);
    XtAddCallback(menuitem, XmNactivateCallback, MenuFile, &save_command);

    XtVaCreateManagedWidget("", xmSeparatorGadgetClass, pulldown, NULL);

    menuitem = XtVaCreateManagedWidget("Quit",
                                       xmPushButtonGadgetClass, 
                                       pulldown,
                                       NULL);
    XtVaSetValues(menuitem, XmNmnemonic, 'Q', NULL);
    XtAddCallback(menuitem, XmNactivateCallback, MenuFile, &quit_command);

    /* Help Menu */
    pulldown = XmCreatePulldownMenu(menubar, "help_menu", NULL, 0);

    n = 0;
    XtSetArg(args[n], XmNsubMenuId, pulldown); n++;
    XtSetArg(args[n], XmNlabelString, help_str); n++;
    XtSetArg(args[n], XmNmnemonic, 'H'); n++;
    cascade = XmCreateCascadeButton(menubar, "help_menu", args, n);
    XtManageChild(cascade);
    XmStringFree(help_str);

    /* Tell the menubar that this is the help menu  */
    XtVaSetValues(menubar, XmNmenuHelpWidget, cascade, NULL);

    menuitem = XtVaCreateManagedWidget("Cluster 3.0 Help",
                                       xmPushButtonGadgetClass, 
                                       pulldown,
                                       NULL);
    XtVaSetValues(menuitem, XmNmnemonic, 'H', NULL);
    XtAddCallback(menuitem, XmNactivateCallback, MenuHelp, &htmlhelp_command);

    menuitem = XtVaCreateManagedWidget("Read local manual",
                                       xmPushButtonGadgetClass, 
                                       pulldown,
                                       NULL);
    XtVaSetValues(menuitem, XmNmnemonic, 'm', NULL);
    XtAddCallback(menuitem, XmNactivateCallback, MenuHelp, &manual_command);

    menuitem = XtVaCreateManagedWidget("Read online manual",
                                       xmPushButtonGadgetClass, 
                                       pulldown,
                                       NULL);
    XtVaSetValues(menuitem, XmNmnemonic, 'o', NULL);
    XtAddCallback(menuitem, XmNactivateCallback, MenuHelp, &download_command);

    menuitem = XtVaCreateManagedWidget("File format help",
                                       xmPushButtonGadgetClass, 
                                       pulldown,
                                       NULL);
    XtVaSetValues(menuitem, XmNmnemonic, 'F', NULL);
    XtAddCallback(menuitem, XmNactivateCallback, MenuHelp, &fileformat_command);

    XtVaCreateManagedWidget("", xmSeparatorGadgetClass, pulldown, NULL);

    menuitem = XtVaCreateManagedWidget("About",
                                       xmPushButtonGadgetClass, 
                                       pulldown,
                                       NULL);
    XtVaSetValues(menuitem, XmNmnemonic, 'A', NULL);
    XtAddCallback(menuitem, XmNactivateCallback, MenuHelp, &about_command);

    XtManageChild(menubar);

    atom = XmInternAtom(XtDisplay(top),"WM_DELETE_WINDOW",False);
    XmAddWMProtocolCallback(top, atom, MenuFile, (XtPointer)&quit_command);

    return menubar;
}

int guimain(int argc, char *argv[])
{   Widget work, statusbar;
    XtAppContext app;
    Arg args[2];
    int n;

    int command;

    /* Initialize toolkit */
    Widget top = XtVaAppInitialize(&app,
                                   "top",
                                   NULL,
                                   0,
                                   &argc,
                                   argv,
                                   NULL,
                                   NULL);
    Widget main_w = XtCreateWidget("main", xmMainWindowWidgetClass,top,NULL,0);
    Widget menubar = CreateMenu(main_w);

    n = 0;
    XtSetArg(args[n], XmNwidth, 570); n++;
    XtSetArg(args[n], XmNheight, 570); n++;
    work = XtCreateManagedWidget("work",xmBulletinBoardWidgetClass,main_w,args,n);

    statusbar = Statusbar(main_w, NULL);

    XtVaSetValues(top, XmNtitle, "Gene Cluster 3.0", NULL);
    XtVaSetValues(main_w,
                  XmNmenuBar, menubar,
                  XmNworkWindow, work,
                  XmNmessageWindow, statusbar, NULL);

    command = ID_FILEMANAGER_INIT;
    FileManager(work, (XtPointer)&command, NULL);
    InitTabpages(work);

    XtManageChild(main_w);

    XtRealizeWidget(top);

    XtAppMainLoop(app);
    /* If the code gets here, something is wrong => return error code */
    return 1;
}
