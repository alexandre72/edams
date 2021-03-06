/* xPL-utils.c - misc support for xPLLib */
/* Copyright 2004 (c), Gerald R Duprey Jr */

#include <stdio.h>
#include <stdarg.h>
#include "xpl.h"
#include "xpl-priv.h"

/* static buffer to create log messages */
static char convertBuffer[32];

#define NAME_VALUE_LIST_GROW_BY 8;


/* Add a new entry to a passed name/value pair list */
xPL_NameValuePairPtr xPL_newNamedValuePair(xPL_NameValueListPtr nameValueList, String theName) {
  xPL_NameValuePairPtr theNewNamedValue;

  /* See if the current array is big enough and if not, allocate more space */
  if ((nameValueList->namedValues == NULL) || (nameValueList->namedValueCount == nameValueList->namedValueAlloc)) {
    /* Resize the passed list, allocate & init a new name/value pair, and add it to the list */
    nameValueList->namedValueAlloc += NAME_VALUE_LIST_GROW_BY;
    nameValueList->namedValues = realloc(nameValueList->namedValues, sizeof(xPL_NameValuePairPtr) * nameValueList->namedValueAlloc);
  }

  /* Allocate a new item and insert it into the list */
  theNewNamedValue = xPL_AllocNVPair();
  nameValueList->namedValues[nameValueList->namedValueCount] = theNewNamedValue;
  nameValueList->namedValueCount++;

  /* Init the name value pair and return it */
  theNewNamedValue->itemName = xPL_StrDup(theName);
  return theNewNamedValue;
}

/* Just add a simple entry to the list */
void xPL_addNamedValue(xPL_NameValueListPtr theList, String theName, String theValue) {
  xPL_NameValuePairPtr theNamedValue = xPL_newNamedValuePair(theList, theName);
  if (theValue != NULL) theNamedValue->itemValue = xPL_StrDup(theValue);
}

/* Attempt to update an existing name/value and if it is not */
/* existing, create and add a new one                        */
void xPL_setNamedValue(xPL_NameValueListPtr theList, String theName, String theValue) {
  int nvIndex = xPL_getNamedValueIndex(theList, theName);
  if (nvIndex == -1) {
    xPL_addNamedValue(theList, theName, theValue);
    return;
  }

  /* Clear if it was binary */
  if (theList->namedValues[nvIndex]->isBinary) {
    theList->namedValues[nvIndex]->isBinary = EINA_FALSE;
    theList->namedValues[nvIndex]->itemValue = NULL;
  }

  /* Install new value */
  STR_FREE(theList->namedValues[nvIndex]->itemValue);
  if (theValue == NULL)
    theList->namedValues[nvIndex]->itemValue = NULL;
  else
    theList->namedValues[nvIndex]->itemValue = xPL_StrDup(theValue);
}

/* Set a series of NameValue pairs for a message */
void xPL_setNamedValues(xPL_NameValueListPtr theList, ...) {
  va_list argPtr;
  String theName, theValue;

 /* Handle the name/value pairs */
  va_start(argPtr, theList);
  for(;;) {
    /* Get the name.  NULL means End of List */
    if ((theName = va_arg(argPtr, String)) == NULL) break;

    /* Get the value */
    theValue = va_arg(argPtr, String);

    /* Create a name/value pair */
    xPL_setNamedValue(theList, theName, theValue);
  }
  va_end(argPtr);
}


/* Search for a name in a list of name values and return the */
/* index into the list of the value or -1 if not found       */
int xPL_getNamedValueIndex(xPL_NameValueListPtr theList, String theName) {
  int nvIndex;

  if (theList == NULL) return -1;

  for (nvIndex = 0; nvIndex < theList->namedValueCount; nvIndex++) {
    if (!strcasecmp(theName, theList->namedValues[nvIndex]->itemName))
      return nvIndex;
  }

  return -1;
}

/* Find the specified name in the name/value pair or return NULL */
xPL_NameValuePairPtr xPL_getNamedValuePair(xPL_NameValueListPtr theList, String theName) {
  int nvIndex = xPL_getNamedValueIndex(theList, theName);
  if (nvIndex == -1) return NULL;

  return theList->namedValues[nvIndex];
}

/* Find the specified name in the list and return it's value or NULL */
String xPL_getNamedValue(xPL_NameValueListPtr theList, String theName) {
  int nvIndex = xPL_getNamedValueIndex(theList, theName);
  if (nvIndex == -1) return NULL;

  if (theList->namedValues[nvIndex]->isBinary) return NULL;
  return theList->namedValues[nvIndex]->itemValue;
}

/* Return EINA_TRUE if there is a matching named value */
Eina_Bool xPL_doesNamedValueExist(xPL_NameValueListPtr theList, String theName) {
  return (xPL_getNamedValueIndex(theList, theName) != -1);
}

/* Return number of name value pairs */
int xPL_getNamedValueCount(xPL_NameValueListPtr theList) {
  if (theList == NULL) return 0;
  return theList->namedValueCount;
}

/* Return the value indexed at */
xPL_NameValuePairPtr xPL_getNamedValuePairAt(xPL_NameValueListPtr theList, int listIndex) {
  if ((theList == NULL) || (listIndex < 0) || (listIndex >= theList->namedValueCount)) return NULL;
  return theList->namedValues[listIndex];
}

/* Remove the name/value pair specified by the passed index */
void xPL_clearNamedValueAt(xPL_NameValueListPtr theList, int nameIndex) {
  /* Skip out if there is any problem */
  if ((theList == NULL) || (theList->namedValues == NULL) || (nameIndex < 0) || (nameIndex >= theList->namedValueCount)) return;

  /* First, free this thing */
  xPL_freeNamedValuePair(theList->namedValues[nameIndex]);

  /* Now fix up the array */
  if (nameIndex < (theList->namedValueCount - 1)) {
    memcpy(&(theList->namedValues[nameIndex]), &(theList->namedValues[nameIndex + 1]),
	   ((theList->namedValueCount - nameIndex) - 1) * sizeof(xPL_NameValuePairPtr));
  }

  /* Adjust counter and we are done */
  theList->namedValueCount--;
}

/* Remove all isntances of the passed name from the passed list */
void xPL_clearNamedValue(xPL_NameValueListPtr theList, String theName) {
  int nvIndex = 0;
  for(;;) {
    /* Fetch next instance of a named value */
    if ((nvIndex = xPL_getNamedValueIndex(theList, theName)) == -1) break;
    xPL_clearNamedValueAt(theList, nvIndex);
  }
}

/* Remove All name/value pairs from the passed list */
void xPL_clearAllNamedValues(xPL_NameValueListPtr theList) {
  int nvIndex;

  /* Bail if trouble is brewing */
  if ((theList == NULL) || (theList->namedValueCount <= 0)) return;

  /* Release each item */
  for (nvIndex = 0; nvIndex < theList->namedValueCount; nvIndex++)
    xPL_freeNamedValuePair(theList->namedValues[nvIndex]);

  theList->namedValueCount = 0;
}

/* Allocate a new named/value list */
xPL_NameValueListPtr xPL_newNamedValueList() {
  return xPL_AllocNVList();
}

/* Free an entire name/value list */
void
xPL_freeNamedValueList(xPL_NameValueListPtr theList)
{
  /* Skip out quick if things suck */
  if (theList == NULL) return;

  /* Clear values out, if any */
  if (theList->namedValues != NULL) xPL_clearAllNamedValues(theList);

  /* Free the list itself */
  xPL_FreeNVList(theList);
}

/* Free an Name/Value pair */
void
xPL_freeNamedValuePair(xPL_NameValuePairPtr thePair)
{
  STR_FREE(thePair->itemName);
  if (!thePair->isBinary) STR_FREE(thePair->itemValue);
  xPL_FreeNVPair(thePair);
}

/* For the passed integer with a value of 0-15, return a */
/* hex encoded number from 0-F.                          */
static char
nibbleToHex(int theValue)
{
  /* Handle simple digit */
  if ((theValue >= 0) && (theValue <= 9))
    return (char) 48 + theValue;

  /* Handle alphas */
  if ((theValue >= 10) && (theValue <= 15))
    return (char) 55 + theValue;

  /* Error */
  return '*';
}

/* Convert a hex character to a nibble value */
static int
hexToNibble(char theValue)
{
  if ((theValue >= '0') && (theValue <= '9'))
    return theValue - 48;
  else if ((theValue >= 'A') && (theValue <= 'F'))
    return theValue - 55;

  /* Anything else is an error */
  return -1;
}

/* Convert a number to hex.  Only the lower 8 bits */
/* of the passed value are examined and converted  */
String
xPL_intToHex(int theValue)
{
  convertBuffer[0] = nibbleToHex((theValue / 16) % 16);
  convertBuffer[1] = nibbleToHex(theValue % 16);
  convertBuffer[2] = '\0';
  return convertBuffer;
}

/* Convert a two digit hex string to an integer value */
/* If the string is invalid, EINA_FALSE is returned        */
Eina_Bool xPL_hexToInt(String theHexValue, int *theValue) {
  int lowValue, highValue;

  /* Convert high value */
  if ((highValue = hexToNibble(theHexValue[0])) == -1) return EINA_FALSE;

  /* Convert low value */
  if ((lowValue = hexToNibble(theHexValue[1])) == -1) return EINA_FALSE;

  /* And return it */
  *theValue = (highValue * 16) | lowValue;
  return EINA_TRUE;
}

/* Convert a passed string into a number and store in */
/* the passed value.  If the number is OK, then EINA_TRUE  */
/* is returned.  If there is an error, EINA_FALSE          */
Eina_Bool xPL_strToInt(String theValue, int *theResult) {
  String endChar;
  int intResult;

  /* Convert the value */
  intResult = strtol(theValue, &endChar, 10);
  if (*endChar != '\0') return EINA_FALSE;

  /* Set the Value in place */
  *theResult = intResult;
  return EINA_TRUE;
}

/* Convert an integer into a string */
String xPL_intToStr(int theValue) {
  sprintf(convertBuffer, "%d", theValue);
  return convertBuffer;
}



/* Context defined for parser */
static xPL_ConnectType xPL_ParsedConnectionType = xcViaHub;

/* Accessors for context */
xPL_ConnectType xPL_getParsedConnectionType() { return xPL_ParsedConnectionType; }
