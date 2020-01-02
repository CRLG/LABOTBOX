/*
 * File: plateformer_robot.h
 *
 * Code generated for Simulink model 'plateformer_robot'.
 *
 * Model version                  : 1.7
 * Simulink Coder version         : 8.9 (R2015b) 13-Aug-2015
 * C/C++ source code generated on : Thu Nov 14 10:12:56 2019
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */
#ifndef RTW_HEADER_plateformer_robot_h_
#define RTW_HEADER_plateformer_robot_h_
#include <math.h>
#ifndef plateformer_robot_COMMON_INCLUDES_
# define plateformer_robot_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* plateformer_robot_COMMON_INCLUDES_ */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
# define rtmGetErrorStatus(rtm)        ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
# define rtmSetErrorStatus(rtm, val)   ((rtm)->errorStatus = (val))
#endif

/* Forward declaration for rtModel */
typedef struct tag_RTM RT_MODEL;

/* Block signals and states (auto storage) for system '<Root>' */
typedef struct {
  real_T UnitDelay2_DSTATE;            /* '<S1>/Unit Delay2' */
  real_T UnitDelay5_DSTATE;            /* '<S1>/Unit Delay5' */
  real_T UD_DSTATE;                    /* '<S2>/UD' */
  real_T UD_DSTATE_a;                  /* '<S4>/UD' */
  real_T UD_DSTATE_g;                  /* '<S3>/UD' */
  real_T UnitDelay_DSTATE;             /* '<S1>/Unit Delay' */
  real_T UD_DSTATE_n;                  /* '<S5>/UD' */
  real_T UnitDelay1_DSTATE;            /* '<S1>/Unit Delay1' */
  real_T UnitDelay3_DSTATE;            /* '<S1>/Unit Delay3' */
  real_T UnitDelay4_DSTATE;            /* '<S1>/Unit Delay4' */
} DW;

/* External inputs (root inport signals with auto storage) */
typedef struct {
  real_T ubatt;                        /* '<Root>/ubatt' */
  real_T cde_mot_G;                    /* '<Root>/cde_mot_G' */
  real_T cde_mot_D;                    /* '<Root>/cde_mot_D' */
} ExtU;

/* External outputs (root outports fed by signals with auto storage) */
typedef struct {
  real_T RegistrecodeurG;              /* '<Root>/Registre codeur G' */
  real_T RegistrecodeurD;              /* '<Root>/Registre codeur D' */
} ExtY;

/* Real-time Model Data Structure */
struct tag_RTM {
  const char_T * volatile errorStatus;
};

/* Block signals and states (auto storage) */
extern DW rtDW;

/* External inputs (root inport signals with auto storage) */
extern ExtU rtU;

/* External outputs (root outports fed by signals with auto storage) */
extern ExtY rtY;

/* Model entry point functions */
extern void plateformer_robot_initialize(void);
extern void plateformer_robot_step(void);

/* Real-time Model object */
extern RT_MODEL *const rtM;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S2>/Data Type Duplicate' : Unused code path elimination
 * Block '<S3>/Data Type Duplicate' : Unused code path elimination
 * Block '<S4>/Data Type Duplicate' : Unused code path elimination
 * Block '<S5>/Data Type Duplicate' : Unused code path elimination
 */

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Note that this particular code originates from a subsystem build,
 * and has its own system numbers different from the parent model.
 * Refer to the system hierarchy for this subsystem below, and use the
 * MATLAB hilite_system command to trace the generated code back
 * to the parent model.  For example,
 *
 * hilite_system('Mdl_plateforme_2015b/plateformer_robot')    - opens subsystem Mdl_plateforme_2015b/plateformer_robot
 * hilite_system('Mdl_plateforme_2015b/plateformer_robot/Kp') - opens and selects block Kp
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'Mdl_plateforme_2015b'
 * '<S1>'   : 'Mdl_plateforme_2015b/plateformer_robot'
 * '<S2>'   : 'Mdl_plateforme_2015b/plateformer_robot/Transfer Fcn First Order'
 * '<S3>'   : 'Mdl_plateforme_2015b/plateformer_robot/Transfer Fcn First Order1'
 * '<S4>'   : 'Mdl_plateforme_2015b/plateformer_robot/Transfer Fcn First Order2'
 * '<S5>'   : 'Mdl_plateforme_2015b/plateformer_robot/Transfer Fcn First Order3'
 */


#endif                                 /* RTW_HEADER_plateformer_robot_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
