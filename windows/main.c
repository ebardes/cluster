/* The C clustering library.
 * Copyright (C) 2002 Michiel Jan Laurens de Hoon.
 *
 * This library was written at the Laboratory of DNA Information Analysis,
 * Human Genome Center, Institute of Medical Science, University of Tokyo,
 * 4-6-1 Shirokanedai, Minato-ku, Tokyo 108-8639, Japan.
 * Contact: mdehoon 'AT' gsc.riken.jp
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation with or without modifications and for any purpose and
 * without fee is hereby granted, provided that any copyright notices
 * appear in all copies and that both those copyright notices and this
 * permission notice appear in supporting documentation, and that the
 * names of the contributors or copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific prior permission.
 * 
 * THE CONTRIBUTORS AND COPYRIGHT HOLDERS OF THIS SOFTWARE DISCLAIM ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT
 * OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */

/* If the user specified any command-line parameters, we run the command-line
 * version of Cluster 3.0. If not, we run the GUI version of Cluster 3.0, if
 * available.
 */

#include "command.h"

#ifdef UNICODE
#define _UNICODE
#endif

#include "stdlib.h"
#include "windows.h"
#include "tchar.h"

int main(int argc, const char* argv[])
{
  if (argc <= 1) {
    /* Invoked without command-line arguments: start the GUI. */
    int result;
    TCHAR* p;
    TCHAR szHomeDir[MAX_PATH];
    GetModuleFileName(NULL, szHomeDir, MAX_PATH);
    p = _tcsrchr(szHomeDir,'\\');
    *p = '\0';
    result = (int) ShellExecute(NULL,
                                TEXT("open"),
                                TEXT("cluster.exe"),
                                0,
                                szHomeDir,
                                SW_SHOWNORMAL);
    if (result > 32) return 0; /* GUI started */
    if (result==ERROR_FILE_NOT_FOUND)
      MessageBox(NULL,
                 TEXT("Unable to find cluster.exe"),
                 TEXT("Error"),
                 0);
    else
      MessageBox(NULL,
                 TEXT("Unknown error"),
                 TEXT("Error"),
                 0);
    return 1;
  }

  return commandmain(argc, argv);
}
