/*
Software and source code Copyright (C) 1998-2000 Stanford University

Written by Michael Eisen (eisen@genome.stanford.edu)

This software is copyright under the following conditions:

Permission to use, copy, and modify this software and its documentation
is hereby granted to all academic and not-for-profit institutions
without fee, provided that the above copyright notice and this permission
notice appear in all copies of the software and related documentation.
Permission to distribute the software or modified or extended versions
thereof on a not-for-profit basis is explicitly granted, under the above
conditions. However, the right to use this software in conjunction with
for profit activities, and the right to distribute the software or modified or
extended versions thereof for profit are *NOT* granted except by prior
arrangement and written consent of the copyright holders.

Use of this source code constitutes an agreement not to criticize, in any
way, the code-writing style of the author, including any statements regarding
the extent of documentation and comments present.

The software is provided "AS-IS" and without warranty of ank kind, express,
implied or otherwise, including without limitation, any warranty of
merchantability or fitness for a particular purpose.

In no event shall Stanford University or the authors be liable for any special,
incudental, indirect or consequential damages of any kind, or any damages
whatsoever resulting from loss of use, data or profits, whether or not
advised of the possibility of damage, and on any theory of liability,
arising out of or in connection with the use or performance of this software.

This code was written using Borland C++ Builder 4 (Inprise Inc.,
www.inprise.com) and may be subject to certain additional restrictions as a
result.
*/

/*
This program was modified by Michiel de Hoon of the University of Tokyo,
Human Genome Center (currently at the RIKEN Genomic Sciences Center;
mdehoon 'AT' gsc.riken.jp). The core numerical routines are now located in the
C clustering library; this program mainly constains gui-related routines.
Instead of the Borland C++ Builder, the GNU C compiler under Cygwin/MinGW
was used.
MdH 2002.06.13.
*/

#ifdef UNICODE
#define _UNICODE
#endif

/*============================================================================*/
/* Header files                                                               */
/*============================================================================*/

/* Standard C header files */
#include <stdio.h>
#include <math.h>

/* Standard Windows header files */
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

/* Local header files */
#include "resources.h" /* Defines dialog windows, menubar, icon, and bitmap */
#include "data.h"      /* Includes data handling and file reading/writing */
#include "cluster.h"   /* The C clustering library */


/*============================================================================*/
/* GUI utilities                                                              */
/*============================================================================*/

static
double GetDlgItemDouble(HWND hDlg, int nlDDlgItem, BOOL* lpTranslated)
{ TCHAR szBuffer[256];
  TCHAR* endptr;
  if(GetDlgItemText(hDlg, nlDDlgItem, szBuffer, sizeof(szBuffer)))
  { const double dValue = _tcstod(szBuffer, &endptr);
    if (lpTranslated) *lpTranslated = (*endptr=='\0');
    return dValue;
  }
  else
  { if (lpTranslated) *lpTranslated=FALSE;
    return 0;
  }
}

static
BOOL SetDlgItemDouble (HWND hwndDlg, int idControl, double dValue)
{ TCHAR szBuffer[32];
  /* Note: wsprintf does not handle floating point values. */
#ifdef UNICODE
  swprintf (szBuffer, TEXT("%4g"), dValue);
#else
  sprintf (szBuffer, "%4g", dValue);
#endif
  return SetDlgItemText (hwndDlg, idControl, szBuffer);
}

static
void SetMetrics(HWND hWnd, char initial)
{ SendMessage(hWnd,
              CB_ADDSTRING,
              0,
              (LPARAM)TEXT("Correlation (uncentered)"));
  SendMessage(hWnd,
              CB_ADDSTRING,
              0,
              (LPARAM)TEXT("Correlation (centered)"));
  SendMessage(hWnd,
              CB_ADDSTRING,
              0,
              (LPARAM)TEXT("Absolute Correlation (uncentered)"));
  SendMessage(hWnd,
              CB_ADDSTRING,
              0,
              (LPARAM)TEXT("Absolute Correlation (centered)"));
  SendMessage(hWnd,
              CB_ADDSTRING,
              0,
              (LPARAM)TEXT("Spearman Rank Correlation"));
  SendMessage(hWnd,
              CB_ADDSTRING,
              0,
              (LPARAM)TEXT("Kendall's tau"));
  SendMessage(hWnd,
              CB_ADDSTRING,
              0,
              (LPARAM)TEXT("Euclidean distance"));
  SendMessage(hWnd,
              CB_ADDSTRING,
              0,
              (LPARAM)TEXT("City-block distance"));
  switch (initial)
  { case 'u': SendMessage(hWnd, CB_SETCURSEL, 0, 0); break;
    case 'c': SendMessage(hWnd, CB_SETCURSEL, 1, 0); break;
    case 'a': SendMessage(hWnd, CB_SETCURSEL, 2, 0); break;
    case 'x': SendMessage(hWnd, CB_SETCURSEL, 3, 0); break;
    case 's': SendMessage(hWnd, CB_SETCURSEL, 4, 0); break;
    case 'k': SendMessage(hWnd, CB_SETCURSEL, 5, 0); break;
    case 'e': SendMessage(hWnd, CB_SETCURSEL, 6, 0); break;
    case 'b': SendMessage(hWnd, CB_SETCURSEL, 7, 0); break;
    default : SendMessage(hWnd, CB_SETCURSEL, 0, 0); break;
  }
  return;
}

static
char GetMetric(HWND hWnd)
{ int index = SendMessage (hWnd, CB_GETCURSEL, 0, 0);
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

/*============================================================================*/
/* Callback functions --- Tab pages                                           */
/*============================================================================*/

BOOL CALLBACK
FilterDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{ static BOOL* use = NULL;
  static int useRows;
  switch (nMsg)
  { case WM_INITDIALOG:
    { use = NULL;
      return TRUE;
    }
    case WM_COMMAND:
    { if (HIWORD(wParam)==BN_CLICKED)
      { switch ((int) LOWORD(wParam))
        { case ID_FILTER_APPLY:
          /* Filter data. Apply user selected criteria to flag (for subsequent
           * removal) rows that fail to pass tests. Note that filters are
           * assessed here and applied separately so the user can adjust
           * parameters to get appropriate number of rows passing
           */
          { TCHAR buffer[256];
            HWND hTabCtrl = GetParent(hWnd);
            HWND hWndMain = GetParent(hTabCtrl);

            const int Rows = GetRows();

            const BOOL bStd =
              IsDlgButtonChecked (hWnd, ID_FILTER_STD_XB);
            const BOOL bPercent =
              IsDlgButtonChecked (hWnd, ID_FILTER_PERCENT_XB);
            const BOOL bAbsVal =
              IsDlgButtonChecked (hWnd,ID_FILTER_OBSERVATION_XB);
            const BOOL bMaxMin =
              IsDlgButtonChecked (hWnd, ID_FILTER_MAXMIN_XB);

            double value;
            double absVal;
            double percent;
            double std;
            int numberAbs;
            double maxmin;
            int intvalue;
            BOOL error;

            int Row;

            /* Read information from the edit boxes */
            value = GetDlgItemDouble(hWnd, ID_FILTER_OBSERVATION_VALUE, &error);
            absVal = error ? value : 0.0;
            value = GetDlgItemDouble(hWnd, ID_FILTER_PERCENT, &error);
            percent = error ? value : 0.0;
            value = GetDlgItemDouble (hWnd, ID_FILTER_STD, &error);
            std = error ? value : 0.0;
            intvalue = GetDlgItemInt (hWnd, ID_FILTER_NUMBER, &error, FALSE);
            numberAbs = error ? intvalue : 0;
            value = GetDlgItemDouble (hWnd, ID_FILTER_MAXMIN, &error);
            maxmin = error ? value : 0.0;

	    SendMessage(hWnd, IDM_RESET, 0, 0);
            /* Store results in boolean use */
            if(use) free(use);
            use = malloc(Rows*sizeof(BOOL));
            useRows = 0;

            for (Row = 0; Row < Rows; Row++)
            { wsprintf (buffer, TEXT("Assessing filters for gene %d"), Row);
              SendMessage (hWndMain,IDM_SETSTATUSBAR,(WPARAM)buffer,0);
              use[Row] = FilterRow (Row,bStd,bPercent,bAbsVal,bMaxMin,
                                          absVal,percent,std,numberAbs,maxmin);
              /* Count how many passed */
              if (use[Row]) useRows++;
            }

            /* Tell user how many rows passed */
            wsprintf (buffer, TEXT("%d passed out of %d"), useRows, Rows);
            SetDlgItemText (hWnd, ID_FILTER_RESULT, buffer);

            EnableWindow (GetDlgItem(hWnd,ID_FILTER_ACCEPT), TRUE);
            SendMessage(hWndMain,
                        IDM_SETSTATUSBAR,
                        (WPARAM)TEXT("Done Analyzing Filters"),
                        0);
            return TRUE;
          }
          case ID_FILTER_ACCEPT:
          /* Accept results of last filtering */
          { HWND hTabCtrl = GetParent(hWnd);
            HWND hWndMain = GetParent(hTabCtrl);
            EnableWindow (GetDlgItem(hWnd,ID_FILTER_ACCEPT), FALSE);
            SelectSubset(useRows, use);
            SendMessage(hWndMain,IDM_UPDATEINFO,0,0);
            return TRUE;
          }
        }
      }
      break;
    }
    case IDM_RESET:
    { SetDlgItemText (hWnd, ID_FILTER_RESULT, TEXT(""));
      EnableWindow (GetDlgItem(hWnd,ID_FILTER_ACCEPT), FALSE);
      return TRUE;
    }
    case WM_DESTROY:
    { if(use) free(use);
      return TRUE;
    }
  }
  return FALSE;
}

BOOL CALLBACK
AdjustDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
/* Adjust data values in various ways */
{ switch (nMsg)
  { case WM_INITDIALOG:
    { HWND GeneMeanButton = GetDlgItem(hWnd, ID_ADJUST_MEAN_GENES);
      HWND GeneMedianButton = GetDlgItem(hWnd, ID_ADJUST_MEDIAN_GENES);
      HWND ArrayMeanButton = GetDlgItem(hWnd, ID_ADJUST_MEAN_ARRAYS);
      HWND ArrayMedianButton = GetDlgItem(hWnd, ID_ADJUST_MEDIAN_ARRAYS);
      CheckDlgButton(hWnd,ID_ADJUST_MEAN_GENES,1);
      CheckDlgButton(hWnd,ID_ADJUST_MEAN_ARRAYS,1);
      EnableWindow(GeneMeanButton, FALSE);
      EnableWindow(GeneMedianButton, FALSE);
      EnableWindow(ArrayMeanButton, FALSE);
      EnableWindow(ArrayMedianButton, FALSE);
      return TRUE;
    }
    case WM_COMMAND:
    { if (HIWORD(wParam)==BN_CLICKED)
      { switch ((int) LOWORD(wParam))
        { case ID_ADJUST_CENTER_GENES_XB:
          { UINT state = IsDlgButtonChecked(hWnd, ID_ADJUST_CENTER_GENES_XB);
            int flag = (state==BST_CHECKED);
            HWND GeneMeanButton = GetDlgItem(hWnd, ID_ADJUST_MEAN_GENES);
            HWND GeneMedianButton = GetDlgItem(hWnd, ID_ADJUST_MEDIAN_GENES);
            EnableWindow(GeneMeanButton, flag);
            EnableWindow(GeneMedianButton, flag);
            return TRUE;
          }
          case ID_ADJUST_CENTER_ARRAYS_XB:
          { UINT state = IsDlgButtonChecked(hWnd, ID_ADJUST_CENTER_ARRAYS_XB);
            int flag = (state==BST_CHECKED);
            HWND ArrayMeanButton = GetDlgItem(hWnd, ID_ADJUST_MEAN_ARRAYS);
            HWND ArrayMedianButton = GetDlgItem(hWnd, ID_ADJUST_MEDIAN_ARRAYS);
            EnableWindow(ArrayMeanButton, flag);
            EnableWindow(ArrayMedianButton, flag);
            return TRUE;
          }
          case ID_ADJUST_APPLY:
          { HWND hTabCtrl = GetParent(hWnd);
            HWND hWndMain = GetParent(hTabCtrl);
            BOOL bLogTransform;
            BOOL GeneMeanCenter = FALSE;
            BOOL GeneMedianCenter = FALSE;
            BOOL GeneNormalize;
            BOOL ArrayMeanCenter = FALSE;
            BOOL ArrayMedianCenter = FALSE;
            BOOL ArrayNormalize;

            SendMessage(hWndMain,
                        IDM_SETSTATUSBAR,
                        (WPARAM)TEXT("Adjusting data"),
                        0);

            bLogTransform = IsDlgButtonChecked (hWnd, ID_ADJUST_LOG_XB);
            if (bLogTransform) LogTransform();

            if(IsDlgButtonChecked(hWnd, ID_ADJUST_CENTER_GENES_XB))
            { GeneMeanCenter =
                IsDlgButtonChecked(hWnd, ID_ADJUST_MEAN_GENES);
              GeneMedianCenter =
                IsDlgButtonChecked(hWnd, ID_ADJUST_MEDIAN_GENES);
            }
            GeneNormalize = IsDlgButtonChecked(hWnd,ID_ADJUST_NORMALIZE_GENES);
            AdjustGenes(GeneMeanCenter, GeneMedianCenter, GeneNormalize);

            if(IsDlgButtonChecked(hWnd, ID_ADJUST_CENTER_ARRAYS_XB))
            { ArrayMeanCenter =
                IsDlgButtonChecked(hWnd,ID_ADJUST_MEAN_ARRAYS);
              ArrayMedianCenter =
                IsDlgButtonChecked(hWnd,ID_ADJUST_MEDIAN_ARRAYS);
            }
            ArrayNormalize =IsDlgButtonChecked(hWnd,ID_ADJUST_NORMALIZE_ARRAYS);
            AdjustArrays(ArrayMeanCenter,ArrayMedianCenter,ArrayNormalize);

            SendMessage(hWndMain,
                        IDM_SETSTATUSBAR,
                        (WPARAM)TEXT("Done adjusting data"),
                        0);
            return TRUE;
          }
        }
      }
    }
  }
  return 0;
}

BOOL CALLBACK
HierarchicalDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{ static HWND hWndGeneWeight;
  static HWND hWndArrayWeight;
  switch (nMsg)
  { case WM_INITDIALOG:
    { /* First assign the default values for the weights */
      double GeneWeightExp = 1.0;
      double ArrayWeightExp = 1.0;
      double GeneWeightCutoff = 0.9;
      double ArrayWeightCutoff = 0.9;
      /* Try to read saved values from the registry */
      HKEY hKey;
      LONG lRet;
      lRet = RegOpenKeyEx(HKEY_CURRENT_USER,
                          TEXT("Software\\Stanford\\Cluster\\WeightSettings"),
                          0,
                          KEY_QUERY_VALUE,
                          &hKey);
      if (lRet==ERROR_SUCCESS)
      { DWORD dwSize = sizeof(double);
        DWORD dwType = REG_BINARY;
        double value;
        lRet = RegQueryValueEx(hKey,
                               TEXT("GeneWeightCutoff"),
                               NULL,
                               &dwType,
                               (LPBYTE) &value,
                               &dwSize);
        if(lRet == ERROR_SUCCESS) GeneWeightCutoff = value;
        lRet = RegQueryValueEx(hKey,
                               TEXT("GeneWeightExp"),
                               NULL,
                               &dwType,
                               (LPBYTE) &value,
                               &dwSize);
        if(lRet == ERROR_SUCCESS) GeneWeightExp = value;
        lRet = RegQueryValueEx(hKey,
                               TEXT("ArrayWeightCutoff"),
                               NULL,
                               &dwType,
                               (LPBYTE) &value,
                               &dwSize);
        if(lRet == ERROR_SUCCESS) ArrayWeightCutoff = value;
        lRet = RegQueryValueEx(hKey,
                               TEXT("ArrayWeightExp"),
                               NULL,
                               &dwType,
                               (LPBYTE) &value,
                               &dwSize);
        if(lRet == ERROR_SUCCESS) ArrayWeightExp = value;
      }
      RegCloseKey(hKey);
      /* Create weights window */
      hWndGeneWeight =
        CreateDialog(0, MAKEINTRESOURCE(ID_HIERARCHICAL_GENE_WEIGHT), hWnd, 0);
      hWndArrayWeight =
        CreateDialog(0, MAKEINTRESOURCE(ID_HIERARCHICAL_ARRAY_WEIGHT), hWnd, 0);
      /* Set the weights that we found */
      SetDlgItemDouble
        (hWndGeneWeight,ID_HIERARCHICAL_GENE_EXP,GeneWeightExp);
      SetDlgItemDouble
        (hWndArrayWeight,ID_HIERARCHICAL_ARRAY_EXP,ArrayWeightExp);
      SetDlgItemDouble
        (hWndGeneWeight,ID_HIERARCHICAL_GENE_CUTOFF,GeneWeightCutoff);
      SetDlgItemDouble
        (hWndArrayWeight,ID_HIERARCHICAL_ARRAY_CUTOFF,ArrayWeightCutoff);
      /* Set up metric combo boxes */
      SetMetrics (GetDlgItem(hWnd,ID_HIERARCHICAL_GENE_METRIC), 'u');
      SetMetrics (GetDlgItem(hWnd,ID_HIERARCHICAL_ARRAY_METRIC), 'u');
      return TRUE;
    }
    case WM_COMMAND:
    { if (HIWORD(wParam)==BN_CLICKED)
      { switch ((int) LOWORD(wParam))
        { case ID_HIERARCHICAL_CENTROID:
	  { char c = 'c';
	    SendMessage(hWnd,IDM_HIERARCHICAL_EXECUTE,(WPARAM)&c,0);
            return TRUE;
          }
          case ID_HIERARCHICAL_SINGLE:
	  { char c = 's';
	    SendMessage(hWnd,IDM_HIERARCHICAL_EXECUTE,(WPARAM)&c,0);
            return TRUE;
          }
          case ID_HIERARCHICAL_COMPLETE:
	  { char c = 'm';
	    SendMessage(hWnd,IDM_HIERARCHICAL_EXECUTE,(WPARAM)&c,0);
            return TRUE;
          }
          case ID_HIERARCHICAL_AVERAGE:
	  { char c = 'a';
	    SendMessage(hWnd,IDM_HIERARCHICAL_EXECUTE,(WPARAM)&c,0);
            return TRUE;
          }
          case ID_HIERARCHICAL_GENE_WEIGHT_XB:
          { if (IsDlgButtonChecked (hWnd, ID_HIERARCHICAL_GENE_WEIGHT_XB))
              ShowWindow (hWndArrayWeight, SW_SHOWNORMAL);
            else
              ShowWindow (hWndArrayWeight, SW_HIDE);
            return TRUE;
          }
          case ID_HIERARCHICAL_ARRAY_WEIGHT_XB:
          { if (IsDlgButtonChecked (hWnd, ID_HIERARCHICAL_ARRAY_WEIGHT_XB))
              ShowWindow (hWndGeneWeight, SW_SHOWNORMAL);
            else
              ShowWindow (hWndGeneWeight, SW_HIDE);
            return TRUE;
          }
        }
      }
      break;
    }
    case WM_DESTROY:
    { double GeneWeightExp = 1;
      double ArrayWeightExp = 1;
      double GeneWeightCutoff = 0.1;
      double ArrayWeightCutoff = 0.1;
      BOOL error;
      double value;
      HKEY hKey;
      LONG lRet;

      value =
        GetDlgItemDouble(hWndGeneWeight,ID_HIERARCHICAL_GENE_EXP,&error);
      if (!error) GeneWeightExp = value;
      value =
        GetDlgItemDouble(hWndArrayWeight,ID_HIERARCHICAL_ARRAY_EXP,&error);
      if (!error) ArrayWeightExp = value;
      value =
        GetDlgItemDouble(hWndGeneWeight,ID_HIERARCHICAL_GENE_CUTOFF,&error);
      if (!error) GeneWeightCutoff = value;
      value =
        GetDlgItemDouble(hWndArrayWeight,ID_HIERARCHICAL_ARRAY_CUTOFF,&error);
      if (!error) ArrayWeightCutoff = value;
      /* Write weight information to the registry */
      lRet = RegOpenKeyEx(HKEY_CURRENT_USER,
                          TEXT("Software\\Stanford\\Cluster\\WeightSettings"),
                          0,
                          KEY_SET_VALUE,
                          &hKey);
      if (lRet == ERROR_SUCCESS)
      { const DWORD dwSize = sizeof(double);
        RegSetValueEx(hKey,
                      TEXT("GeneWeightCutOff"),
                      0,
                      REG_BINARY,
                      (LPBYTE) &GeneWeightCutoff,
                      dwSize);
        RegSetValueEx(hKey,
                      TEXT("GeneWeightExp"),
                      0,
                      REG_BINARY,
                      (LPBYTE) &GeneWeightExp,
                      dwSize);
        RegSetValueEx(hKey,
                      TEXT("ArrayWeightCutOff"),
                      0,
                      REG_BINARY,
                      (LPBYTE) &ArrayWeightCutoff,
                      dwSize);
        RegSetValueEx(hKey,
                      TEXT("ArrayWeightExp"),
                      0,
                      REG_BINARY,
                      (LPBYTE) &ArrayWeightExp,
                      dwSize);
      }
      RegCloseKey(hKey);
      return TRUE;
    }
    case IDM_HIERARCHICAL_EXECUTE:
    { char method = *((char*)wParam);
      const int Rows = GetRows();
      const int Columns = GetColumns();
      HWND hTabCtrl = GetParent(hWnd);
      HWND hWndMain = GetParent(hTabCtrl);

      BOOL ClusterGenes;
      BOOL ClusterArrays;
      BOOL bCalculateGeneWeights;
      BOOL bCalculateArrayWeights;
      char genemetric;
      char arraymetric;

      FILE* outputfile;
      TCHAR* jobname;
      TCHAR filename[MAX_PATH];
      TCHAR* filetag;

      if (Rows==0 || Columns==0)
      { SendMessage(hWndMain,
                    IDM_SETSTATUSBAR,
                    (WPARAM)TEXT("No data available"),
                    0);
        return TRUE;
      }

      ClusterGenes = IsDlgButtonChecked (hWnd, ID_HIERARCHICAL_GENE_XB);
      ClusterArrays = IsDlgButtonChecked (hWnd, ID_HIERARCHICAL_ARRAY_XB);

      /* Check if we are really clustering anything here */
      if(!ClusterGenes && !ClusterArrays) return TRUE;

      bCalculateGeneWeights =
        IsDlgButtonChecked (hWnd, ID_HIERARCHICAL_GENE_WEIGHT_XB);
      bCalculateArrayWeights =
        IsDlgButtonChecked (hWnd, ID_HIERARCHICAL_ARRAY_WEIGHT_XB);

      /* Find out which metrics to use */
      genemetric = GetMetric(GetDlgItem(hWnd, ID_HIERARCHICAL_GENE_METRIC));
      arraymetric = GetMetric(GetDlgItem(hWnd, ID_HIERARCHICAL_ARRAY_METRIC));

      if (bCalculateGeneWeights || bCalculateArrayWeights)
      { const char* error;
        double GeneCutoff = 0.0;
        double GeneExponent = 0.0;
        double ArrayCutoff = 0.0;
        double ArrayExponent = 0.0;
        SendMessage(hWndMain,
          IDM_SETSTATUSBAR,
          (WPARAM)TEXT("Calculating weights"),
          0);
        if (bCalculateGeneWeights)
        { ArrayCutoff = GetDlgItemDouble(hWndArrayWeight,
                                         ID_HIERARCHICAL_ARRAY_CUTOFF, NULL);
          ArrayExponent = GetDlgItemDouble(hWndArrayWeight,
                                         ID_HIERARCHICAL_ARRAY_EXP, NULL);
        }
        if (bCalculateArrayWeights)
        { GeneCutoff = GetDlgItemDouble(hWndGeneWeight,
                                        ID_HIERARCHICAL_GENE_CUTOFF, NULL);
          GeneExponent = GetDlgItemDouble(hWndGeneWeight,
                                         ID_HIERARCHICAL_GENE_EXP, NULL);
        }
        error = CalculateWeights(GeneCutoff, GeneExponent, genemetric,
                                 ArrayCutoff, ArrayExponent, arraymetric);
        if (error)
        {
#ifdef UNICODE
          TCHAR buffer[256];
          MultiByteToWideChar(CP_ACP, 0, error, -1, buffer, 256);
          MessageBox (NULL, buffer, TEXT("Error"), MB_OK);
#else
          MessageBox (NULL, error, TEXT("Error"), MB_OK);
#endif
          return TRUE;
        }
      }

      SendMessage(hWndMain, IDM_GETJOBNAME, (WPARAM)&jobname, 0);
      filetag = filename + wsprintf(filename, jobname);
      free(jobname);

      switch(method)
      { case 'c':
          SendMessage(hWndMain,
            IDM_SETSTATUSBAR,
            (WPARAM)TEXT("Performing centroid linkage hierarchical clustering"),
            0);
          break;
        case 's':
          SendMessage(hWndMain,
            IDM_SETSTATUSBAR,
            (WPARAM)TEXT("Performing single linkage hierarchical clustering"),
            0);
          break;
        case 'm':
          SendMessage(hWndMain,
            IDM_SETSTATUSBAR,
            (WPARAM)TEXT("Performing complete linkage hierarchical clustering"),
            0);
          break;
        case 'a':
          SendMessage(hWndMain,
            IDM_SETSTATUSBAR,
            (WPARAM)TEXT("Performing average linkage hierarchical clustering"),
            0);
          break;
        default:
          SendMessage(hWndMain,
            IDM_SETSTATUSBAR,
            (WPARAM)TEXT("ERROR: Unknown clustering method"),
            0);
          /* Never get here */
          return TRUE;
      }

      /* Find out what we need to do here */
      if (ClusterGenes)
      { int result;
        wsprintf (filetag, TEXT(".gtr"));
        outputfile = _tfopen(filename, TEXT("wt"));
        if (!outputfile)
        { MessageBox(NULL,
                     TEXT("Error: Unable to open the output file"),
                     TEXT("Error"),
                     0);
          return TRUE;
        }
        result = HierarchicalCluster (outputfile, genemetric, FALSE, method);
        fclose(outputfile);
        if (!result)
        { MessageBox(NULL,
                     TEXT("Error: Insufficient memory"),
                     TEXT("Error"),
                     0);
          return TRUE;
        }
      }

      if (ClusterArrays)
      { int result;
        wsprintf(filetag, TEXT(".atr"));
        outputfile = _tfopen(filename, TEXT("wt"));
        if (!outputfile)
        { MessageBox(NULL,
                     TEXT("Error: Unable to open the output file"),
                     TEXT("Error"),
                     0);
          return TRUE;
        }
        result = HierarchicalCluster(outputfile, arraymetric, TRUE, method);
        fclose(outputfile);
        if (!result)
        { MessageBox(NULL,
                     TEXT("Error: Insufficient memory"),
                     TEXT("Error"),
                     0);
          return TRUE;
        }
      }

      SendMessage(hWndMain,
        IDM_SETSTATUSBAR,
        (WPARAM)TEXT("Saving the clustering result"),
        0);

      /* Now make output .cdt file */
      wsprintf(filetag, TEXT(".cdt"));
      outputfile = _tfopen(filename, TEXT("wt"));
      if (!outputfile)
      { MessageBox(NULL,
                   TEXT("Error: Unable to open the output file"),
                   TEXT("Error"),
                   0);
        return TRUE;
      }
      Save(outputfile, ClusterGenes, ClusterArrays);
      fclose(outputfile);
      SendMessage(hWndMain,IDM_SETSTATUSBAR,(WPARAM)TEXT("Done Clustering"),0);
      return TRUE;
    }
  }
  return FALSE;
}

BOOL CALLBACK
KmeansDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{ switch (nMsg)
  { case WM_INITDIALOG:
    { /* Set up metric combo boxes */
      SetMetrics (GetDlgItem(hWnd,ID_KMEANS_GENE_METRIC), 'e');
      SetMetrics (GetDlgItem(hWnd,ID_KMEANS_ARRAY_METRIC), 'e');
      CheckDlgButton(hWnd,ID_KMEANS_GENE_MEAN,1);
      CheckDlgButton(hWnd,ID_KMEANS_ARRAY_MEAN,1);
      return TRUE;
    }
    case WM_COMMAND:
    { if (HIWORD(wParam)==BN_CLICKED)
      { switch ((int) LOWORD(wParam))
        { case ID_KMEANS_BUTTON:
          { const int Rows = GetRows();
            const int Columns = GetColumns();
            HWND hTabCtrl = GetParent(hWnd);
            HWND hWndMain = GetParent(hTabCtrl);
            TCHAR* jobname;
            TCHAR filename[MAX_PATH];
            TCHAR* filetag;
            FILE* outputfile;
            /* One for the terminating \0; to be increased below */

            BOOL ClusterGenes;
            BOOL ClusterArrays;

            int kGenes = 0;
            int kArrays = 0;
      
            if (Rows==0 || Columns==0)
            { SendMessage(hWndMain,
                          IDM_SETSTATUSBAR,
                          (WPARAM)TEXT("No data available"),
                          0);
              return TRUE;
            }

            SendMessage(hWndMain,IDM_GETJOBNAME,(WPARAM)&jobname,0);
            filetag = filename + wsprintf(filename, jobname);
            free(jobname);
      
            ClusterGenes = IsDlgButtonChecked (hWnd, ID_KMEANS_GENE_XB);
            ClusterArrays = IsDlgButtonChecked (hWnd, ID_KMEANS_ARRAY_XB);
            if (!ClusterGenes && !ClusterArrays) return TRUE;
            /* Nothing to do */
                
            SendMessage(hWndMain,
                        IDM_SETSTATUSBAR,
                        (WPARAM)TEXT("Executing k-means clustering"),
                        0);

            if (ClusterGenes)
            { char method;
              char dist;
              int* NodeMap;
              int nGeneTrials;
              int ifound;
              TCHAR buffer[256];

              kGenes = GetDlgItemInt (hWnd,ID_KMEANS_GENE_K,0,FALSE);
              if (kGenes==0)
              { SendMessage(hWndMain,
                          IDM_SETSTATUSBAR,
                          (WPARAM)TEXT("Choose a nonzero number of clusters"),
                          0);
                return TRUE;
              }
              if (Rows < kGenes)
              { SendMessage(hWndMain,
                          IDM_SETSTATUSBAR,
                          (WPARAM)TEXT("More clusters than genes available"),
                          0);
                return TRUE;
              }
    
              method = IsDlgButtonChecked(hWnd,ID_KMEANS_GENE_MEAN) ? 'a' : 'm';
              /* 'a' is average, 'm' is median */
          
              dist = GetMetric (GetDlgItem(hWnd,ID_KMEANS_GENE_METRIC));
              NodeMap = malloc(Rows*sizeof(int));
              nGeneTrials = GetDlgItemInt (hWnd,ID_KMEANS_GENE_RUNS,0,FALSE);
              ifound = GeneKCluster(kGenes, nGeneTrials, method, dist, NodeMap);
              wsprintf(buffer, TEXT("Solution was found %d times"), ifound);
              SendMessage(hWndMain,
                          IDM_SETSTATUSBAR,
                          (WPARAM)buffer,
                          0);

              wsprintf (filetag, TEXT("_K_G%d.kgg"), kGenes);
              outputfile = _tfopen(filename, TEXT("wt"));
              if (!outputfile)
              { MessageBox(NULL,
                           TEXT("Error: Unable to open the output file"),
                           TEXT("Error"),
                           0);
                free(NodeMap);
                return TRUE;
              }

              SaveGeneKCluster(outputfile, kGenes, NodeMap);
              fclose(outputfile);
              free(NodeMap);
            }

            if (ClusterArrays)
            { char method;
              char dist;
              int nArrayTrials;
              int* NodeMap;
              int ifound;
              TCHAR buffer[256];

              kArrays = GetDlgItemInt (hWnd,ID_KMEANS_ARRAY_K,0,FALSE);
              if (kArrays==0)
              { SendMessage(hWndMain,
                          IDM_SETSTATUSBAR,
                          (WPARAM)TEXT("Choose a nonzero number of clusters"),
                          0);
                return TRUE;
              }
              if (Columns < kArrays)
              { SendMessage(hWndMain,
                     IDM_SETSTATUSBAR,
                     (WPARAM)TEXT("More clusters than experiments available"),
                     0);
                return TRUE;
              }
          
              method =IsDlgButtonChecked(hWnd,ID_KMEANS_ARRAY_MEAN) ? 'a' : 'm';
              /* 'a' is average, 'm' is median */
              dist = GetMetric (GetDlgItem(hWnd,ID_KMEANS_ARRAY_METRIC));
              nArrayTrials = GetDlgItemInt(hWnd,ID_KMEANS_ARRAY_RUNS,0,FALSE);
      
              NodeMap = malloc(Columns*sizeof(int));
              ifound = ArrayKCluster (kArrays,nArrayTrials,method,dist,NodeMap);
              wsprintf(buffer, TEXT("Solution was found %d times"), ifound);
              SendMessage(hWndMain, IDM_SETSTATUSBAR, (WPARAM)buffer, 0);
              wsprintf (filetag, TEXT("_K_A%d.kag"), kArrays);
              outputfile = _tfopen(filename, TEXT("wt"));
              if (!outputfile)
              { MessageBox(NULL,
                           TEXT("Error: Unable to open the output file"),
                           TEXT("Error"),
                           0);
                free(NodeMap);
                return TRUE;
              }
              SaveArrayKCluster(outputfile, kArrays, NodeMap);
              fclose(outputfile);
              free(NodeMap);
            }

            /* Now write the data file */
            if (ClusterGenes && ClusterArrays)
              wsprintf (filetag, TEXT("_K_G%d_A%d.CDT"), kGenes, kArrays);
            else if (ClusterGenes)
              wsprintf (filetag, TEXT("_K_G%d.CDT"), kGenes);
            else if (ClusterArrays)
              wsprintf (filetag, TEXT("_K_A%d.CDT"), kArrays);
            outputfile = _tfopen(filename, TEXT("wt"));
            if (!outputfile)
            { MessageBox(NULL,
                         TEXT("Error: Unable to open the output file"),
                         TEXT("Error"),
                         0);
              return TRUE;
            }
            Save(outputfile,0,0);
            fclose (outputfile);
            return TRUE;
          }
        }
      }
    }
  }
  return FALSE;
}

BOOL CALLBACK
SOMDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{ switch (nMsg)
  { case WM_INITDIALOG:
    { SetMetrics (GetDlgItem(hWnd,ID_SOM_GENE_METRIC), 'e');
      SetMetrics (GetDlgItem(hWnd,ID_SOM_ARRAY_METRIC), 'e');
      return TRUE;
    }
    case IDM_UPDATEINFO:
    { int Rows = *((int*)wParam);
      int Columns = *((int*)lParam);
      int dim = 1 + (int)pow(Rows,0.25);
      SetDlgItemInt (hWnd, ID_SOM_GENE_XDIM, dim, FALSE);
      SetDlgItemInt (hWnd, ID_SOM_GENE_YDIM, dim, FALSE);
      dim = 1 + (int)pow(Columns,0.25);
      SetDlgItemInt (hWnd, ID_SOM_ARRAY_XDIM, dim, FALSE);
      SetDlgItemInt (hWnd, ID_SOM_ARRAY_YDIM, dim, FALSE);
      return TRUE;
    }
    case WM_COMMAND:
    { if (HIWORD(wParam)==BN_CLICKED)
      { switch ((int) LOWORD(wParam))
        { case ID_SOM_BUTTON:
          { const int Rows = GetRows();
            const int Columns = GetColumns();
            HWND hTabCtrl = GetParent(hWnd);
            HWND hWndMain = GetParent(hTabCtrl);
            TCHAR* jobname;
            TCHAR* filetag;
            TCHAR filename[MAX_PATH];
            FILE* GeneFile = NULL;
            FILE* ArrayFile = NULL;
            FILE* DataFile = NULL;

            const BOOL ClusterGenes = IsDlgButtonChecked(hWnd,ID_SOM_GENE_XB);
            const BOOL ClusterArrays = IsDlgButtonChecked(hWnd,ID_SOM_ARRAY_XB);
            
            const int GeneXDim = GetDlgItemInt(hWnd,ID_SOM_GENE_XDIM,0,FALSE);
            const int GeneYDim = GetDlgItemInt(hWnd,ID_SOM_GENE_YDIM,0,FALSE);
            const int ArrayXDim = GetDlgItemInt(hWnd,ID_SOM_ARRAY_XDIM,0,FALSE);
            const int ArrayYDim = GetDlgItemInt(hWnd,ID_SOM_ARRAY_YDIM,0,FALSE);

            const int GeneIters = ClusterGenes ?
              GetDlgItemInt(hWnd,ID_SOM_GENE_ITERS,0,FALSE) : 0;
            const double GeneTau =
              GetDlgItemDouble (hWnd,ID_SOM_GENE_TAU,0);
            const char GeneMetric =
              GetMetric(GetDlgItem(hWnd,ID_SOM_GENE_METRIC));
            const int ArrayIters = ClusterArrays ?
              GetDlgItemInt(hWnd,ID_SOM_ARRAY_ITERS,0,FALSE) : 0;
            const double ArrayTau =
              GetDlgItemDouble (hWnd,ID_SOM_ARRAY_TAU,0);
            const char ArrayMetric =
              GetMetric(GetDlgItem(hWnd,ID_SOM_ARRAY_METRIC));

            if (!ClusterGenes && !ClusterArrays) return TRUE;
            /* Nothing to do here */

            if (Rows==0 || Columns==0)
            { SendMessage (hWndMain,
                           IDM_SETSTATUSBAR,
                           (WPARAM)TEXT("No data available"),
                           0);
              return TRUE;
            }

            SendMessage(hWndMain,IDM_GETJOBNAME,(WPARAM)&jobname,0);
            filetag = filename + wsprintf(filename, TEXT("%s_SOM"), jobname);
            free(jobname);
            
            SendMessage (hWndMain,
                         IDM_SETSTATUSBAR,
                         (WPARAM)TEXT("Calculating Self-Organizing Map"),
                         0);
 
            if (ClusterGenes)
              filetag += wsprintf(filetag,TEXT("_G%d-%d"),GeneXDim,GeneYDim);
            if (ClusterArrays)
              filetag += wsprintf(filetag,TEXT("_A%d-%d"),ArrayXDim,ArrayYDim);

            wsprintf (filetag, TEXT(".TXT"));
            DataFile = _tfopen(filename, TEXT("wt"));
            if (!DataFile)
            { MessageBox(NULL,
                         TEXT("Error: Unable to open the output file"),
                         TEXT("Error"),
                         0);
              return TRUE;
            }

            if (ClusterGenes)
            { if((GeneIters==0)||(GeneTau==0)||(GeneXDim==0)||(GeneYDim==0))
              { SendMessage(hWndMain,
                            IDM_SETSTATUSBAR,
                            (WPARAM)TEXT("Error starting SOM: Check options"),
                            0);
                fclose(DataFile);
                return TRUE;
              }
              wsprintf (filetag, TEXT(".GNF"));
              GeneFile = _tfopen(filename, TEXT("wt"));
              if (!GeneFile)
              { MessageBox(NULL,
                           TEXT("Error: Unable to open the output file"),
                           TEXT("Error"),
                           0);
                fclose(DataFile);
                return TRUE;
              }
            }
        
            if (ClusterArrays)
            { if((ArrayIters==0)||(ArrayTau==0)||(ArrayXDim==0)||(ArrayYDim==0))
              { SendMessage(hWndMain,
                            IDM_SETSTATUSBAR,
                            (WPARAM)TEXT("Error starting SOM: Check options"),
                            0);
                if (GeneFile) fclose(GeneFile);
                fclose(DataFile);
                return TRUE;
              }
              wsprintf (filetag, TEXT(".ANF"));
              ArrayFile = _tfopen(filename, TEXT("wt"));
              if (!ArrayFile)
              { MessageBox(NULL,
                           TEXT("Error: Unable to open the output file"),
                           TEXT("Error"),
                           0);
                if (GeneFile) fclose(GeneFile);
                fclose(DataFile);
                return TRUE;
              }
            }

            PerformSOM(GeneFile, GeneXDim, GeneYDim, GeneIters, GeneTau,
                       GeneMetric, ArrayFile, ArrayXDim, ArrayYDim, ArrayIters,
                       ArrayTau, ArrayMetric);
            if (GeneFile) fclose(GeneFile);
            if (ArrayFile) fclose(ArrayFile);

            Save(DataFile,0,0);
            fclose(DataFile);

            SendMessage(hWndMain,
                        IDM_SETSTATUSBAR,
                        (WPARAM)TEXT("Done making SOM"),
                        0);
            return TRUE;
          }
        }
      }
    }
  }
  return FALSE;
}

BOOL CALLBACK
PcaDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{ switch (nMsg)
  { case WM_COMMAND:
    { if (HIWORD(wParam)==BN_CLICKED)
      { switch ((int) LOWORD(wParam))
        { case ID_PCA_BUTTON:
          { const int Rows = GetRows();
            const int Columns = GetColumns();
            HWND hTabCtrl = GetParent(hWnd);
            HWND hWndMain = GetParent(hTabCtrl);
            TCHAR* jobname;
            TCHAR filename[MAX_PATH];
            FILE* genefile;
            FILE* arrayfile;
            const char* error;

            if (Rows==0 || Columns==0)
            { SendMessage (hWndMain,
                           IDM_SETSTATUSBAR,
                           (WPARAM)TEXT("No data available"),
                           0);
              return TRUE;
            }

            SendMessage(hWndMain,IDM_GETJOBNAME,(WPARAM)&jobname,0);
            wsprintf (filename, TEXT("%s_svv.txt"), jobname);
            genefile = _tfopen(filename, TEXT("wt"));
            wsprintf (filename, TEXT("%s_svu.txt"), jobname);
            arrayfile = _tfopen(filename, TEXT("wt"));
            free(jobname);
            if(!genefile || !arrayfile)
            { if(genefile) fclose(genefile);
              if(arrayfile) fclose(genefile);
              SendMessage(hWndMain,
                          IDM_SETSTATUSBAR,
                          (WPARAM)TEXT("Error: Unable to open output file"),
                          0);
              return TRUE;
            }

            SendMessage(hWndMain,
                        IDM_SETSTATUSBAR,
                        (WPARAM)TEXT("Calculating SVD"),
                        0);
	    error = PerformPCA(genefile, arrayfile);
            fclose(genefile);
            fclose(arrayfile);
            if (error)
#ifdef UNICODE
            { TCHAR buffer[256];
              MultiByteToWideChar(CP_ACP, 0, error, -1, buffer, 256);
              MessageBox (NULL,
                          buffer,
                          TEXT("Error calculating PCA"),
                          MB_OK);
            }
#else
            { MessageBox (NULL,
                          error,
                          TEXT("Error calculating PCA"),
                          MB_OK);
            }
#endif
            SendMessage(hWndMain,
                        IDM_SETSTATUSBAR,
                        (WPARAM)TEXT("Finished Principal Component Analysis"),
                        0);
            return TRUE;
          }
        }
      }
    }
  }
  return 0;
}

/*============================================================================*/
/* Callback functions --- Other windows                                       */
/*============================================================================*/

BOOL CALLBACK
FileFormatProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{ static HBITMAP hBmp = NULL;
  switch (uMsg)
  { case WM_INITDIALOG:
    { HINSTANCE hInst = GetModuleHandle(NULL);
      hBmp = (HBITMAP)LoadImage(hInst,
                                MAKEINTRESOURCE(ID_FILEFORMAT_BMP),
                                IMAGE_BITMAP,
                                0,
                                0,
                                LR_DEFAULTCOLOR);
      if(hBmp) SendDlgItemMessage(hwndDlg,
                                  ID_FILEFORMAT_BMP_LOCATION,
                                  STM_SETIMAGE,
                                  IMAGE_BITMAP,
                                  (LPARAM)hBmp);
      return TRUE;
    }
    case WM_SYSCOMMAND:
      if (wParam == SC_CLOSE)
      { if (hBmp)
        { DeleteObject(hBmp);
          hBmp = NULL;
        }
        DestroyWindow (hwndDlg);
        return TRUE;
      }
      break;
  }
  return FALSE;
}

BOOL CALLBACK
AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ switch (uMsg)
  { case WM_SYSCOMMAND:
      if (wParam == SC_CLOSE)
      { DestroyWindow (hwndDlg);
        return TRUE;
      }
      break;
  }
  return FALSE;
}

/*============================================================================*/
/* Callback functions --- Main dialog window                                  */
/*============================================================================*/

BOOL CALLBACK
MainDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{ static HWND hWndTabCtrl;
  static HWND hWndPages[6];
  static int iCurrentPage = 0;
  static TCHAR* szHomeDir = NULL;
  static TCHAR* directory;
  switch (nMsg)
  { case WM_INITDIALOG:
    { /* Variables needed to read registry information */
      HKEY hKey;
      LONG lRet;
      /* Reserve MAX_PATH bytes for the file name */
      DWORD dwSize = MAX_PATH;
      TCHAR buffer[MAX_PATH];
      int n;
      /* Generic tab page information */
      TCITEM tie;

      /* Get the icon */
      HINSTANCE hInst = GetModuleHandle(NULL);
      HICON hIcon = LoadIcon (hInst, MAKEINTRESOURCE (ID_ICON));
      SendMessage (hWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
      /* Read directory information from the registry */
      lRet = RegOpenKeyEx(HKEY_CURRENT_USER,
                          TEXT("Software\\Stanford\\Cluster\\Directory"),
                          0,
                          KEY_QUERY_VALUE,
                          &hKey);
      directory = malloc(dwSize*sizeof(TCHAR));
      lRet = RegQueryValueEx(hKey,
                             TEXT("LastOpenDirectory"),
                             NULL,
                             NULL,
                             (LPBYTE)directory,
                             &dwSize);
      if (lRet==ERROR_MORE_DATA)
      /* directory was not large enough; dwSize now contains the needed size */
      { free (directory);
        directory = malloc(dwSize*sizeof(TCHAR));
        lRet = RegQueryValueEx(hKey,
                               TEXT("LastOpenDirectory"),
                               NULL,
                               NULL,
                               (LPBYTE)directory,
                               &dwSize);
      }
      if (lRet!=ERROR_SUCCESS)
      { free(directory);
        directory = NULL;
      }
      RegCloseKey(hKey);
      /* Create tab pages */
      hWndTabCtrl = GetDlgItem(hWnd, ID_TABCTRL);
      memset(&tie, 0, sizeof(TCITEM));
      tie.mask = TCIF_TEXT; 
      tie.pszText = TEXT("Filter Data");
      TabCtrl_InsertItem(hWndTabCtrl, 0, &tie);
      tie.pszText = TEXT("Adjust Data");
      TabCtrl_InsertItem(hWndTabCtrl, 1, &tie);
      tie.pszText = TEXT("Hierarchical");
      TabCtrl_InsertItem(hWndTabCtrl, 2, &tie);
      tie.pszText = TEXT("k-Means");
      TabCtrl_InsertItem(hWndTabCtrl, 3, &tie);
      tie.pszText = TEXT("SOMs");
      TabCtrl_InsertItem(hWndTabCtrl, 4, &tie);
      tie.pszText = TEXT("PCA");
      TabCtrl_InsertItem(hWndTabCtrl, 5, &tie);
      hWndPages[0]=
        CreateDialog(0,
                     MAKEINTRESOURCE(ID_FILTER_TAB),
                     hWndTabCtrl,
                     &FilterDialogProc);
      hWndPages[1]=
        CreateDialog(0,
                     MAKEINTRESOURCE(ID_ADJUST_TAB),
                     hWndTabCtrl,
                     &AdjustDialogProc);
      hWndPages[2]=
        CreateDialog(0,
                     MAKEINTRESOURCE(ID_HIERARCHICAL_TAB),
                     hWndTabCtrl,
                     &HierarchicalDialogProc);
      hWndPages[3]=
        CreateDialog(0,
                     MAKEINTRESOURCE(ID_KMEANS_TAB),
                     hWndTabCtrl,
                     &KmeansDialogProc);
      hWndPages[4]=
        CreateDialog(0,
                     MAKEINTRESOURCE(ID_SOM_TAB),
                     hWndTabCtrl,
                     &SOMDialogProc);
      hWndPages[5]=
        CreateDialog(0,
                     MAKEINTRESOURCE(ID_PCA_TAB),
                     hWndTabCtrl,
                     &PcaDialogProc);

      SetWindowLong(hWndPages[0], GWL_USERDATA, (LONG)ID_FILTER_TAB);
      SetWindowLong(hWndPages[1], GWL_USERDATA, (LONG)ID_ADJUST_TAB); 
      SetWindowLong(hWndPages[2], GWL_USERDATA, (LONG)ID_HIERARCHICAL_TAB); 
      SetWindowLong(hWndPages[3], GWL_USERDATA, (LONG)ID_KMEANS_TAB);
      SetWindowLong(hWndPages[4], GWL_USERDATA, (LONG)ID_SOM_TAB);
      SetWindowLong(hWndPages[5], GWL_USERDATA, (LONG)ID_PCA_TAB);

      /* Get directory information */
      GetModuleFileName (NULL,buffer,MAX_PATH);
      *_tcsrchr(buffer,'\\') = '\0';
      n = lstrlen(buffer) + 1;
      szHomeDir = malloc(n*sizeof(TCHAR));
      _tcscpy (szHomeDir,buffer);
      /* Show the dialog */
      ShowWindow (hWndPages[iCurrentPage],SW_SHOWNORMAL);
      return TRUE;
    }
    case WM_NOTIFY:
    { if (lParam)
      { switch (((NMHDR*)lParam)->code)
        { case TCN_SELCHANGE:
          { ShowWindow (hWndPages[iCurrentPage],SW_HIDE);
            iCurrentPage = TabCtrl_GetCurSel (hWndTabCtrl);
            ShowWindow(hWndPages[iCurrentPage],SW_SHOWNORMAL);
            return TRUE;
          }
        }
      }
      return FALSE;
    }
    case WM_COMMAND:
    { switch (LOWORD(wParam))
      { case CMD_FILE_OPEN:
        /* User will select a data file (*.txt) */
        { OPENFILENAME ofn;
          TCHAR lpstrFile[MAX_PATH];
          ZeroMemory(&ofn, sizeof(OPENFILENAME));
          ofn.lStructSize = sizeof(OPENFILENAME);
          ofn.hwndOwner = hWnd;
          ofn.lpstrFile = lpstrFile;
          ofn.nMaxFile = MAX_PATH;
          ofn.lpstrFilter = TEXT("Data (*.txt)\0*.TXT\0All files (*.*)\0*.*\0");
          ofn.nFilterIndex = 1;
          ofn.lpstrTitle = TEXT("Select data file to open");
          ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
          ofn.lpstrFile[0] = '\0';
          SendMessage(hWnd,
                      IDM_SETSTATUSBAR,
                      (WPARAM)TEXT("Opening file"),
                      0);
          if (GetOpenFileName(&ofn))
          { char* error = NULL;
            TCHAR buffer[256];
            FILE* inputfile = _tfopen (ofn.lpstrFile, TEXT("rt"));
            /* Save the directory name based on the file name */
	    SendMessage(hWnd,IDM_SAVEDIR,(WPARAM)ofn.lpstrFile,0);
            /* Read file */
            if (!inputfile)
            { MessageBox(NULL,TEXT("Error opening file"),TEXT("Error"),MB_OK);
              return TRUE;
            }
            SetDlgItemText (hWnd, ID_FILEMANAGER_FILEMEMO, TEXT(""));
            SetDlgItemText (hWnd, ID_FILEMANAGER_JOBNAME, TEXT(""));
            SetDlgItemText (hWnd, ID_FILEMANAGER_ROWS, TEXT(""));
            SetDlgItemText (hWnd, ID_FILEMANAGER_COLUMNS, TEXT(""));
            error = Load(inputfile);
            fclose(inputfile);
            if (error)
            {
#ifdef UNICODE
              MultiByteToWideChar(CP_ACP, 0, error, -1, buffer, 256);
              MessageBox (NULL,
                          buffer,
                          TEXT("Error in data file"),
                          MB_OK);
#else
              MessageBox (NULL,
                          error,
                          TEXT("Error in data file"),
                          MB_OK);
#endif
              free(error);
              wsprintf (buffer, TEXT("Error reading file %s"), ofn.lpstrFile);
              SendMessage(hWnd,IDM_SETSTATUSBAR, (WPARAM)buffer,0);
              return TRUE;
            }
            /* Extract job name from file name */
            SendMessage(hWnd,
                        IDM_SETSTATUSBAR,
                        (WPARAM)TEXT("Done loading data"),
                        0);
	    SendMessage(GetDlgItem(hWnd, ID_FILTER_TAB), IDM_RESET, 0, 0);
            SetDlgItemText (hWnd, ID_FILEMANAGER_FILEMEMO, lpstrFile);
            *(_tcsrchr(lpstrFile,'.')) = '\0';
            SetDlgItemText (hWnd,
                            ID_FILEMANAGER_JOBNAME,
                            _tcsrchr(lpstrFile,'\\')+1);
            SendMessage(hWnd,IDM_UPDATEINFO,0,0);
          }
          else
            SendMessage(hWnd ,IDM_SETSTATUSBAR, (WPARAM)TEXT("Cancelled"), 0);
          return TRUE;
        }
        case CMD_FILE_SAVE:
        { TCHAR lpstrFile[MAX_PATH];
          OPENFILENAME ofn;
          SendMessage(hWnd,
                      IDM_SETSTATUSBAR,
                      (WPARAM)TEXT("Saving data to file"),
                      0);
          ZeroMemory(&ofn, sizeof(OPENFILENAME));
          ofn.lStructSize = sizeof(OPENFILENAME);
          ofn.hwndOwner = hWnd;
          ofn.nMaxFile = MAX_PATH;
          ofn.lpstrFile = lpstrFile;
          ofn.lpstrFilter = TEXT("Text files\0*.TXT\0All files\0*.*\0");
          ofn.nFilterIndex = 1;
          ofn.lpstrInitialDir = directory;
          ofn.lpstrTitle = TEXT("Select file name to save to");
          ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
          GetDlgItemText(hWnd,
                         ID_FILEMANAGER_JOBNAME,
                         ofn.lpstrFile,
                         ofn.nMaxFile);
          lstrcat (ofn.lpstrFile, TEXT(".txt"));
          if (GetSaveFileName(&ofn))
          { /* Save the data to file */
            FILE* outputfile;
	    SendMessage(hWnd,IDM_SAVEDIR,(WPARAM)ofn.lpstrFile,0);
            outputfile = _tfopen(ofn.lpstrFile, TEXT("wt"));
            if (!outputfile)
            { MessageBox(NULL,
                         TEXT("Error: Unable to open the output file"),
                         TEXT("Error"),
                         0);
              return TRUE;
            }
            Save(outputfile,0,0);
            fclose(outputfile);
            SendMessage(hWnd,
                        IDM_SETSTATUSBAR,
                        (WPARAM)TEXT("Finished saving file"),
                        0);
          }
          else
            SendMessage(hWnd, IDM_SETSTATUSBAR, (WPARAM)TEXT("Cancelled"), 0);

          return TRUE;
        }
        case CMD_FILE_EXIT:
        { DestroyWindow(hWnd);
          return TRUE;
        }
        case CMD_HELP_HTMLHELP:
        { /* Open Windows HTML Help file */
          ShellExecute(hWnd,
                       TEXT("open"),
                       TEXT("cluster.chm"),
                       0,
                       szHomeDir,
                       SW_SHOWNORMAL);
          return TRUE;
        }
        case CMD_HELP_MANUAL:
        { /* Open local manual (PDF file) */
          ShellExecute(hWnd,
                       TEXT("open"),
                       TEXT("doc\\cluster3.pdf"),
                       0,
                       szHomeDir,
                       SW_SHOWNORMAL);
          return TRUE;
        }
        case CMD_HELP_DOWNLOAD:
        { /* Open Cluster Manual in Browser Window */
          ShellExecute(hWnd,
                       TEXT("open"),
TEXT("http://bonsai.ims.u-tokyo.ac.jp/~mdehoon/software/cluster/manual"),
                       0,
                       0,
                       SW_SHOWNORMAL);
          return TRUE;
        }
        case CMD_HELP_FILEFORMAT:
        { HWND hWndFileFormat =
            CreateDialog(0,MAKEINTRESOURCE(ID_FILEFORMAT),hWnd,&FileFormatProc);
          ShowWindow(hWndFileFormat,SW_SHOWDEFAULT);
          return TRUE;
        }
        case CMD_HELP_ABOUT:
        { HWND hWndAbout =
            CreateDialog(0, MAKEINTRESOURCE(ID_ABOUT), hWnd, &AboutProc);
          ShowWindow(hWndAbout, SW_SHOWDEFAULT);
          return TRUE;
        }
      }
      return FALSE;
    }
    case IDM_SAVEDIR:
    { TCHAR* fullpathfilename = (TCHAR*)wParam;
      TCHAR* filename = _tcsrchr (fullpathfilename,'\\');
      int n = filename - fullpathfilename + 1; 
      free (directory);
      directory  = malloc((n+1)*sizeof(TCHAR));
      _tcsncpy (directory, fullpathfilename, n);
      directory[n] = '\0';
      return TRUE;
    }
    case IDM_GETJOBNAME:
    { TCHAR buffer[MAX_PATH];
      TCHAR* jobname;
      const int n=GetDlgItemText(hWnd,ID_FILEMANAGER_JOBNAME,buffer,MAX_PATH);
      if (n==0)
      { MessageBox (NULL,
                    TEXT("Please specify a job name"),
                    TEXT("Error starting calculation"),
                    MB_OK);
        return 0;
      }
      jobname = malloc((n+1)*lstrlen(buffer)*sizeof(TCHAR));
      /* One more for the terminating \0 */
      lstrcpy (jobname, buffer);
      *(TCHAR**)wParam = jobname;
      return TRUE;
    }
    case IDM_UPDATEINFO:
    { const int Rows = GetRows();
      const int Columns = GetColumns();
      SetDlgItemInt(hWnd,ID_FILEMANAGER_ROWS,Rows,FALSE);
      SetDlgItemInt(hWnd,ID_FILEMANAGER_COLUMNS,Columns,FALSE);
      /* Update SOM defaults to reflect new number of rows */
      SendMessage (hWndPages[4],IDM_UPDATEINFO,(WPARAM)&Rows,(LPARAM)&Columns);
      return 0;
    }
    case IDM_SETSTATUSBAR:
    { HWND hWndStatusBar = GetDlgItem(hWnd, ID_STATUSBAR);
      SetWindowText (hWndStatusBar, (TCHAR*)wParam);
      return 0;
    }
    case WM_DESTROY:
    { /* Write directory information to the registry */
      HKEY hKey;
      LONG lRet;
      lRet = RegOpenKeyEx(HKEY_CURRENT_USER,
                          TEXT("Software\\Stanford\\Cluster\\Directory"),
                          0,
                          KEY_SET_VALUE,
                          &hKey);
      if (lRet == ERROR_SUCCESS)
      { DWORD dwSize = lstrlen(directory)+1;
        /* One more for the terminating \0 */
        RegSetValueEx(hKey,
                      TEXT("LastOpenDirectory"),
                      0,
                      REG_SZ,
                      (LPBYTE)directory,
                      dwSize);
        RegCloseKey(hKey);
      }
      free(szHomeDir);
      free(directory);
      Free();
      PostQuitMessage((int)wParam);
    }
    case WM_CLOSE:
    { return EndDialog(hWnd,0);
    }
  }
  return FALSE;
}

/*============================================================================*/
/*  Main                                                                      */
/*============================================================================*/

int STDCALL
WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{ /* Show the main window */
  return DialogBox(hInst,MAKEINTRESOURCE(ID_MAINDIALOG),NULL,&MainDialogProc);
}
