/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "../nsSBCharSetProber.h"

/********* Language model for: Nederlands *********/

/**
 * Generated by BuildLangModel.py
 * On: 2019-03-05 22:48:03.613070
 **/

/* Character Mapping Table:
 * ILL: illegal character.
 * CTR: control character specific to the charset.
 * RET: carriage/return.
 * SYM: symbol (punctuation) that does not belong to word.
 * NUM: 0 - 9.
 *
 * Other characters are ordered by probabilities
 * (0 is the most common character in the language).
 *
 * Orders are generic to a language. So the codepoint with order X in
 * CHARSET1 maps to the same character as the codepoint with the same
 * order X in CHARSET2 for the same language.
 * As such, it is possible to get missing order. For instance the
 * ligature of 'o' and 'e' exists in ISO-8859-15 but not in ISO-8859-1
 * even though they are both used for French. Same for the euro sign.
 */
constexpr unsigned char Windows_1252_CharToOrderMap[] =
{
  CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,RET,CTR,CTR,RET,CTR,CTR, /* 0X */
  CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR, /* 1X */
  SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM, /* 2X */
  NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,SYM,SYM,SYM,SYM,SYM,SYM, /* 3X */
  SYM,  2, 17, 16,  7,  0, 22, 10, 13,  3, 20, 14,  9, 12,  1,  6, /* 4X */
   18, 27,  4,  8,  5, 15, 11, 19, 25, 23, 21,SYM,SYM,SYM,SYM,SYM, /* 5X */
  SYM,  2, 17, 16,  7,  0, 22, 10, 13,  3, 20, 14,  9, 12,  1,  6, /* 6X */
   18, 27,  4,  8,  5, 15, 11, 19, 25, 23, 21,SYM,SYM,SYM,SYM,CTR, /* 7X */
  SYM,ILL,SYM, 55,SYM,SYM,SYM,SYM,SYM,SYM, 45,SYM, 53,ILL, 44,ILL, /* 8X */
  ILL,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM, 45,SYM, 53,ILL, 44, 59, /* 9X */
  SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM, /* AX */
  SYM,SYM,SYM,SYM,SYM, 54,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM, /* BX */
   38, 33, 43, 41, 37, 52, 47, 35, 30, 26, 39, 24, 60, 34, 40, 28, /* CX */
   56, 46, 58, 31, 36, 48, 32,SYM, 50, 61, 42, 62, 29, 63, 64, 65, /* DX */
   38, 33, 43, 41, 37, 52, 47, 35, 30, 26, 39, 24, 66, 34, 40, 28, /* EX */
   56, 46, 58, 31, 36, 48, 32,SYM, 50, 67, 42, 68, 29, 69, 70, 71, /* FX */
};
/*X0  X1  X2  X3  X4  X5  X6  X7  X8  X9  XA  XB  XC  XD  XE  XF */

constexpr unsigned char Iso_8859_1_CharToOrderMap[] =
{
  CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,RET,CTR,CTR,RET,CTR,CTR, /* 0X */
  CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR, /* 1X */
  SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM, /* 2X */
  NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,SYM,SYM,SYM,SYM,SYM,SYM, /* 3X */
  SYM,  2, 17, 16,  7,  0, 22, 10, 13,  3, 20, 14,  9, 12,  1,  6, /* 4X */
   18, 27,  4,  8,  5, 15, 11, 19, 25, 23, 21,SYM,SYM,SYM,SYM,SYM, /* 5X */
  SYM,  2, 17, 16,  7,  0, 22, 10, 13,  3, 20, 14,  9, 12,  1,  6, /* 6X */
   18, 27,  4,  8,  5, 15, 11, 19, 25, 23, 21,SYM,SYM,SYM,SYM,CTR, /* 7X */
  CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR, /* 8X */
  CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR, /* 9X */
  SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM, /* AX */
  SYM,SYM,SYM,SYM,SYM, 54,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM, /* BX */
   38, 33, 43, 41, 37, 52, 47, 35, 30, 26, 39, 24, 72, 34, 40, 28, /* CX */
   56, 46, 58, 31, 36, 48, 32,SYM, 50, 73, 42, 74, 29, 75, 76, 77, /* DX */
   38, 33, 43, 41, 37, 52, 47, 35, 30, 26, 39, 24, 78, 34, 40, 28, /* EX */
   56, 46, 58, 31, 36, 48, 32,SYM, 50, 79, 42, 80, 29, 81, 82, 83, /* FX */
};
/*X0  X1  X2  X3  X4  X5  X6  X7  X8  X9  XA  XB  XC  XD  XE  XF */

constexpr unsigned char Iso_8859_9_CharToOrderMap[] =
{
  CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,RET,CTR,CTR,RET,CTR,CTR, /* 0X */
  CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR, /* 1X */
  SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM, /* 2X */
  NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,SYM,SYM,SYM,SYM,SYM,SYM, /* 3X */
  SYM,  2, 17, 16,  7,  0, 22, 10, 13,  3, 20, 14,  9, 12,  1,  6, /* 4X */
   18, 27,  4,  8,  5, 15, 11, 19, 25, 23, 21,SYM,SYM,SYM,SYM,SYM, /* 5X */
  SYM,  2, 17, 16,  7,  0, 22, 10, 13,  3, 20, 14,  9, 12,  1,  6, /* 6X */
   18, 27,  4,  8,  5, 15, 11, 19, 25, 23, 21,SYM,SYM,SYM,SYM,CTR, /* 7X */
  CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR, /* 8X */
  CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR, /* 9X */
  SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM, /* AX */
  SYM,SYM,SYM,SYM,SYM, 54,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM, /* BX */
   38, 33, 43, 41, 37, 52, 47, 35, 30, 26, 39, 24, 84, 34, 40, 28, /* CX */
   57, 46, 58, 31, 36, 48, 32,SYM, 50, 85, 42, 86, 29, 87, 51, 88, /* DX */
   38, 33, 43, 41, 37, 52, 47, 35, 30, 26, 39, 24, 89, 34, 40, 28, /* EX */
   57, 46, 58, 31, 36, 48, 32,SYM, 50, 90, 42, 91, 29, 49, 51, 92, /* FX */
};
/*X0  X1  X2  X3  X4  X5  X6  X7  X8  X9  XA  XB  XC  XD  XE  XF */

constexpr unsigned char Iso_8859_15_CharToOrderMap[] =
{
  CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,RET,CTR,CTR,RET,CTR,CTR, /* 0X */
  CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR, /* 1X */
  SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM,SYM, /* 2X */
  NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,NUM,SYM,SYM,SYM,SYM,SYM,SYM, /* 3X */
  SYM,  2, 17, 16,  7,  0, 22, 10, 13,  3, 20, 14,  9, 12,  1,  6, /* 4X */
   18, 27,  4,  8,  5, 15, 11, 19, 25, 23, 21,SYM,SYM,SYM,SYM,SYM, /* 5X */
  SYM,  2, 17, 16,  7,  0, 22, 10, 13,  3, 20, 14,  9, 12,  1,  6, /* 6X */
   18, 27,  4,  8,  5, 15, 11, 19, 25, 23, 21,SYM,SYM,SYM,SYM,CTR, /* 7X */
  CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR, /* 8X */
  CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR,CTR, /* 9X */
  SYM,SYM,SYM,SYM,SYM,SYM, 45,SYM, 45,SYM,SYM,SYM,SYM,SYM,SYM,SYM, /* AX */
  SYM,SYM,SYM,SYM, 44, 54,SYM,SYM, 44,SYM,SYM,SYM, 53, 53, 93,SYM, /* BX */
   38, 33, 43, 41, 37, 52, 47, 35, 30, 26, 39, 24, 94, 34, 40, 28, /* CX */
   56, 46, 58, 31, 36, 48, 32,SYM, 50, 95, 42, 96, 29, 97, 98, 99, /* DX */
   38, 33, 43, 41, 37, 52, 47, 35, 30, 26, 39, 24,100, 34, 40, 28, /* EX */
   56, 46, 58, 31, 36, 48, 32,SYM, 50,101, 42,102, 29,103,104,105, /* FX */
};
/*X0  X1  X2  X3  X4  X5  X6  X7  X8  X9  XA  XB  XC  XD  XE  XF */


/* Model Table:
 * Total sequences: 924
 * First 512 sequences: 0.9980487395086337
 * Next 512 sequences (512-1024): 0.0019512604913663406
 * Rest: -4.9439619065339e-17
 * Negative sequences: TODO
 */
constexpr PRUint8 NederlandsLangModel[] =
{
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,3,2,0,2,0,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,2,2,2,0,2,0,0,0,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,3,0,0,0,0,
  3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,3,3,3,3,3,3,3,3,3,3,3,2,3,2,0,3,0,0,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,3,2,0,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,3,2,0,2,0,2,0,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,0,2,3,0,0,0,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,3,3,3,3,2,3,2,0,2,2,0,2,2,2,0,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,2,2,0,0,0,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,2,3,0,0,2,2,2,0,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,3,3,2,3,3,3,0,2,2,0,0,2,2,2,0,
  3,2,3,3,3,2,3,3,3,3,2,2,2,0,2,3,2,2,2,2,3,2,2,3,0,0,2,0,0,0,0,2,0,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,3,3,0,2,2,0,0,2,0,0,0,
  3,3,3,3,3,3,3,2,3,3,2,0,3,2,2,3,2,3,2,2,2,3,2,3,0,0,2,2,0,2,2,0,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,3,3,3,3,2,3,0,2,0,0,0,2,0,2,2,
  3,3,3,3,3,3,2,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,3,2,2,3,2,2,2,2,0,0,0,
  3,2,3,3,3,3,3,3,2,3,2,2,2,3,3,3,3,3,2,0,2,2,2,3,0,2,2,2,0,0,2,2,0,
  3,3,3,3,3,3,3,3,3,3,2,2,2,3,2,3,3,3,3,2,3,2,3,3,0,0,2,0,0,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,3,2,3,0,2,2,0,0,0,0,0,0,
  3,2,3,3,3,3,3,3,3,2,2,2,2,3,2,2,2,3,2,3,2,2,2,3,0,0,2,2,0,2,2,0,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,3,3,3,3,3,2,0,0,2,2,0,2,0,2,2,
  3,2,3,3,2,2,3,2,2,2,2,2,2,2,2,3,2,2,0,3,2,3,2,3,0,0,2,0,0,2,2,0,0,
  3,2,3,3,3,3,3,3,3,3,3,3,2,3,3,3,2,3,2,3,2,3,3,3,0,0,2,0,0,2,0,0,0,
  3,3,3,2,3,3,3,3,3,3,2,2,3,2,2,2,3,2,3,3,2,3,2,2,0,0,2,2,0,0,2,0,2,
  2,3,2,3,3,2,0,3,2,3,0,2,2,0,0,0,0,0,0,0,0,2,2,0,0,2,0,0,0,0,0,0,0,
  3,0,3,3,2,3,3,0,2,2,0,2,2,2,0,2,2,0,3,2,0,0,2,3,0,2,0,0,0,0,0,0,0,
  2,3,2,2,2,2,2,2,2,2,2,2,2,0,2,2,2,2,2,0,0,2,2,0,0,0,3,2,0,0,0,0,0,
  2,0,2,2,0,0,2,0,2,2,0,0,0,0,0,3,0,2,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,
  2,3,0,0,2,2,0,3,3,2,0,0,2,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,2,0,0,2,2,0,0,2,2,2,0,2,2,2,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,2,0,0,3,2,0,0,2,2,2,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,
  0,2,0,0,2,2,0,0,0,0,0,0,2,2,2,0,0,2,2,2,0,2,0,0,0,0,0,0,0,0,0,2,0,
  0,2,0,2,2,2,0,0,0,2,0,0,2,2,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,
};


const SequenceModel Windows_1252NederlandsModel =
{
  Windows_1252_CharToOrderMap,
  NederlandsLangModel,
  33,
  (float)0.9980487395086337,
  PR_TRUE,
  "WINDOWS-1252"
};

const SequenceModel Iso_8859_1NederlandsModel =
{
  Iso_8859_1_CharToOrderMap,
  NederlandsLangModel,
  33,
  (float)0.9980487395086337,
  PR_TRUE,
  "ISO-8859-1"
};

const SequenceModel Iso_8859_9NederlandsModel =
{
  Iso_8859_9_CharToOrderMap,
  NederlandsLangModel,
  33,
  (float)0.9980487395086337,
  PR_TRUE,
  "ISO-8859-9"
};

const SequenceModel Iso_8859_15NederlandsModel =
{
  Iso_8859_15_CharToOrderMap,
  NederlandsLangModel,
  33,
  (float)0.9980487395086337,
  PR_TRUE,
  "ISO-8859-15"
};
