
/****************************************************************************/
/*      TmkInit v2.00 for LINUX 2.4 ELCUS, 1995,2008.                       */
/****************************************************************************/

#include <stdio.h>
#include <string.h>

#define _TMK1553B_MRT
#define USE_TMK_ERROR
#include "ltmk.c"

#ifdef TMK_CONFIGURATION_TABLE
TTmkConfigData aTmkConfig[MAX_TMK_NUMBER + 1];
#endif

#define TMK_FILE_OPEN_ERROR 21
#define TMK_FILE_READ_ERROR 22
#define TMK_FILE_FORMAT_ERROR 23
#define TMK_UNKNOWN_TYPE 24

unsigned int TmkEvents = 0;

int TmkInit(char *pszTMKFileName)
{
	int nResult;
	int hTMK;
	char achParams[81];
	FILE *hTMKFile;

#ifdef TMK_CONFIGURATION_TABLE
	for (hTMK = 0; hTMK <= MAX_TMK_NUMBER; hTMK++) {
		aTmkConfig[hTMK].nType = -1;
		aTmkConfig[hTMK].szName[0] = '\0';
		aTmkConfig[hTMK].wPorts1 = aTmkConfig[hTMK].wPorts2 = 0;
		aTmkConfig[hTMK].wIrq1 = aTmkConfig[hTMK].wIrq2 = 0;
	}
#endif
	if (TmkOpen()) {
		return TMK_FILE_OPEN_ERROR;
	}
	if ((hTMKFile = fopen(pszTMKFileName, "r")) == NULL) {
		return TMK_FILE_OPEN_ERROR;
	}
	while (1) {
		if (fgets(achParams, 80, hTMKFile) == NULL) {
			if (feof(hTMKFile)) {
				break;
			} else {
				nResult = TMK_FILE_READ_ERROR;
				goto ExitTmkInit;
			}
		}
		if (achParams[0] == '*') {
			break;
		}
		if (sscanf(achParams, "%u", &hTMK) != 1) {
			continue;
		}
		if (hTMK > tmkgetmaxn()) {
			nResult = TMK_FILE_FORMAT_ERROR;
			goto ExitTmkInit;
		}
		nResult = tmkconfig(hTMK);
#ifdef TMK_CONFIGURATION_TABLE
		tmkgetinfo(&(aTmkConfig[hTMK]));
#endif
		if (nResult) {
			break;
		}
		TmkEvents |= (1 << hTMK);
	} /* endwhile(!feof()) */
ExitTmkInit:
	fclose(hTMKFile);
	return nResult;
}
