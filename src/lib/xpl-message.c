/* xPL-message.c - xPL Message support functions */
/* Copyright 2004 (c) , Gerald R Duprey Jr */

#include <stdarg.h>
#include <sys/socket.h>
#include "xpl.h"
#include "xpl-priv.h"
#include "utils.h"

#define WRITE_TEXT(x) if (!appendText(x)) return EINA_FALSE;

#define MSG_MAX_SIZE 1500
#define MSG_BUFF_SIZE 2048
static char messageBuff[MSG_BUFF_SIZE];
static int messageBytesWritten;

static char blockHeaderBuff[128];
static char blockNameBuff[256];
static char blockValueBuff[MSG_MAX_SIZE];

static Eina_Bool hubConfirmed = EINA_FALSE;


/* Convert a text HEX character rep into actual binary data */
/* If there is an error in the data, NULL is returned       */
static
String textToBinary(String theText, int *binaryLength __UNUSED__)
{
  int theLength = strlen(theText);
  String theData = (String) malloc(theLength / 2);
  String dataPtr = theData;
  int theValue;
  int charPtr;

  for (charPtr = 0; charPtr < theLength;) {
    /* Convert hex into a value */
    if (!xPL_hexToInt(&theText[charPtr], &theValue)) {
      free(theData);
      return NULL;
    }

    /* Store that value & advance */
    *dataPtr++ = (theValue & 255);
    charPtr += 2;
  }

  /* Return the data */
  return theData;
}


/*
 *Append text and keep track of what we've used
 */
static Eina_Bool
appendText(String theText)
{
  int textLen = strlen(theText);

  /* Make sure this will fit in the buffer */
  if ((messageBytesWritten + textLen) >= MSG_MAX_SIZE)
  {
    return EINA_FALSE;
  }

  /* Copy the text in */
  memcpy(&messageBuff[messageBytesWritten], theText, textLen);
  messageBytesWritten += textLen;
  messageBuff[messageBytesWritten] = '\0';
  return EINA_TRUE;
}


/*
 *Convert a binary value in xPL encoded data
 */
static Eina_Bool
writeBinaryValue(String theData, int dataLen)
{
  int dataIndex;

  for (dataIndex = 0; dataIndex < dataLen; dataIndex++)
    WRITE_TEXT(xPL_intToHex(theData[dataIndex]));

  return EINA_TRUE;
}


Eina_Bool xPL_isHubConfirmed()
{
	return hubConfirmed;
}


void
xPL_setMessageType(xPL_MessagePtr theMessage, xPL_MessageType messageType) {
  if (theMessage->messageType == messageType) return;
  theMessage->messageType = messageType;
}
xPL_MessageType xPL_getMessageType(xPL_MessagePtr theMessage) {
  return theMessage->messageType;
}

int
xPL_getHopCount(xPL_MessagePtr theMessage)
{
  return theMessage->hopCount;
}

Eina_Bool xPL_isReceivedMessage(xPL_MessagePtr theMessage)
{
  return theMessage->receivedMessage;
}

void
xPL_setBroadcastMessage(xPL_MessagePtr theMessage, Eina_Bool isBroadcast)
{
  if (theMessage->isBroadcastMessage == isBroadcast) return;
  theMessage->isBroadcastMessage = isBroadcast;
}


Eina_Bool
xPL_isBroadcastMessage(xPL_MessagePtr theMessage)
{
  return theMessage->isBroadcastMessage;
}

void
xPL_setTargetGroup(xPL_MessagePtr theMessage, String theGroup)
{
  if (theGroup == NULL) {
    STR_FREE(theMessage->groupName);
    theMessage->isGroupMessage = EINA_FALSE;
  } else {
    if ((theMessage->groupName != NULL) && !strcasecmp(theMessage->groupName, theGroup)) return;
    STR_FREE(theMessage->targetVendor);
    STR_FREE(theMessage->targetDeviceID);
    STR_FREE(theMessage->targetInstanceID);
    STR_FREE(theMessage->groupName);

    theMessage->isGroupMessage = EINA_TRUE;
    theMessage->groupName = xPL_StrDup(theGroup);
  }
}


String
xPL_getTargetGroup(xPL_MessagePtr theMessage)
{
  return theMessage->groupName;
}


Eina_Bool xPL_isGroupMessage(xPL_MessagePtr theMessage)
{
  return theMessage->isGroupMessage;
}


void
xPL_setTargetVendor(xPL_MessagePtr theMessage, String theVendor)
{
  /* Skip unless a real change */
  if ((theMessage->targetVendor != NULL) && !strcasecmp(theMessage->targetVendor, theVendor)) return;

  /* Install new value */
  STR_FREE(theMessage->targetVendor);
  theMessage->targetVendor = xPL_StrDup(theVendor);

  /* Clear any group */
  if (theMessage->isGroupMessage)
   {
    STR_FREE(theMessage->groupName);
    theMessage->isGroupMessage = EINA_FALSE;
  }
}

String xPL_getTargetVendor(xPL_MessagePtr theMessage)
{
  return theMessage->targetVendor;
}


void xPL_setTargetDeviceID(xPL_MessagePtr theMessage, String theDeviceID)
{
  /* Skip unless a real change */
  if ((theMessage->targetDeviceID != NULL) && !strcasecmp(theMessage->targetDeviceID, theDeviceID)) return;

  /* Install new value */
  STR_FREE(theMessage->targetDeviceID);
  theMessage->targetDeviceID = xPL_StrDup(theDeviceID);

  /* Clear any group */
  if (theMessage->isGroupMessage)
  {
    STR_FREE(theMessage->groupName);
    theMessage->isGroupMessage = EINA_FALSE;
  }
}


String
xPL_getTargetDeviceID(xPL_MessagePtr theMessage)
{
  return theMessage->targetDeviceID;
}


void
xPL_setTargetInstanceID(xPL_MessagePtr theMessage, String theInstanceID)
 {
  /* Skip unless a real change */
  if ((theMessage->targetInstanceID != NULL) && !strcasecmp(theMessage->targetInstanceID, theInstanceID)) return;

  /* Install new value */
  STR_FREE(theMessage->targetInstanceID);
  theMessage->targetInstanceID = xPL_StrDup(theInstanceID);

  /* Clear any group */
  if (theMessage->isGroupMessage)
  {
    STR_FREE(theMessage->groupName);
    theMessage->isGroupMessage = EINA_FALSE;
  }
}


String
xPL_getTargetInstanceID(xPL_MessagePtr theMessage)
{
  return theMessage->targetInstanceID;
}


void
xPL_setTarget(xPL_MessagePtr theMessage, String theVendor, String theDeviceID, String theInstanceID)
{
  xPL_setTargetVendor(theMessage, theVendor);
  xPL_setTargetDeviceID(theMessage, theDeviceID);
  xPL_setTargetInstanceID(theMessage, theInstanceID);
}


void
xPL_setSourceVendor(xPL_MessagePtr theMessage, String theVendor)
{
  /* Skip unless a real change and this is a received message (can't change sendable messages) */
  if (!theMessage->receivedMessage) return;
  if ((theMessage->sourceVendor != NULL) && !strcasecmp(theMessage->sourceVendor, theVendor)) return;

  /* Install new value */
  STR_FREE(theMessage->sourceVendor);
  theMessage->sourceVendor = xPL_StrDup(theVendor);
}


String
xPL_getSourceVendor(xPL_MessagePtr theMessage)
{
  return theMessage->sourceVendor;
}


void
xPL_setSourceDeviceID(xPL_MessagePtr theMessage, String theDeviceID)
{
  /* Skip unless a real change and this is a received message (can't change sendable messages) */
  if (!theMessage->receivedMessage) return;
  if ((theMessage->sourceDeviceID != NULL) && !strcasecmp(theMessage->sourceDeviceID, theDeviceID)) return;

  /* Install new value */
  STR_FREE(theMessage->sourceDeviceID);
  theMessage->sourceDeviceID = xPL_StrDup(theDeviceID);
}


String
xPL_getSourceDeviceID(xPL_MessagePtr theMessage)
{
  return theMessage->sourceDeviceID;
}

void
xPL_setSourceInstanceID(xPL_MessagePtr theMessage, String theInstanceID)
{
  /* Skip unless a real change and this is a received message (can't change sendable messages) */
  if (!theMessage->receivedMessage) return;
  if ((theMessage->sourceInstanceID != NULL) && !strcasecmp(theMessage->sourceInstanceID, theInstanceID)) return;

  /* Install new value */
  STR_FREE(theMessage->sourceInstanceID);
  theMessage->sourceInstanceID = xPL_StrDup(theInstanceID);
}

String xPL_getSourceInstanceID(xPL_MessagePtr theMessage) {
  return theMessage->sourceInstanceID;
}

void
xPL_setSource(xPL_MessagePtr theMessage, String theVendor, String theDeviceID, String theInstanceID)
{
  /* Skip unless this is a received message (can't change sendable messages) */
  if (!theMessage->receivedMessage) return;

  xPL_setSourceVendor(theMessage, theVendor);
  xPL_setSourceDeviceID(theMessage, theDeviceID);
  xPL_setSourceInstanceID(theMessage, theInstanceID);
}


void
xPL_setSchemaClass(xPL_MessagePtr theMessage, String theSchemaClass)
{
  /* Skip unless a real change */
  if ((theMessage->schemaClass != NULL) && !strcasecmp(theMessage->schemaClass, theSchemaClass)) return;

  /* Install new value */
  STR_FREE(theMessage->schemaClass);
  theMessage->schemaClass = xPL_StrDup(theSchemaClass);
}

String xPL_getSchemaClass(xPL_MessagePtr theMessage) {
  return theMessage->schemaClass;
}

void xPL_setSchemaType(xPL_MessagePtr theMessage, String theSchemaType) {
  /* Skip unless a real change */
  if ((theMessage->schemaType != NULL) && !strcasecmp(theMessage->schemaType, theSchemaType)) return;

  /* Install new value */
  STR_FREE(theMessage->schemaType);
  theMessage->schemaType = xPL_StrDup(theSchemaType);
}

String xPL_getSchemaType(xPL_MessagePtr theMessage) {
  return theMessage->schemaType;
}

void xPL_setSchema(xPL_MessagePtr theMessage, String theSchemaClass, String theSchemaType) {
  xPL_setSchemaClass(theMessage, theSchemaClass);
  xPL_setSchemaType(theMessage, theSchemaType);
}

xPL_NameValueListPtr xPL_getMessageBody(xPL_MessagePtr theMessage) {
  if (theMessage->messageBody == NULL) return NULL;
  return theMessage->messageBody;
}

void xPL_clearMessageNamedValues(xPL_MessagePtr theMessage) {
  xPL_clearAllNamedValues(theMessage->messageBody);
}

Eina_Bool xPL_doesMessageNamedValueExist(xPL_MessagePtr theMessage, String theName) {
  if (theMessage->messageBody == NULL) return EINA_FALSE;
  return xPL_doesNamedValueExist(theMessage->messageBody, theName);
}

String xPL_getMessageNamedValue(xPL_MessagePtr theMessage, String theName) {
  if (theMessage->messageBody == NULL) return NULL;
  return xPL_getNamedValue(theMessage->messageBody, theName);
}

void xPL_addMessageNamedValue(xPL_MessagePtr theMessage, String theName, String theValue) {
  if (theMessage->messageBody == NULL) theMessage->messageBody = xPL_newNamedValueList();
  xPL_addNamedValue(theMessage->messageBody, theName, theValue);
}

void xPL_setMessageNamedValue(xPL_MessagePtr theMessage, String theName, String theValue) {
  if (theMessage->messageBody == NULL) {
    xPL_addMessageNamedValue(theMessage, theName, theValue);
    return;
  }

  xPL_setNamedValue(theMessage->messageBody, theName, theValue);
}

/* Set a series of NameValue pairs for a message */
void
xPL_setMessageNamedValues(xPL_MessagePtr theMessage, ...)
{
  va_list argPtr;
  String theName, theValue;

 /* Handle the name/value pairs */
  va_start(argPtr, theMessage);
  for(;;) {
    /* Get the name.  NULL means End of List */
    if ((theName = va_arg(argPtr, String)) == NULL) break;

    /* Get the value */
    theValue = va_arg(argPtr, String);

    /* Create a name/value pair */
    xPL_setMessageNamedValue(theMessage, theName, theValue);
  }
  va_end(argPtr);
}


/* Create a new message based on a service */
static xPL_MessagePtr
createSendableMessage(xPL_ServicePtr theService, xPL_MessageType messageType)
{
  xPL_MessagePtr theMessage;

  /* Allocate the message */
  theMessage = xPL_AllocMessage();

  /* Set the version (NOT DYNAMIC) */
  theMessage->messageType = messageType;
  theMessage->hopCount = 1;
  theMessage->receivedMessage = EINA_FALSE;

  theMessage->sourceVendor = xPL_getServiceVendor(theService);
  theMessage->sourceDeviceID = xPL_getServiceDeviceID(theService);
  theMessage->sourceInstanceID = xPL_getServiceInstanceID(theService);

  /* And we are done */
  return theMessage;
}

/* Create a new message based on a service */
static xPL_MessagePtr
createReceivedMessage(xPL_MessageType messageType)
{
  xPL_MessagePtr theMessage;

  /* Allocate the message */
  theMessage = xPL_AllocMessage();

  /* Set the version (NOT DYNAMIC) */
  theMessage->messageType = messageType;
  theMessage->receivedMessage = EINA_TRUE;

  /* Allocate a name/value list, if needed */
  if (theMessage->messageBody == NULL) theMessage->messageBody = xPL_AllocNVList();

  /* And we are done */
  return theMessage;
}

/* Create a message suitable for sending to a specific receiver */
xPL_MessagePtr xPL_createTargetedMessage(xPL_ServicePtr theService, xPL_MessageType messageType,
					 String theVendor, String theDevice, String theInstance) {

  xPL_MessagePtr theMessage = createSendableMessage(theService, messageType);
  xPL_setTarget(theMessage, theVendor, theDevice, theInstance);
  return theMessage;
}

/* Create a message suitable for sending to a group */
xPL_MessagePtr xPL_createGroupTargetedMessage(xPL_ServicePtr theService, xPL_MessageType messageType, String theGroup) {
  xPL_MessagePtr theMessage = createSendableMessage(theService, messageType);
  xPL_setTargetGroup(theMessage, theGroup);
  return theMessage;
}

/* Create a message suitable for broadcasting to all listeners */
xPL_MessagePtr xPL_createBroadcastMessage(xPL_ServicePtr theService, xPL_MessageType messageType) {
  xPL_MessagePtr theMessage = createSendableMessage(theService, messageType);
  xPL_setBroadcastMessage(theMessage, EINA_TRUE);
  return theMessage;
}

/* Release a message and all it's resources */
void
xPL_releaseMessage(xPL_MessagePtr theMessage)
{
  /* Free Parsed stuff */
  if (theMessage->receivedMessage) {
    STR_FREE(theMessage->sourceVendor);
    STR_FREE(theMessage->sourceDeviceID);
    STR_FREE(theMessage->sourceInstanceID);
  } else {
    theMessage->sourceVendor = NULL;
    theMessage->sourceDeviceID = NULL;
    theMessage->sourceInstanceID = NULL;
  }

  theMessage->isBroadcastMessage = EINA_FALSE;
  STR_FREE(theMessage->targetVendor);
  STR_FREE(theMessage->targetDeviceID);
  STR_FREE(theMessage->targetInstanceID);

  theMessage->isGroupMessage = EINA_FALSE;
  STR_FREE(theMessage->groupName);

  STR_FREE(theMessage->schemaClass);
  STR_FREE(theMessage->schemaType);

  xPL_clearAllNamedValues(theMessage->messageBody);

  /* Clear header info */
  theMessage->messageType = xPL_MESSAGE_ANY;
  theMessage->hopCount = 0;
  theMessage->receivedMessage = EINA_FALSE;

  /* Free Message */
  xPL_FreeMessage(theMessage);
}

/* Write out the message */
String
xPL_formatMessage(xPL_MessagePtr theMessage)
 {
  xPL_NameValueListPtr nvList = xPL_getMessageBody(theMessage);
  xPL_NameValuePairPtr nvPair = NULL;
  int nvIndex = 0;
  int nvCount = xPL_getNamedValueCount(nvList);

  /* Init the write buffer */
  messageBytesWritten = 0;

  /* Write header */
  switch (theMessage->messageType)
  {
  case xPL_MESSAGE_COMMAND:
    WRITE_TEXT("xpl-cmnd");
    break;
  case xPL_MESSAGE_STATUS:
    WRITE_TEXT("xpl-stat");
    break;
  case xPL_MESSAGE_TRIGGER:
    WRITE_TEXT("xpl-trig");
    break;
  default:
    debug(MSG_XPL, _("Can't format xPL message -- invalid/unknown message type '%d'"), theMessage->messageType);
    return NULL;
  }

  /* Write hop and source info */
  WRITE_TEXT("\n{\nhop=1\nsource=");
  WRITE_TEXT(xPL_getSourceVendor(theMessage));
  WRITE_TEXT("-");
  WRITE_TEXT(xPL_getSourceDeviceID(theMessage));
  WRITE_TEXT(".");
  WRITE_TEXT(xPL_getSourceInstanceID(theMessage));
  WRITE_TEXT("\n");

  /* Write target */
  if (xPL_isBroadcastMessage(theMessage))
  {
    WRITE_TEXT("target=*");
  } else {
    if (xPL_isGroupMessage(theMessage))
    {
      WRITE_TEXT("target=XPL-GROUP.");
      WRITE_TEXT(xPL_getTargetGroup(theMessage));
    } else {
      WRITE_TEXT("target=");
      WRITE_TEXT(xPL_getTargetVendor(theMessage));
      WRITE_TEXT("-");
      WRITE_TEXT(xPL_getTargetDeviceID(theMessage));
      WRITE_TEXT(".");
      WRITE_TEXT(xPL_getTargetInstanceID(theMessage));
    }
  }
  WRITE_TEXT("\n}\n");

  /* Write the schema out */
  WRITE_TEXT(xPL_getSchemaClass(theMessage));
  WRITE_TEXT(".");
  WRITE_TEXT(xPL_getSchemaType(theMessage));
  WRITE_TEXT("\n{\n");

  /* Write Name/Value Pairs out */
  for (nvIndex = 0; nvIndex < nvCount; nvIndex++) {
    nvPair = xPL_getNamedValuePairAt(nvList, nvIndex);
    WRITE_TEXT(nvPair->itemName);
    WRITE_TEXT("=");

    /* Write data content out */
    if (nvPair->itemValue != NULL) {
      if (nvPair->isBinary)
	writeBinaryValue(nvPair->itemValue, nvPair->binaryLength);
      else
	WRITE_TEXT(nvPair->itemValue);
    }

    /* Terminate line/entry */
    WRITE_TEXT("\n");
  }

  /* Write message terminator */
  WRITE_TEXT("}\n");

  /* Terminate and return text */
  messageBuff[messageBytesWritten] = '\0';
  return messageBuff;
}

/* Send an xPL message.  If the message is valid and is successfully sent, */
/* EINA_TRUE is returned.                                                       */
Eina_Bool xPL_sendMessage(xPL_MessagePtr theMessage) {
  /* Write the message to text */
  if (xPL_formatMessage(theMessage) == NULL) return EINA_FALSE;

  /* Attempt to brodcast it */
  if (!xPL_sendRawMessage(messageBuff, messageBytesWritten)) return EINA_FALSE;

  /* And we are done */
  return EINA_TRUE;
}

#define VALID_CHAR(theChar) (((theChar >= 32) && (theChar < 123)) || (theChar = 124) || (theChar = 126))

/* Parse data until end of block as a block.  If the block is valid, then the number of bytes */
/* parsed is returned.  If there is an error, a negated number of bytes read thus far is      */
/* returned (ABS of this number points to the failing character)                              */
/* If we run out of bytes before we start a new block, it's likely end of stream garbage and  */
/* we return 0 (which means parsing this message is done)                                     */
static int
parseBlock(String theText, String *blockHeader, xPL_NameValueListPtr nameList, Eina_Bool forceUpperCase)
{
  int curState = 0, curIndex, theLength = strlen(theText);
  char theChar;
  String headerBuff = blockHeaderBuff;
  String nameBuff = NULL, valueBuff = NULL;
  Eina_Bool isBinaryValue = EINA_FALSE, blockStarted = EINA_FALSE;
  xPL_NameValuePairPtr theNameValue;

  /* Parse character by character */
  for (curIndex = 0; curIndex < theLength; curIndex++) {
    theChar = theText[curIndex];

    /* Convert identifiers to upper case */
    /* if (((curState != 4) || forceUpperCase) && (theChar >= 97) && (theChar <= 122)) theChar -= 32; */
    if (forceUpperCase && (theChar >= 97) && (theChar <= 122)) theChar -= 32;

    switch(curState) {
    case 0:
      /* Handle an LF transition */
      if ((theChar == '\n') && blockStarted) {
	*headerBuff = '\0';
	curState = 1;
	continue;
      }

      /* Handle leading junk chars */
      if (!blockStarted && (theChar <= 32)) continue;

      /* Handle known good characters */
      if (VALID_CHAR(theChar)) {
	/* Handle normal letters */
	blockStarted = EINA_TRUE;
	*headerBuff++ = theChar;
	continue;
      }

      /* Handle error */
      debug(MSG_XPL, _("Got invalid character parsing block header - %c at position %d"), theChar, curIndex);
      return -curIndex;

    case 1:
      /* Advance */
      if (theChar == '{') {
	curState = 2;
	continue;
      }

      /* Crapola */
      debug(MSG_XPL, _("Got invalid character parsing start of block - %c at position %d (wanted a {)"), theChar, curIndex);
      return -curIndex;


    case 2:
      /* Advance */
      if (theChar == '\n') {
	curState = 3;
	nameBuff = blockNameBuff;
	continue;
      }

      /* Crapola */
      debug(MSG_XPL, _("Got invalid character parsing start of block -  %c at position %d (wanted a LF)"), theChar, curIndex);
      return -curIndex;

    case 3:
      /* Handle end of name */
      if (theChar == '=') {
	*nameBuff = '\0';
	isBinaryValue = EINA_FALSE;
	valueBuff = blockValueBuff;
	curState = 4;
	continue;
      }

      /* Handle end of binary name */
      if (theChar == '!') {
	*nameBuff = '\0';
	isBinaryValue = EINA_TRUE;
	valueBuff = blockValueBuff;
	curState = 4;
	continue;
      }

      /* Handle end of block */
      if (theChar == '}') {
	curState = 5;
	continue;
      }

      /* Handle normal chars */
      if (VALID_CHAR(theChar)) {
	/* Buffer Name */
	*nameBuff++ = theChar;
	continue;
      }

      /* Bad chararters! */
      debug(MSG_XPL, _("Got invalid character parsing block name/value name -  %c at position %d"), theChar, curIndex);
      return -curIndex;

    case 4:
      /* Handle end of line */
      if (theChar == '\n') {
	*valueBuff = '\0';

	/* Save off name/value pair */
	theNameValue = xPL_newNamedValuePair(nameList, blockNameBuff);
	theNameValue->isBinary = isBinaryValue;
	if (!isBinaryValue)
	  theNameValue->itemValue = xPL_StrDup(blockValueBuff);
	else
	{
	  if ((theNameValue->itemValue = textToBinary(blockValueBuff, &theNameValue->binaryLength)) == NULL)
	  {
	    debug(MSG_XPL, _("Can't xlate binary value for name %s"), blockValueBuff);
	    return -curIndex;
	  }
	}

	/* Reset things */
	curState = 3;
	nameBuff = blockNameBuff;
	continue;
      }

      /* Handle normal characters */
      if (VALID_CHAR(theChar) || (theChar == 32)) {
	/* Buffer char */
	*valueBuff++ = theChar;
	continue;
      }

      /* Bad character! */
      debug(MSG_XPL, _("Got invalid character parsing name/value value -  %c at position %d"), theChar, curIndex);
      return -curIndex;

    case 5:
      /* Should be an EOL - we are done if so */
      if (theChar == '\n') {
	/* Copy off block header */
	*blockHeader = xPL_StrDup(blockHeaderBuff);

	/* And we are done */
	return curIndex + 1;
      }

      /* Bad data */
      debug(MSG_XPL, _("Got invalid character parsing end of name/value -  %c at position %d (wanted a LF)"), theChar, curIndex);
      return -curIndex;
    }
    break;
  }

  /* If we didn't start a block, then it's just end of the stream */
  if (!blockStarted) return 0;

  /* If we got here, we ran out of characters - this is an error too */
  debug(MSG_XPL, _("Ran out of characters parsing block"));
  return -theLength;
}

/* Parse the name/value pairs for this message.  If they are all found and valid, then */
/* we return EINA_TRUE.  Otherwise, EINA_FALSE.                                                  */
static Eina_Bool
parseMessageHeader(xPL_MessagePtr theMessage, xPL_NameValueListPtr nameValueList)
{
  int hopCount;
  String dashPtr = NULL, periodPtr = NULL;
  xPL_NameValuePairPtr theNameValue;
  String theVendor, theDeviceID, theInstanceID;
  char groupNameBuffer[40];

  /* Parse the hop count */
  if ((theNameValue = xPL_getNamedValuePair(nameValueList, "HOP")) == NULL) {
    debug(MSG_XPL, _("Message missing HOP count"));
    return EINA_FALSE;
  }
  if (!xPL_strToInt(theNameValue->itemValue, &hopCount) || (hopCount < 1)) {
    debug(MSG_XPL, _("Message HOP Count invalid"));
    return EINA_FALSE;
  }
  theMessage->hopCount = hopCount;

  /* Parse the source */
  if ((theNameValue = xPL_getNamedValuePair(nameValueList, "SOURCE")) == NULL) {
    debug(MSG_XPL, _("Message missing SOURCE"));
    return EINA_FALSE;
  }
  theVendor = theNameValue->itemValue;
  if ((theDeviceID = strchr(theVendor, '-')) == NULL) {
    debug(MSG_XPL, _("SOURCE Missing Device ID - %s"), theVendor);
    return EINA_FALSE;
  }

  dashPtr = theDeviceID;
  *theDeviceID++ = '\0';
  if ((theInstanceID = strchr(theDeviceID, '.')) == NULL) {
    debug(MSG_XPL, _("SOURCE Missing Instance ID - %s.%s"), theVendor, theDeviceID);
    return EINA_FALSE;
  }

  periodPtr = theInstanceID;
  *theInstanceID++ = '\0';

  /* Install source into message */
  xPL_setSource(theMessage, theVendor, theDeviceID, theInstanceID);

  /* Fix mangled string */
  if (dashPtr != NULL) *dashPtr = '-';
  if (periodPtr !=NULL) *periodPtr = '.';


  /* Parse the target (if anything) */
  if ((theNameValue = xPL_getNamedValuePair(nameValueList, "TARGET")) == NULL) {
    debug(MSG_XPL, _("Message missing TARGET"));
    return EINA_FALSE;
  }

  /* Parse the target */
  dashPtr = NULL;
  periodPtr = NULL;

  /* Check for a wildcard */
  if (!strcmp(theNameValue->itemValue, "*")) {
    xPL_setBroadcastMessage(theMessage, EINA_TRUE);
  } else {
    /* Parse vendor and such */
    theVendor = theNameValue->itemValue;
    if ((theDeviceID = strchr(theVendor, '-')) == NULL) {
      debug(MSG_XPL, _("TARGET Missing Device ID - %s"), theVendor);
      return EINA_FALSE;
    }

    dashPtr = theDeviceID;
    *theDeviceID++ = '\0';
    if ((theInstanceID = strchr(theDeviceID, '.')) == NULL) {
      debug(MSG_XPL, _("TARGET Missing Instance ID - %s.%s"), theVendor, theDeviceID);
      return EINA_FALSE;
    }

    periodPtr = theInstanceID;
    *theInstanceID++ = '\0';

    /* See if this was a group message */
    if ((strcasecmp(theVendor, "XPL") == 0) && (strcasecmp(theDeviceID, "GROUP") == 0)) {
      strcpy(groupNameBuffer, "XPL-GROUP.");
      strncat(groupNameBuffer, theInstanceID, 16);
      groupNameBuffer[16] = '\0';
      xPL_setTargetGroup(theMessage, groupNameBuffer);
    } else {
      xPL_setTarget(theMessage, theVendor, theDeviceID, theInstanceID);
    }

    /* Fix mangled string */
    if (dashPtr != NULL) *dashPtr = '-';
    if (periodPtr !=NULL) *periodPtr = '.';
  }

  /* Header parsed OK */
  return EINA_TRUE;
}

/* Convert a text message into a xPL message.  Return the message */
/* or NULL if there is a parse error                              */
xPL_MessagePtr
 parseMessage(String theText)
{
  int textLen = strlen(theText);
  int parsedChars, parsedThisTime;
  String blockHeaderKeyword;
  String blockDelimPtr, periodPtr = NULL;
  xPL_MessagePtr theMessage;

  /* Allocate a message */
  theMessage = createReceivedMessage(xPL_MESSAGE_ANY);

  /* Parse the header */
  if ((parsedThisTime = parseBlock(theText, &blockHeaderKeyword, theMessage->messageBody, EINA_FALSE)) <= 0) {
    debug(MSG_XPL, _("Error parsing message header"));
    xPL_releaseMessage(theMessage);
    return NULL;
  }
  parsedChars = parsedThisTime;

  /* Parse the header */
  if (!strcasecmp(blockHeaderKeyword, "XPL-CMND")) {
    xPL_setMessageType(theMessage, xPL_MESSAGE_COMMAND);
  } else if (!strcasecmp(blockHeaderKeyword, "XPL-STAT")) {
    xPL_setMessageType(theMessage, xPL_MESSAGE_STATUS);
  } else if (!strcasecmp(blockHeaderKeyword, "XPL-TRIG")) {
    xPL_setMessageType(theMessage, xPL_MESSAGE_TRIGGER);
  } else {
    debug(MSG_XPL, _("Unknown message header of %s - bad message"), blockHeaderKeyword);
    STR_FREE(blockHeaderKeyword);
    xPL_releaseMessage(theMessage);
    return NULL;
  }

  /* We are done with this now - drop it while we are still thinking about it */
  STR_FREE(blockHeaderKeyword);

  /* Parse the name/values into the message */
  if (!parseMessageHeader(theMessage, theMessage->messageBody))
  {
    debug(MSG_XPL, _("Can't parse message header"));
    xPL_releaseMessage(theMessage);
    return NULL;
  }

  /* Parse multiple blocks until we are done */
  for(;parsedChars < textLen;) {
    /* Clear the name/value list for the message */
    xPL_clearAllNamedValues(theMessage->messageBody);
    periodPtr = NULL;

    /* Parse the next block */
    if ((parsedThisTime = parseBlock(&(theText[parsedChars]), &blockHeaderKeyword, theMessage->messageBody, EINA_FALSE)) < 0) {
      debug(MSG_XPL, _("Can't parse message block"));
      xPL_releaseMessage(theMessage);
      STR_FREE(blockHeaderKeyword);
      return NULL;
    }

    /* If we ran out of characters, no more blocks */
    if (parsedThisTime == 0) break;

    /* Up Parsed count */
    parsedChars += parsedThisTime;

    /* Parse the block header */
    if ((blockDelimPtr = strchr(blockHeaderKeyword, '.')) == NULL) {
      debug(MSG_XPL, _("Malformed message block header - %s"), blockHeaderKeyword);
      xPL_releaseMessage(theMessage);
      STR_FREE(blockHeaderKeyword);
      return NULL;
    }
    periodPtr = blockDelimPtr;
    *blockDelimPtr++ = '\0';

    /* Record the message schema class/type */
    xPL_setSchemaClass(theMessage, blockHeaderKeyword);
    xPL_setSchemaType(theMessage, blockDelimPtr);

    /* Fix mangled string & release string */
    if (periodPtr != NULL) *periodPtr = '.';
    STR_FREE(blockHeaderKeyword);
    break;
  }

  /* Return the message */
  return theMessage;
}

/* Check to see if the passed message is a hub echo.  */
static Eina_Bool isHubEcho(xPL_MessagePtr theMessage) {
  String remoteIP, thePort;

  if (theMessage == NULL) return EINA_FALSE;

  /* If this is not a heartbeat, ignore it */
  if (!(!strcasecmp(theMessage->schemaClass, "hbeat")
     || !strcasecmp(theMessage->schemaClass, "config"))) return EINA_FALSE;

  /* Insure it has an IP address and port */
  if ((remoteIP = xPL_getMessageNamedValue(theMessage, "remote-ip")) == NULL) return EINA_FALSE;
  if ((thePort = xPL_getMessageNamedValue(theMessage, "port")) == NULL) return EINA_FALSE;

  /* Now See if the IP address & port matches ours */
  if (strcmp(remoteIP, xPL_getListenerIPAddr())) return EINA_FALSE;
  if (strcmp(thePort, xPL_intToStr(xPL_getPort()))) return EINA_FALSE;

  /* Clearly this is a message from us */
  return EINA_TRUE;
}

/* Read, parse and dispatch an xPL message */
void
xPL_receiveMessage(int theFD __UNUSED__, int thePollInfo __UNUSED__, int userValue __UNUSED__)
{
  int bytesRead;
  xPL_MessagePtr theMessage = NULL;

  for (;;) {
    /* Fetch the next message, if any */
    if ((bytesRead = recvfrom(xPLFD, &messageBuff, MSG_BUFF_SIZE - 1, 0, NULL, NULL)) < 0) {
      /* Expected response when queue is empty */
      if (errno == EAGAIN) return;

      /* Note the error and bail */
      debug(MSG_XPL, _("Can't read xPL message from network - %s (%d)"), strerror(errno), errno);
      return;
    }

    /* We receive a message - clean it up */
    messageBuff[bytesRead] = '\0';

    /* Send the raw message to any raw message listeners */
    xPL_dispatchRawEvent(messageBuff, bytesRead);

    /* Parse the message */
    if ((theMessage = parseMessage(messageBuff)) == NULL) {
      debug(MSG_XPL, _("Can't parse network message - ignored"));
      continue;
    }

    /* See if we need to check the message for hub detection */
    if (!hubConfirmed && isHubEcho(theMessage))
    {
      hubConfirmed = EINA_TRUE;
    }

    /* Dispatch the message */
    xPL_dispatchMessageEvent(theMessage);

    /* Release the message */
    xPL_releaseMessage(theMessage);
  }
}
