/* xPL.h - xPL Public API */
/* Copyright 2004 (c), Gerald R Duprey Jr */


#ifndef __XPL_H
#define __XPL_H

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#include <Eina.h>
#include <Ecore.h>

#include "widget.h"


#define XPL_VERSION "20091005"
#define XPLLIB_VERSION "V1.3a"

#define BASE_XPL_PORT 3865

#define DEFAULT_HEARTBEAT_INTERVAL 300
#define CONFIG_HEARTBEAT_INTERVAL 60
#define HUB_DISCOVERY_INTERVAL 3

typedef char * String;

#ifndef INADDR_NONE
  #define INADDR_NONE 0xffffffff
#endif

#define SYSCALL(call) while(((call) == -1) && (errno == EINTR))

/* xPL Connection mode */
typedef enum {xcStandAlone, xcViaHub, xcAuto} xPL_ConnectType;

/* Pointer to nothing in particular */
typedef void * xPL_ObjectPtr;

/* A discrete name/value structure */
typedef struct
{
  char * itemName;
  char * itemValue;
  Eina_Bool isBinary;
  int binaryLength;
} xPL_NameValuePair, *xPL_NameValuePairPtr;

/* A list of name/value pairs */
typedef struct
{
  int namedValueCount;
  int namedValueAlloc;
  xPL_NameValuePairPtr *namedValues;
} xPL_NameValueList, *xPL_NameValueListPtr;

/* Possible xPL message types */
typedef enum { xPL_MESSAGE_ANY, xPL_MESSAGE_COMMAND, xPL_MESSAGE_STATUS, xPL_MESSAGE_TRIGGER } xPL_MessageType;

/* Describe a received message */
typedef struct
{
  xPL_MessageType messageType;
  int hopCount;
  Eina_Bool receivedMessage; /* TRUE if received, FALSE if being sent */

  char * sourceVendor;
  char * sourceDeviceID;
  char * sourceInstanceID;

  Eina_Bool isGroupMessage;
  char * groupName;

  Eina_Bool isBroadcastMessage;
  char * targetVendor;
  char * targetDeviceID;
  char * targetInstanceID;

  char * schemaClass;
  char * schemaType;

  xPL_NameValueListPtr messageBody;
} xPL_Message, *xPL_MessagePtr;

typedef struct
{
  xPL_MessageType matchOnMessageType;
  char * matchOnVendor;
  char * matchOnDeviceID;
  char * matchOnInstanceID;
  char * matchOnSchemaClass;
  char * matchOnSchemaType;
} xPL_ServiceFilter, *xPL_ServiceFilterPtr;

typedef enum { xPL_CONFIG_OPTIONAL, xPL_CONFIG_MANDATORY, xPL_CONFIG_RECONF } xPL_ConfigurableType;

typedef struct
{
  char * itemName;
  xPL_ConfigurableType itemType;
  int maxValueCount;

  int valueCount;
  int valueAllocCount;
  char * *valueList;
} xPL_ServiceConfigurable, *xPL_ServiceConfigurablePtr;


typedef struct _xPL_Service xPL_Service;
typedef struct _xPL_Service *xPL_ServicePtr;

/** Changes to a services configuration **/
typedef void (* xPL_ServiceConfigChangedListener) (xPL_ServicePtr, xPL_ObjectPtr);

/** Service Listener Support **/
typedef void (* xPL_ServiceListener)(xPL_ServicePtr, xPL_MessagePtr, xPL_ObjectPtr);

typedef struct {
  xPL_MessageType matchMessageType;
  char * matchSchemaClass;
  char * matchSchemaType;
  xPL_ObjectPtr userValue;
  xPL_ServiceListener serviceListener;
} xPL_ServiceListenerDef, *xPL_ServiceListenerDefPtr;

typedef struct {
  xPL_ServiceConfigChangedListener changeListener;
  xPL_ObjectPtr userValue;
} xPL_ServiceChangedListenerDef, *xPL_ServiceChangedListenerDefPtr;

/* Describe a xPL service */
struct _xPL_Service {
  Eina_Bool serviceEnabled;

  char * serviceVendor;
  char * serviceDeviceID;
  char * serviceInstanceID;

  char * serviceVersion;

  int groupCount;
  int groupAllocCount;
  char * *groupList;

  Eina_Bool ignoreBroadcasts;

  int heartbeatInterval;
  time_t lastHeartbeatAt;
  xPL_MessagePtr heartbeatMessage;

  Eina_Bool configurableService;
  Eina_Bool serviceConfigured;
  char * configFileName;
  int configChangedCount;
  int configChangedAllocCount;
  xPL_ServiceChangedListenerDefPtr changedListenerList;

  int configCount;
  int configAllocCount;
  xPL_ServiceConfigurablePtr configList;

  int filterCount;
  int filterAllocCount;
  xPL_ServiceFilterPtr messageFilterList;

  Eina_Bool reportOwnMessages;
  int listenerCount;
  int listenerAllocCount;
  xPL_ServiceListenerDefPtr serviceListenerList;
};

typedef enum _Xpl_Type
{
	XPL_TYPE_UNKNOWN	    	        = (0),
	XPL_TYPE_BATTERY_SENSOR_BASIC	    = (1),  /*battery - a battery level in percent.*/
	XPL_TYPE_COUNT_SENSOR_BASIC		    = (2),  /*count - a counter value (door openings, rain fall, etc).*/
	XPL_TYPE_CURRENT_SENSOR_BASIC	    = (3),	/*current - a current value in Amps.*/
	XPL_TYPE_DIRECTION_SENSOR_BASIC	    = (4),  /*direction - direction, represented as degrees from north (0-360, 0=north, 180=south, etc)*/
	XPL_TYPE_DISTANCE_SENSOR_BASIC	    = (5),  /*distance - distance measurments. Default unit of measure is meters.*/
	XPL_TYPE_ENERGY_SENSOR_BASIC	    = (6),  /*energy - consumption of energy over a preiod of time in kWh (kilowatt hours).*/
	XPL_TYPE_FAN_SENSOR_BASIC		    = (7),  /*fan - a fan speed in RPM.*/
	XPL_TYPE_GENERIC_SENSOR_BASIC	    = (8),  /*generic - a generic analogue value who's units of measurement are application specific*/
	XPL_TYPE_HUMIDITY_SENSOR_BASIC	    = (9),  /*humidity - a relative humidity percentage (0 to 100, no percent sign).*/
	XPL_TYPE_INPUT_SENSOR_BASIC		    = (10), /*input - a switch that can either be current=HIGH (on), current=LOW (off) or current=PULSE (representing a button press)*/
	XPL_TYPE_OUTPUT_SENSOR_BASIC	    = (11), /*output - a change in an output state with values of LOW and HIGH*/
	XPL_TYPE_POWER_SENSOR_BASIC		    = (12), /*power - instantaneous energy consumption level in kW*/
	XPL_TYPE_PRESSURE_SENSOR_BASIC	    = (13), /*pressure - a pressure value in Pascals (N/m2)*/
	XPL_TYPE_SETPOINT_SENSOR_BASIC	    = (14), /*setpoint - a thermostat threshold temperature value in degrees. Default unit of measure is centigrade/celsius.*/
	XPL_TYPE_SPEED_SENSOR_BASIC		    = (15),	/*speed - a generic speed. Default unit of measure is Miles per Hour.*/
	XPL_TYPE_TEMP_SENSOR_BASIC		    = (16),	/*temp - a temperature value in degrees. Default unit of measure is centigrade celsius.*/
	XPL_TYPE_UV_SENSOR_BASIC            = (17), /*uv - UV Index (with no units). See http://en.wikipedia.org/wiki/UV_index.*/
	XPL_TYPE_VOLTAGE_SENSOR_BASIC	    = (18), /*voltage - a voltage value in Volts.*/
	XPL_TYPE_VOLUME_SENSOR_BASIC	    = (19), /*volume - a volume in m3. Often used as a measure of gas and water consumption.*/
	XPL_TYPE_WEIGHT_SENSOR_BASIC 	    = (20), /*weight - the default unit is kilograms (yes, kilograms are a unit of mass, not weight)*/
	XPL_TYPE_BALANCE_CONTROL_BASIC      = (21), /*balance - -100 to +100.*/
	XPL_TYPE_FLAG_CONTROL_BASIC		    = (22), /*flag - set, clear, neutral.*/
	XPL_TYPE_INFRARED_CONTROL_BASIC	    = (23), /*infrared - send, enable_rx, disable_rx, enable_tx, disable_tx, sendx (send x times).*/
	XPL_TYPE_INPUT_CONTROL_BASIC	    = (24), /*input - enable, disable.*/
	XPL_TYPE_MACRO_CONTROL_BASIC	    = (25), /*macro - enable, disable, do.*/
	XPL_TYPE_MUTE_CONTROL_BASIC		    = (26), /*mute - yes, no.*/
	XPL_TYPE_OUTPUT_CONTROL_BASIC	    = (27), /*output - enable, disable, high, low, toggle, pulse.*/
	XPL_TYPE_VARIABLE_CONTROL_BASIC	    = (28), /*variable - inc, dec, 0-255 (for set).*/
    XPL_TYPE_PERIODIC_CONTROL_BASIC	    = (29), /*periodic - started, enable, disable.*/
	XPL_TYPE_SCHEDULED_CONTROL_BASIC    = (30), /*scheduled - started, enable, disable.*/
	XPL_TYPE_SLIDER_CONTROL_BASIC	    = (31), /*slider -	nn = set to value (0-255),*/
									            /* +nn = increment by nn, -nn = decrement by nn,*/
											    /*n% = set to nn (where nn is a percentage - 0-100%)*/
	XPL_TYPE_TIMER_CONTROL_BASIC        = (32), /*timer - went off, start, stop, halt, resume.*/
	XPL_TYPE_LAST
}Xpl_Type;



/*xPL misc stuff*/
Eina_Bool xpl_init();
Eina_Bool xpl_shutdown();
Ecore_Pipe *xpl_start();

Eina_Bool xpl_control_basic_cmnd_send(Widget *widget);
void xpl_services_install(Ecore_Pipe *pipe);
void xpl_process_messages();

const char *xpl_control_basic_cmnd_to_elm_str(Widget *widget);
const char *xpl_type_to_str(Xpl_Type type);
Xpl_Type xpl_str_to_type(const char *xpl_type);
const char *xpl_type_to_units(Xpl_Type type);
const char *xpl_type_to_unit_symbol(Xpl_Type type);
int xpl_type_current_min_get(Xpl_Type type);
int xpl_type_current_max_get(Xpl_Type type);

Eina_List *xpl_sensor_basic_list_get();


/* xPL Service Support */
extern xPL_ServicePtr xPL_createService(char *, char *, char *);
extern void xPL_releaseService(xPL_ServicePtr);

extern void xPL_setServiceEnabled(xPL_ServicePtr, Eina_Bool);
extern Eina_Bool xPL_isServiceEnabled(xPL_ServicePtr);

extern void xPL_setServiceVendor(xPL_ServicePtr, char *);
extern char * xPL_getServiceVendor(xPL_ServicePtr);

extern void xPL_setServiceDeviceID(xPL_ServicePtr, char *);
extern char * xPL_getServiceDeviceID(xPL_ServicePtr);

extern void xPL_setServiceInstanceID(xPL_ServicePtr, char *);
extern char * xPL_getServiceInstanceID(xPL_ServicePtr);

extern void xPL_setRespondingToBroadcasts(xPL_ServicePtr, Eina_Bool);
extern Eina_Bool xPL_isRespondingToBroadcasts(xPL_ServicePtr);

extern void xPL_setServiceVersion(xPL_ServicePtr, char *);
extern char * xPL_getServiceVersion(xPL_ServicePtr);

extern void xPL_setReportOwnMessages(xPL_ServicePtr, Eina_Bool);
extern Eina_Bool xPL_isReportOwnMessages(xPL_ServicePtr);

extern Eina_Bool xPL_isServiceFiltered(xPL_ServicePtr);
extern void xPL_clearServiceFilters(xPL_ServicePtr);

extern Eina_Bool xPL_doesServiceHaveGroups(xPL_ServicePtr);
extern void xPL_clearServiceGroups(xPL_ServicePtr);

extern void xPL_setHeartbeatInterval(xPL_ServicePtr, int);
extern int xPL_getHeartbeatInterval(xPL_ServicePtr);
extern void xPL_setTimelyHeartbeats();

extern Eina_Bool xPL_sendServiceMessage(xPL_ServicePtr, xPL_MessagePtr);

extern int xPL_getServiceCount();
extern xPL_ServicePtr xPL_getServiceAt(int);

/* xPL Service Configuratuion Support */
extern xPL_ServicePtr xPL_createConfigurableService(char *, char *, char *);

extern Eina_Bool xPL_isConfigurableService(xPL_ServicePtr);
extern Eina_Bool xPL_isServiceConfigured(xPL_ServicePtr);

extern char * xPL_getServiceConfigFile(xPL_ServicePtr);

extern Eina_Bool xPL_addServiceConfigurable(xPL_ServicePtr, char *, xPL_ConfigurableType, int);
extern Eina_Bool xPL_removeServiceConfigurable(xPL_ServicePtr, char *);
extern void xPL_removeAllServiceConfigurables(xPL_ServicePtr);

extern void xPL_clearServiceConfigValues(xPL_ServicePtr, char *);
extern void xPL_clearAllServiceConfigValues(xPL_ServicePtr);

extern Eina_Bool xPL_addServiceConfigValue(xPL_ServicePtr, char *, char *);
extern void xPL_setServiceConfigValueAt(xPL_ServicePtr, char *, int, char *);
extern void xPL_setServiceConfigValue(xPL_ServicePtr, char *, char *);

extern int xPL_getServiceConfigValueCount(xPL_ServicePtr, char *);
extern char * xPL_getServiceConfigValueAt(xPL_ServicePtr, char *, int);
extern char * xPL_getServiceConfigValue(xPL_ServicePtr, char *);

extern void xPL_addServiceConfigChangedListener(xPL_ServicePtr, xPL_ServiceConfigChangedListener, xPL_ObjectPtr);
extern Eina_Bool xPL_removeServiceConfigChangedListener(xPL_ServicePtr, xPL_ServiceConfigChangedListener);


/* Message support */

/* Accessors for messages (getters/setters).  Remember that you must NOT */
/* modify the returned value of an getter and that all setter values are */
/* copied off (i.e. future changes to the passed parameter will not be */
/* reflected in the message).  Finally, all char * values are convered to */
/* upper case, except where noted.                                        */
extern void xPL_setMessageType(xPL_MessagePtr, xPL_MessageType);
extern xPL_MessageType xPL_getMessageType(xPL_MessagePtr);

extern int xPL_getHopCount(xPL_MessagePtr);
extern Eina_Bool xPL_isReceivedMessage(xPL_MessagePtr);

extern void xPL_setBroadcastMessage(xPL_MessagePtr, Eina_Bool);
extern Eina_Bool xPL_isBroadcastMessage(xPL_MessagePtr);

extern void xPL_setTargetGroup(xPL_MessagePtr, char *);
extern char * xPL_getTargetGroup(xPL_MessagePtr);
extern Eina_Bool xPL_isGroupMessage(xPL_MessagePtr);

extern void xPL_setTargetVendor(xPL_MessagePtr, char *);
extern char * xPL_getTargetVendor(xPL_MessagePtr);

extern void xPL_setTargetDeviceID(xPL_MessagePtr, char *);
extern char * xPL_getTargetDeviceID(xPL_MessagePtr);

extern void xPL_setTargetInstanceID(xPL_MessagePtr, char *);
extern char * xPL_getTargetInstanceID(xPL_MessagePtr);

extern void xPL_setTarget(xPL_MessagePtr, char *, char *, char *);

extern void xPL_setSourceVendor(xPL_MessagePtr, char *);
extern char * xPL_getSourceVendor(xPL_MessagePtr);

extern void xPL_setSourceDeviceID(xPL_MessagePtr, char *);
extern char * xPL_getSourceDeviceID(xPL_MessagePtr);

extern void xPL_setSourceInstanceID(xPL_MessagePtr, char *);
extern char * xPL_getSourceInstanceID(xPL_MessagePtr);

extern void xPL_setSource(xPL_MessagePtr, char *, char *, char *);

extern void xPL_setSchemaClass(xPL_MessagePtr, char *);
extern char * xPL_getSchemaClass(xPL_MessagePtr);
extern void xPL_setSchemaType(xPL_MessagePtr, char *);
extern char * xPL_getSchemaType(xPL_MessagePtr);
extern void xPL_setSchema(xPL_MessagePtr, char *, char *);

extern xPL_NameValueListPtr xPL_getMessageBody(xPL_MessagePtr);
extern Eina_Bool xPL_doesMessageNamedValueExist(xPL_MessagePtr, char *);
extern char * xPL_getMessageNamedValue(xPL_MessagePtr, char *);

extern void xPL_clearMessageNamedValues(xPL_MessagePtr);
extern void xPL_addMessageNamedValue(xPL_MessagePtr, char *, char *);
extern void xPL_setMessageNamedValue(xPL_MessagePtr, char *, char *);
extern void xPL_setMessageNamedValues(xPL_MessagePtr, ...);

extern xPL_MessagePtr xPL_createTargetedMessage(xPL_ServicePtr, xPL_MessageType, char *, char *, char *);
extern xPL_MessagePtr xPL_createGroupTargetedMessage(xPL_ServicePtr, xPL_MessageType, char *);
extern xPL_MessagePtr xPL_createBroadcastMessage(xPL_ServicePtr, xPL_MessageType);
extern void xPL_releaseMessage(xPL_MessagePtr);

extern Eina_Bool xPL_sendMessage(xPL_MessagePtr);

extern void xPL_addServiceListener(xPL_ServicePtr, xPL_ServiceListener, xPL_MessageType, char *, char *, xPL_ObjectPtr);
extern Eina_Bool xPL_removeServiceListener(xPL_ServicePtr, xPL_ServiceListener);

/* General Library support */
extern Eina_Bool xPL_initialize(xPL_ConnectType);
extern Eina_Bool xPL_shutdown();

extern int xPL_getFD();
extern int xPL_getPort();
extern xPL_ConnectType xPL_getConnectType();

extern Eina_Bool xPL_isHubConfirmed();

extern char * xPL_formatMessage(xPL_MessagePtr);

extern char * xPL_getBroadcastInterface();
extern void xPL_setBroadcastInterface(char *);
extern char * xPL_getBroadcastIPAddr();

extern char * xPL_getListenerIPAddr();

extern Eina_Bool xPL_processMessages(int);

/* Event handler for user-registered I/O management */
typedef void (* xPL_IOHandler)(int, int, int);
extern Eina_Bool xPL_addIODevice(xPL_IOHandler, int, int, Eina_Bool, Eina_Bool, Eina_Bool);
extern Eina_Bool xPL_removeIODevice(int);

/* Event management of user timeouts */
typedef void (* xPL_TimeoutHandler)(int, xPL_ObjectPtr);
extern void xPL_addTimeoutHandler(xPL_TimeoutHandler, int, xPL_ObjectPtr);
extern Eina_Bool xPL_removeTimeoutHandler(xPL_TimeoutHandler);


/* General utility/helpers */
extern char * xPL_intToHex(int);
extern Eina_Bool xPL_hexToInt(char *, int *);
extern char * xPL_intToStr(int);
extern Eina_Bool xPL_strToInt(char *, int *);
extern xPL_ConnectType xPL_getParsedConnectionType();

/* Name/value list support */

/* Add a new named value to the list */
extern void xPL_addNamedValue(xPL_NameValueListPtr, char *, char *);

/* Update an existing named value or, if the name does not exist, add a new name/value */
extern void xPL_setNamedValue(xPL_NameValueListPtr, char *, char *);

/* Set a series of name/value pairs.  Each name must be followed by a value */
/* even if that value is NULL.                                              */
extern void xPL_setNamedValues(xPL_NameValueListPtr, ...);

/* Return # of name/value pairs in this list */
extern int xPL_getNamedValueCount(xPL_NameValueListPtr);

/* Return a Name/Value pair at the passed index (or NULL for a bad index) */
extern xPL_NameValuePairPtr xPL_getNamedValuePairAt(xPL_NameValueListPtr, int);

/* Given the passed list and name, return the index into the list of the name or -1 */
extern int xPL_getNamedValueIndex(xPL_NameValueListPtr, char *);

/* Given the passed list and name, return the first matching name/value pair or NULL */
extern xPL_NameValuePairPtr xPL_getNamedValuePair(xPL_NameValueListPtr, char *);

/* Given the passed list and name, return the first matching value or NULL */
/* Note: Value can be NULL because it was not found or because the actual */
/* value of the name/value pair is NULL.  If this matters, use doesNamedValueExist() */
extern char * xPL_getNamedValue(xPL_NameValueListPtr, char *);

/* Given the passed list and name, return TRUE if the named value exists in the list */
extern Eina_Bool xPL_doesNamedValueExist(xPL_NameValueListPtr, char *);

/* Remove name specified by the passed index from the passed list */
extern void xPL_clearNamedValueAt(xPL_NameValueListPtr, int);

/* Remove all isntances of the passed name from the passed list */
extern void xPL_clearNamedValue(xPL_NameValueListPtr, char *);

/* Remove All name/value pairs from the passed list */
extern void xPL_clearAllNamedValues(xPL_NameValueListPtr);

/** Raw Listener Support **/
typedef void (* xPL_rawListener)(char *, int, xPL_ObjectPtr);
extern void xPL_addRawListener(xPL_rawListener, xPL_ObjectPtr);
extern Eina_Bool xPL_removeRawListener(xPL_rawListener);

/** Message Listener Support **/
typedef void (* xPL_messageListener)(xPL_MessagePtr, xPL_ObjectPtr);
extern void xPL_addMessageListener(xPL_messageListener, xPL_ObjectPtr);
extern Eina_Bool xPL_removeMessageListener(xPL_messageListener);
#endif /*__XPL_H*/
