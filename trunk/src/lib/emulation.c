/*
 * emulation.c
 * This file is part of EDAMS
 *
 * Copyright (C) 2013 - Alexandre Dussart
 *
 * Based on xPLSend.c - Command Line xPL message sending tool
 * Copyright (c) 2005, Gerald R Duprey Jr.
 *
 *
 * EDAMS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * EDAMS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EDAMS. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <time.h>
#include <signal.h>
#include "xPL.h"

char msgSource[64] = "cdp1802-xplsend.default";
String srcVendor = NULL;
String srcDeviceID = NULL;
String srcInstanceID = NULL;

String msgTarget = NULL;
String tgtVendor = NULL;
String tgtDeviceID = NULL;
String tgtInstanceID = NULL;

xPL_MessageType msgType = xPL_MESSAGE_COMMAND;
String msgSchemaClass = NULL;
String msgSchemaType = NULL;

/* Print usage info */
void printUsage(String commandName) {
  fprintf(stderr, "%s - xPL Message Sender\n", commandName);
  fprintf(stderr, "Copyright (c) 2005, Gerald R Duprey Jr\n\n");
  fprintf(stderr, "Usage: %s [-s source] [-b] [-t target] [-m msgType] -c schema  name=value name=value ...\n", commandName);
  fprintf(stderr, "  -s source - source of message in vendor-device.instance (default: cdp1802-xplsend.default)\n");
  fprintf(stderr, "  -b send broadcast message (default)\n");
  fprintf(stderr, "  -t target - target of message in vendor-device.instance format.  (default: broadcast)\n");
  fprintf(stderr, "  -m message type: cmnd, trig or stat (default: cmnd)\n");
  fprintf(stderr, "  -c schema class and type formatted as class.type - REQUIRED\n\n");
}

Eina_Bool
parseSourceIdent()
{
  String dashPtr, periodPtr;

  /* Make sure we have something to work with */
  if ((msgSource == NULL) || (strlen(msgSource) < 5)) {
    fprintf(stderr, "Empty or too short message source ID\n");
    return EINA_FALSE;
  }

  /* Locate the delimiters */
  if ((dashPtr = strstr(msgSource, "-")) == NULL) {
    fprintf(stderr, "Missing dash in source ident -- invalid source ID\n");
    return EINA_FALSE;
  }
  if ((periodPtr = strstr(dashPtr, ".")) == NULL) {
    fprintf(stderr, "Missing period in source ident -- invalid source ID\n");
    return EINA_FALSE;
  }

  /* Install pointers */
  *dashPtr++ = '\0';
  *periodPtr++ = '\0';
  srcVendor = msgSource;
  srcDeviceID = dashPtr;
  srcInstanceID = periodPtr;

  return EINA_TRUE;
}

Eina_Bool
parseTargetIdent()
{
  String dashPtr, periodPtr;

  /* Make sure we have something to work with */
  if (msgTarget == NULL) return EINA_TRUE;
  if (strlen(msgTarget) < 5) {
    fprintf(stderr, "Empty or too short message target ID\n");
    return EINA_FALSE;
  }

  /* Locate the delimiters */
  if ((dashPtr = strstr(msgTarget, "-")) == NULL) {
    fprintf(stderr, "Missing dash in target ident -- invalid target ID\n");
    return EINA_FALSE;
  }
  if ((periodPtr = strstr(dashPtr, ".")) == NULL) {
    fprintf(stderr, "Missing period in target ident -- invalid target ID\n");
    return EINA_FALSE;
  }

  /* Install pointers */
  *dashPtr++ = '\0';
  *periodPtr++ = '\0';
  tgtVendor = msgTarget;
  tgtDeviceID = dashPtr;
  tgtInstanceID = periodPtr;

  return EINA_TRUE;
}

/** Parse command line for switches */
/** -s - source of message ident */
/** -t - target of message ident */
/** -b - target is broadcast */
/** -m - message type */
/** -c - schema class/type */
Eina_Bool
parseCmdLine( int *argc, char *argv[])
 {
  int swptr;
  int newcnt = 0;
  String delimPtr;


  /* Handle each item of the command line.  If it starts with a '-', then */
  /* process it as a switch.  If not, then copy it to a new position in   */
  /* the argv list and up the new parm counter.                           */
  for(swptr = 0; swptr < *argc; swptr++) {
    /* If it doesn't begin with a '-', it's not a switch. */
    if (argv[swptr][0] != '-') {
      if (swptr != newcnt) argv[++newcnt] = argv[swptr];
    }
    else {
      if (!strcmp(argv[swptr],"-s")) {
	swptr++;
	strcpy(msgSource, argv[swptr]);
	continue;
      }

      if (!strcmp(argv[swptr],"-t")) {
	swptr++;
	msgTarget = argv[swptr];
	continue;
      }

      if (!strcmp(argv[swptr],"-b")) {
	msgTarget = NULL;
	continue;
      }

      if (!strcmp(argv[swptr],"-m")) {
	swptr++;

	if (strcasecmp(argv[swptr], "cmnd") == 0) {
	  msgType = xPL_MESSAGE_COMMAND;
	} else if (strcasecmp(argv[swptr], "trig") == 0) {
	  msgType = xPL_MESSAGE_TRIGGER;
	} else if (strcasecmp(argv[swptr], "stat") == 0) {
	  msgType = xPL_MESSAGE_STATUS;
	} else {
	  fprintf(stderr, "Unknown message type of %s for -m", argv[swptr]);
	  return EINA_FALSE;
	}

	continue;
      }

      if (!strcmp(argv[swptr],"-c")) {
	swptr++;

	if ((delimPtr = strstr(argv[swptr], ".")) == NULL) {
	  fprintf(stderr, "Improperly formatted schema class.type of %s", argv[swptr]);
	  return EINA_FALSE;
	}

	*delimPtr++ = '\0';
	msgSchemaClass = strdup(argv[swptr]);
	msgSchemaType = strdup(delimPtr);
	continue;
      }


      /* Anything left is unknown */
      fprintf(stderr, "Unknown switch `%s'", argv[swptr] );
      return EINA_FALSE;
    }
  }

  /* Set in place the new argument count and exit */
  *argc = newcnt + 1;
  return EINA_TRUE;
}


Eina_Bool sendMessage(int argc, String argv[]) {
  int argIndex = 0;
  xPL_ServicePtr theService = NULL;
  xPL_MessagePtr theMessage = NULL;
  String delimPtr;

  /* Create service so we can create messages */
  if ((theService = xPL_createService(srcVendor, srcDeviceID, srcInstanceID)) == NULL) {
    fprintf(stderr, "Unable to create xPL service\n");
    return EINA_FALSE;
  }

  /* Create an appropriate message */
  if (msgTarget == NULL) {
    if ((theMessage = xPL_createBroadcastMessage(theService, msgType)) == NULL) {
      fprintf(stderr, "Unable to create broadcast message\n");
      return EINA_FALSE;
    }
  } else {
    if ((theMessage = xPL_createTargetedMessage(theService, msgType, tgtVendor, tgtDeviceID, tgtInstanceID)) == NULL) {
      fprintf(stderr, "Unable to create targetted message\n");
      return EINA_FALSE;
    }
  }

  /* Install the schema */
  xPL_setSchema(theMessage, msgSchemaClass, msgSchemaType);

  /* Install named values */
  for (argIndex = 1; argIndex < argc; argIndex++) {
    if ((delimPtr = strstr(argv[argIndex], "=")) == NULL) {
      fprintf(stderr, "Improperly formatted name/value pair %s\n", argv[argIndex]);
      return EINA_FALSE;
    }

    /* Break them up  and add it */
    *delimPtr++ = '\0';
    xPL_addMessageNamedValue(theMessage, argv[argIndex], delimPtr);
  }

  /* Send the message */
  if (!xPL_sendMessage(theMessage)) {
    fprintf(stderr, "Unable to send xPL message\n");
    return EINA_FALSE;
  }

  return EINA_TRUE;
}


void
start_emulation_test(int ac, char *av[])
{
  /* Handle a plea for help */
  if (ac == 1) {
    printUsage(av[0]);
    return;
  }

  /* Parse xPL & program command line parms */
  if (!parseCmdLine(&ac, av)) return;

  /* Ensure we have a class */
  if ((msgSchemaClass == NULL) || (msgSchemaType == NULL))
  {
    fprintf(stderr, "The -c schema class.type is REQUIRED\n");
    return ;
  }

  /* Start xPL up */
  if (!xPL_initialize(xPL_getParsedConnectionType())) {
    fprintf(stderr, "Unable to start xPL");
    return;
  }

  /* Parse the source */
  if (!parseSourceIdent()) return;

  /* Parse the target */
  if (!parseTargetIdent()) return;

  /* Send the message */
  if (!sendMessage(ac, av)) return;
  return;
}
