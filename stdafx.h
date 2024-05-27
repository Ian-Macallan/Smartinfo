// stdafx.h : fichier Include pour les fichiers Include système standard,
// ou les fichiers Include spécifiques aux projets qui sont utilisés fréquemment,
// et sont rarement modifiés
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // certains constructeurs CString seront explicites
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS         // supprimer la prise en charge des contrôles MFC dans les boîtes de dialogue

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclure les en-têtes Windows rarement utilisés
#endif

#include <afx.h>
#include <afxwin.h>         // composants MFC principaux et standard
#include <afxext.h>         // extensions MFC
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Prise en charge MFC pour les contrôles communs Internet Explorer 4
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // Prise en charge des MFC pour les contrôles communs Windows
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <iostream>



// TODO: faites référence ici aux en-têtes supplémentaires nécessaires au programme
