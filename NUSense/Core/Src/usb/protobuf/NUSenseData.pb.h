/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.9-dev */

#ifndef PB_MESSAGE_NUSENSEDATA_PB_H_INCLUDED
#define PB_MESSAGE_NUSENSEDATA_PB_H_INCLUDED
#include "pb.h"

#if PB_PROTO_HEADER_VERSION != 40
    #error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
/* Message mainly for NUSense <-> NUC communication. NUSense will encode this message via nanopb after it queries all
 servo states and send it to the NUC. */
typedef struct _message_Servo {
    /* / The ID of the servo */
    uint32_t id;
    /* / Current error state of the servo
/ different to the dynamixel packet error status */
    uint32_t hardware_error;
    /* / Whether the servo's torque is enabled, allowing it to move */
    bool torque_enabled;
    /* Values that are either simulated or read from the servos
/ The last read pulse width modulation of the servo */
    float present_pwm;
    /* / The last read current of the servo */
    float present_current;
    /* / The last read velocity of the servo */
    float present_velocity;
    /* / The last read position of the servo */
    float present_position;
    /* The desired pwm value for motor control */
    float goal_pwm;
    /* The desired current value for the servo */
    float goal_current;
    /* The desired velocity value for the servo */
    float goal_velocity;
    /* The desired position value for the servo */
    float goal_position;
    /* / The last read voltage of the servo */
    float voltage;
    /* / The last read temperature of the servo */
    float temperature;
} message_Servo;

typedef struct _message_IMU_fvec3 {
    float x;
    float y;
    float z;
} message_IMU_fvec3;

typedef struct _message_IMU {
    bool has_accel;
    message_IMU_fvec3 accel;
    bool has_gyro;
    message_IMU_fvec3 gyro;
    uint32_t temperature;
} message_IMU;

typedef struct _message_nusense_ServoMapEntry {
    uint32_t key;
    bool has_value;
    message_Servo value;
} message_nusense_ServoMapEntry;

typedef struct _message_nusense {
    /* INDEX MAPPING
  0  : r_shoulder_pitch
  1  : l_shoulder_pitch
  2  : r_shoulder_roll
  3  : l_shoulder_roll
  4  : r_elbow
  5  : l_elbow
  6  : r_hip_yaw
  7  : l_hip_yaw
  8  : r_hip_roll
  9  : l_hip_roll
  10 : r_hip_pitch
  11 : l_hip_pitch
  12 : r_knee
  13 : l_knee
  14 : r_ankle_pitch
  15 : l_ankle_pitch
  16 : r_ankle_roll
  17 : l_ankle_roll
  18 : head_yaw
  19 : head_pitch */
    pb_size_t servo_map_count;
    message_nusense_ServoMapEntry servo_map[20];
    bool has_imu;
    message_IMU imu;
} message_nusense;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define message_Servo_init_default \
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define message_IMU_init_default \
    { false, message_IMU_fvec3_init_default, false, message_IMU_fvec3_init_default, 0 }
#define message_IMU_fvec3_init_default \
    { 0, 0, 0 }
#define message_nusense_init_default                                                                 \
    {                                                                                                \
        0, {message_nusense_ServoMapEntry_init_default, message_nusense_ServoMapEntry_init_default,  \
            message_nusense_ServoMapEntry_init_default, message_nusense_ServoMapEntry_init_default,  \
            message_nusense_ServoMapEntry_init_default, message_nusense_ServoMapEntry_init_default,  \
            message_nusense_ServoMapEntry_init_default, message_nusense_ServoMapEntry_init_default,  \
            message_nusense_ServoMapEntry_init_default, message_nusense_ServoMapEntry_init_default,  \
            message_nusense_ServoMapEntry_init_default, message_nusense_ServoMapEntry_init_default,  \
            message_nusense_ServoMapEntry_init_default, message_nusense_ServoMapEntry_init_default,  \
            message_nusense_ServoMapEntry_init_default, message_nusense_ServoMapEntry_init_default,  \
            message_nusense_ServoMapEntry_init_default, message_nusense_ServoMapEntry_init_default,  \
            message_nusense_ServoMapEntry_init_default, message_nusense_ServoMapEntry_init_default}, \
            false, message_IMU_init_default                                                          \
    }
#define message_nusense_ServoMapEntry_init_default \
    { 0, false, message_Servo_init_default }
#define message_Servo_init_zero \
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define message_IMU_init_zero \
    { false, message_IMU_fvec3_init_zero, false, message_IMU_fvec3_init_zero, 0 }
#define message_IMU_fvec3_init_zero \
    { 0, 0, 0 }
#define message_nusense_init_zero                                                              \
    {                                                                                          \
        0, {message_nusense_ServoMapEntry_init_zero, message_nusense_ServoMapEntry_init_zero,  \
            message_nusense_ServoMapEntry_init_zero, message_nusense_ServoMapEntry_init_zero,  \
            message_nusense_ServoMapEntry_init_zero, message_nusense_ServoMapEntry_init_zero,  \
            message_nusense_ServoMapEntry_init_zero, message_nusense_ServoMapEntry_init_zero,  \
            message_nusense_ServoMapEntry_init_zero, message_nusense_ServoMapEntry_init_zero,  \
            message_nusense_ServoMapEntry_init_zero, message_nusense_ServoMapEntry_init_zero,  \
            message_nusense_ServoMapEntry_init_zero, message_nusense_ServoMapEntry_init_zero,  \
            message_nusense_ServoMapEntry_init_zero, message_nusense_ServoMapEntry_init_zero,  \
            message_nusense_ServoMapEntry_init_zero, message_nusense_ServoMapEntry_init_zero,  \
            message_nusense_ServoMapEntry_init_zero, message_nusense_ServoMapEntry_init_zero}, \
            false, message_IMU_init_zero                                                       \
    }
#define message_nusense_ServoMapEntry_init_zero \
    { 0, false, message_Servo_init_zero }

/* Field tags (for use in manual encoding/decoding) */
#define message_Servo_id_tag                    1
#define message_Servo_hardware_error_tag        2
#define message_Servo_torque_enabled_tag        3
#define message_Servo_present_pwm_tag           4
#define message_Servo_present_current_tag       5
#define message_Servo_present_velocity_tag      6
#define message_Servo_present_position_tag      7
#define message_Servo_goal_pwm_tag              8
#define message_Servo_goal_current_tag          9
#define message_Servo_goal_velocity_tag         10
#define message_Servo_goal_position_tag         11
#define message_Servo_voltage_tag               12
#define message_Servo_temperature_tag           13
#define message_IMU_fvec3_x_tag                 1
#define message_IMU_fvec3_y_tag                 2
#define message_IMU_fvec3_z_tag                 3
#define message_IMU_accel_tag                   1
#define message_IMU_gyro_tag                    2
#define message_IMU_temperature_tag             3
#define message_nusense_ServoMapEntry_key_tag   1
#define message_nusense_ServoMapEntry_value_tag 2
#define message_nusense_servo_map_tag           1
#define message_nusense_imu_tag                 2

/* Struct field encoding specification for nanopb */
#define message_Servo_FIELDLIST(X, a)                  \
    X(a, STATIC, SINGULAR, UINT32, id, 1)              \
    X(a, STATIC, SINGULAR, UINT32, hardware_error, 2)  \
    X(a, STATIC, SINGULAR, BOOL, torque_enabled, 3)    \
    X(a, STATIC, SINGULAR, FLOAT, present_pwm, 4)      \
    X(a, STATIC, SINGULAR, FLOAT, present_current, 5)  \
    X(a, STATIC, SINGULAR, FLOAT, present_velocity, 6) \
    X(a, STATIC, SINGULAR, FLOAT, present_position, 7) \
    X(a, STATIC, SINGULAR, FLOAT, goal_pwm, 8)         \
    X(a, STATIC, SINGULAR, FLOAT, goal_current, 9)     \
    X(a, STATIC, SINGULAR, FLOAT, goal_velocity, 10)   \
    X(a, STATIC, SINGULAR, FLOAT, goal_position, 11)   \
    X(a, STATIC, SINGULAR, FLOAT, voltage, 12)         \
    X(a, STATIC, SINGULAR, FLOAT, temperature, 13)
#define message_Servo_CALLBACK NULL
#define message_Servo_DEFAULT  NULL

#define message_IMU_FIELDLIST(X, a)           \
    X(a, STATIC, OPTIONAL, MESSAGE, accel, 1) \
    X(a, STATIC, OPTIONAL, MESSAGE, gyro, 2)  \
    X(a, STATIC, SINGULAR, UINT32, temperature, 3)
#define message_IMU_CALLBACK      NULL
#define message_IMU_DEFAULT       NULL
#define message_IMU_accel_MSGTYPE message_IMU_fvec3
#define message_IMU_gyro_MSGTYPE  message_IMU_fvec3

#define message_IMU_fvec3_FIELDLIST(X, a) \
    X(a, STATIC, SINGULAR, FLOAT, x, 1)   \
    X(a, STATIC, SINGULAR, FLOAT, y, 2)   \
    X(a, STATIC, SINGULAR, FLOAT, z, 3)
#define message_IMU_fvec3_CALLBACK NULL
#define message_IMU_fvec3_DEFAULT  NULL

#define message_nusense_FIELDLIST(X, a)           \
    X(a, STATIC, REPEATED, MESSAGE, servo_map, 1) \
    X(a, STATIC, OPTIONAL, MESSAGE, imu, 2)
#define message_nusense_CALLBACK          NULL
#define message_nusense_DEFAULT           NULL
#define message_nusense_servo_map_MSGTYPE message_nusense_ServoMapEntry
#define message_nusense_imu_MSGTYPE       message_IMU

#define message_nusense_ServoMapEntry_FIELDLIST(X, a) \
    X(a, STATIC, SINGULAR, UINT32, key, 1)            \
    X(a, STATIC, OPTIONAL, MESSAGE, value, 2)
#define message_nusense_ServoMapEntry_CALLBACK      NULL
#define message_nusense_ServoMapEntry_DEFAULT       NULL
#define message_nusense_ServoMapEntry_value_MSGTYPE message_Servo

extern const pb_msgdesc_t message_Servo_msg;
extern const pb_msgdesc_t message_IMU_msg;
extern const pb_msgdesc_t message_IMU_fvec3_msg;
extern const pb_msgdesc_t message_nusense_msg;
extern const pb_msgdesc_t message_nusense_ServoMapEntry_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define message_Servo_fields                 &message_Servo_msg
#define message_IMU_fields                   &message_IMU_msg
#define message_IMU_fvec3_fields             &message_IMU_fvec3_msg
#define message_nusense_fields               &message_nusense_msg
#define message_nusense_ServoMapEntry_fields &message_nusense_ServoMapEntry_msg

/* Maximum encoded size of messages (where known) */
#define MESSAGE_NUSENSEDATA_PB_H_MAX_SIZE  message_nusense_size
#define message_IMU_fvec3_size             15
#define message_IMU_size                   40
#define message_nusense_ServoMapEntry_size 72
#define message_nusense_size               1522
#define message_Servo_size                 64

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
