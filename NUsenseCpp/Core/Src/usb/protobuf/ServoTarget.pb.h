/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.8-dev */

#ifndef PB_MESSAGE_ACTUATION_SERVOTARGET_PB_H_INCLUDED
#define PB_MESSAGE_ACTUATION_SERVOTARGET_PB_H_INCLUDED
#include "pb.h"
#include "timestamp.pb.h"

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
/* *
 @author Trent Houliston */
typedef struct _message_actuation_ServoTarget {
    /* / When the target was created */
    bool has_time;
    google_protobuf_Timestamp time;
    /* / The ID of the servo being controlled */
    uint32_t id;
    /* / The position value of the destination. 0 to 4,095 (0xFFF) is available.
/ The unit is 0.088 [°]. If Goal Position is out of the range, Angle Limit
/ Error Bit (Bit1) of Status Packet is returned as ‘1’ and Alarm is
/ triggered as set in Alarm LED/Shutdown */
    float position;
    /* / Proportional gain. In proportion to the servo's position error. Gain
/ values are in range 0~254. K_p : p_gain / 8 */
    float gain;
    /* / Used to set the servo on or off. Typically either 0 (off) or 100 (on) */
    float torque;
} message_actuation_ServoTarget;

typedef struct _message_actuation_ServoTargets {
    pb_size_t targets_count;
    message_actuation_ServoTarget targets[20];
} message_actuation_ServoTargets;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define message_actuation_ServoTarget_init_default {false, google_protobuf_Timestamp_init_default, 0, 0, 0, 0}
#define message_actuation_ServoTargets_init_default {0, {message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default, message_actuation_ServoTarget_init_default}}
#define message_actuation_ServoTarget_init_zero  {false, google_protobuf_Timestamp_init_zero, 0, 0, 0, 0}
#define message_actuation_ServoTargets_init_zero {0, {message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero, message_actuation_ServoTarget_init_zero}}

/* Field tags (for use in manual encoding/decoding) */
#define message_actuation_ServoTarget_time_tag   1
#define message_actuation_ServoTarget_id_tag     2
#define message_actuation_ServoTarget_position_tag 3
#define message_actuation_ServoTarget_gain_tag   4
#define message_actuation_ServoTarget_torque_tag 5
#define message_actuation_ServoTargets_targets_tag 1

/* Struct field encoding specification for nanopb */
#define message_actuation_ServoTarget_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, MESSAGE,  time,              1) \
X(a, STATIC,   SINGULAR, UINT32,   id,                2) \
X(a, STATIC,   SINGULAR, FLOAT,    position,          3) \
X(a, STATIC,   SINGULAR, FLOAT,    gain,              4) \
X(a, STATIC,   SINGULAR, FLOAT,    torque,            5)
#define message_actuation_ServoTarget_CALLBACK NULL
#define message_actuation_ServoTarget_DEFAULT NULL
#define message_actuation_ServoTarget_time_MSGTYPE google_protobuf_Timestamp

#define message_actuation_ServoTargets_FIELDLIST(X, a) \
X(a, STATIC,   REPEATED, MESSAGE,  targets,           1)
#define message_actuation_ServoTargets_CALLBACK NULL
#define message_actuation_ServoTargets_DEFAULT NULL
#define message_actuation_ServoTargets_targets_MSGTYPE message_actuation_ServoTarget

extern const pb_msgdesc_t message_actuation_ServoTarget_msg;
extern const pb_msgdesc_t message_actuation_ServoTargets_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define message_actuation_ServoTarget_fields &message_actuation_ServoTarget_msg
#define message_actuation_ServoTargets_fields &message_actuation_ServoTargets_msg

/* Maximum encoded size of messages (where known) */
#define message_actuation_ServoTarget_size       45
#define message_actuation_ServoTargets_size      940

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
