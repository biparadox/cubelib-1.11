#ifndef NAMEVALUE
#define NAMEVALUE

#include "flow_policy.h"
#include "connector.h"
#include "message_struct.h"
#include "sec_entity.h"
#include "router.h"

static NAME2VALUE message_flow_valuelist[] = 
{
    {"INIT",MSG_FLOW_INIT},
    {"LOCAL",MSG_FLOW_LOCAL},
    {"DELIVER",MSG_FLOW_DELIVER},
    {"QUERY",MSG_FLOW_QUERY},
    {"RESPONSE",MSG_FLOW_RESPONSE},
    {"ASPECT",MSG_FLOW_ASPECT},
    {"ASPECT_LOCAL",MSG_FLOW_ASPECT_LOCAL},
    {"ASPECT_RETURN",MSG_FLOW_ASPECT_RETURN},
    {"WAIT",MSG_FLOW_WAIT},
    {"TRANS",MSG_FLOW_TRANS},
    {"DRECV",MSG_FLOW_DRECV},
    {"QRECV",MSG_FLOW_QRECV},
    {"FINISH",MSG_FLOW_FINISH},
//  {"ERROR",MSG_FLOW_ERROR},
    {NULL,0}
};

static NAME2VALUE match_entity_valuelist[] = 
{
	{"MESSAGE",DISPATCH_MATCH_MSG},
	{"MESSAGE_RECORD",DISPATCH_MATCH_MSG_RECORD},
	{"MESSAGE_EXPAND",DISPATCH_MATCH_MSG_EXPAND},
	{"PROC",DISPATCH_MATCH_PROC},
	{NULL,0}
};

static NAME2VALUE connector_type_valuelist[] = 
{
	{"INVALID",CONN_INVALID},
	{"P2P",CONN_P2P},
	{"CLIENT",CONN_CLIENT},
	{"CHANNEL",CONN_CHANNEL},
	{"SERVER",CONN_SERVER},
	{NULL,0}
};
static NAME2VALUE connector_state_valuelist[] = 
{
	// server's state
	{"CONN_SERVER_INIT",CONN_SERVER_INIT},
	{"CONN_SERVER_READY",CONN_SERVER_READY},
	{"CONN_SERVER_LISTEN",CONN_SERVER_LISTEN},
	{"CONN_SERVER_SERVICE",CONN_SERVER_SERVICE},
	{"CONN_SERVER_SLEEP",CONN_SERVER_SLEEP},
	{"CONN_SERVER_SHUTDOWN",CONN_SERVER_SHUTDOWN},
	// client's state
	{"CONN_CLIENT_INIT",CONN_CLIENT_INIT},
	{"CONN_CLIENT_CONNECT",CONN_CLIENT_CONNECT},
	{"CONN_CLIENT_RESPONSE",CONN_CLIENT_RESPONSE},
	{"CONN_CLIENT_DISCONNECT",CONN_CLIENT_DISCONNECT},
	// channel's state
	{"CONN_CHANNEL_INIT",CONN_CHANNEL_INIT},
	{"CONN_CHANNEL_ACCEPT",CONN_CHANNEL_ACCEPT},
	{"CONN_CHANNEL_HANDSHAKE",CONN_CHANNEL_HANDSHAKE},
	{"CONN_CHANNEL_CLOSE",CONN_CHANNEL_CLOSE},
	{NULL,0}
};

static NAME2VALUE message_flag_valuelist[] = 
{
	{"CRYPT",MSG_FLAG_CRYPT},
	{"SIGN",MSG_FLAG_SIGN},
	{"ZIP",MSG_FLAG_SIGN},
	{"VERIFY",MSG_FLAG_VERIFY},
	{NULL,0}
};

static NAME2VALUE sec_subject_type_valuelist[] =
{
	{"PROC_TYPE_MAIN",PROC_TYPE_MAIN},
	{"PROC_TYPE_CONN",PROC_TYPE_CONN},
	{"PROC_TYPE_MONITOR",PROC_TYPE_MONITOR},
	{"PROC_TYPE_DECIDE",PROC_TYPE_DECIDE},
	{"PROC_TYPE_CONTROL",PROC_TYPE_CONTROL},
	{NULL,0}
};
static NAME2VALUE sec_proc_state_valuelist[] =
{
	{"SEC_PROC_CREATE",SEC_PROC_CREATE},
	{"SEC_PROC_INIT",SEC_PROC_INIT},
	{"SEC_PROC_START",SEC_PROC_START},
	{"SEC_PROC_RUNNING",SEC_PROC_RUNNING},
	{"SEC_PROC_WAITING",SEC_PROC_WAITING},
	{"SEC_PROC_STOP",SEC_PROC_STOP},
	{"SEC_PROC_ZOMBIE",SEC_PROC_ZOMBIE},
	{NULL,0}
};

static NAME2VALUE match_rule_op_valuelist[] =
{
    {"AND",MATCH_OP_AND},
    {"OR",MATCH_OP_OR},
    {"NOT",MATCH_OP_NOT},
    {"ERR",MATCH_OP_ERR},
    {NULL,0}
};

static NAME2VALUE message_area_valuelist[] =
{
    {"HEAD",MATCH_AREA_HEAD},
    {"RECORD",MATCH_AREA_RECORD},
    {"EXPAND",MATCH_AREA_EXPAND},
    {"ERR",MATCH_AREA_ERR},
    {NULL,0}
};
/*
static NAME2VALUE message_flow_type_valuelist[] =
{
    {"INIT",MSG_FLOW_INIT},
    {"LOCAL",MSG_FLOW_LOCAL},
    {"DELIVER",MSG_FLOW_DELIVER},
    {"QUERY",MSG_FLOW_QUERY},
    {"RESPONSE",MSG_FLOW_RESPONSE},
    {"ASPECT",MSG_FLOW_ASPECT},
    {"ASPECT_LOCAL",MSG_FLOW_ASPECT_LOCAL},
    {"ASPECT_RETURN",MSG_FLOW_ASPECT_RETURN},
    {"WAIT",MSG_FLOW_WAIT},
    {"FINISH",MSG_FLOW_FINISH},
    {"ERR",MSG_FLOW_ERROR},
    {NULL,0}
};*/

static NAME2VALUE message_target_type_valuelist[] =
{
    {"LOCAL",MSG_TARGET_LOCAL},
    {"NAME",MSG_TARGET_NAME},
    {"UUID",MSG_TARGET_UUID},
    {"RECORD",MSG_TARGET_RECORD},
    {"EXPAND",MSG_TARGET_EXPAND},
    {"SPLIT",MSG_TARGET_SPLIT},
    {"CONN",MSG_TARGET_CONN},
    {"MIXUUID",MSG_TARGET_MIXUUID},
    {"ERR",MSG_TARGET_ERROR},
    {NULL,0}
};
#endif
