/*
 * File: plateformer_robot.c
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

#include "plateformer_robot.h"

/* Block signals and states (auto storage) */
DW rtDW;

/* External inputs (root inport signals with auto storage) */
ExtU rtU;

/* External outputs (root outports fed by signals with auto storage) */
ExtY rtY;

/* Real-time model */
RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;
extern real_T rt_roundd(real_T u);
real_T rt_roundd(real_T u)
{
  real_T y;
  if (fabs(u) < 4.503599627370496E+15) {
    if (u >= 0.5) {
      y = floor(u + 0.5);
    } else if (u > -0.5) {
      y = 0.0;
    } else {
      y = ceil(u - 0.5);
    }
  } else {
    y = u;
  }

  return y;
}

/* Model step function */
void plateformer_robot_step(void)
{
  real_T rtb_UnitDelay4;
  real_T rtb_Gain13;
  real_T rtb_Sum;
  real_T rtb_Sum_e;
  real_T rtb_Sum_a;
  real_T rtb_Sum_j;
  real_T rtb_Add5;
  real_T rtb_Add4;
  real_T rtb_Gain9;
  real_T rtb_Gain10;

  /* Outputs for Atomic SubSystem: '<Root>/plateformer_robot' */
  /* Gain: '<S1>/Gain8' incorporates:
   *  Inport: '<Root>/ubatt'
   */
  rtb_UnitDelay4 = 0.01 * rtU.ubatt;

  /* DeadZone: '<S1>/Dead Zone' incorporates:
   *  Inport: '<Root>/cde_mot_G'
   */
  if (rtU.cde_mot_G > 4.0) {
    rtb_Sum = rtU.cde_mot_G - 4.0;
  } else if (rtU.cde_mot_G >= -4.0) {
    rtb_Sum = 0.0;
  } else {
    rtb_Sum = rtU.cde_mot_G - -4.0;
  }

  /* End of DeadZone: '<S1>/Dead Zone' */

  /* Gain: '<S1>/Gain' incorporates:
   *  Gain: '<S1>/Fct trans vitesse Fém'
   *  Product: '<S1>/Product'
   *  Sum: '<S1>/Add'
   *  UnitDelay: '<S1>/Unit Delay2'
   */
  rtb_Gain13 = (rtb_UnitDelay4 * rtb_Sum - 0.2 * rtDW.UnitDelay2_DSTATE) *
    0.16999999999999998;

  /* DeadZone: '<S1>/Dead Zone1' incorporates:
   *  Inport: '<Root>/cde_mot_D'
   */
  if (rtU.cde_mot_D > 4.0) {
    rtb_Sum = rtU.cde_mot_D - 4.0;
  } else if (rtU.cde_mot_D >= -4.0) {
    rtb_Sum = 0.0;
  } else {
    rtb_Sum = rtU.cde_mot_D - -4.0;
  }

  /* End of DeadZone: '<S1>/Dead Zone1' */

  /* Gain: '<S1>/Gain2' incorporates:
   *  Gain: '<S1>/Gain3'
   *  Product: '<S1>/Product1'
   *  Sum: '<S1>/Add3'
   *  UnitDelay: '<S1>/Unit Delay5'
   */
  rtb_UnitDelay4 = (rtb_UnitDelay4 * rtb_Sum - 0.2 * rtDW.UnitDelay5_DSTATE) *
    0.16999999999999998;

  /* Sum: '<S2>/Sum' incorporates:
   *  Gain: '<S2>/Gain'
   *  Sum: '<S2>/Diff'
   *  UnitDelay: '<S2>/UD'
   *
   * Block description for '<S2>/Sum':
   *
   *  Add in CPU
   *
   * Block description for '<S2>/Diff':
   *
   *  Add in CPU
   *
   * Block description for '<S2>/UD':
   *
   *  Store in Global RAM
   */
  rtb_Sum = (rtDW.UD_DSTATE - rtb_Gain13) * 0.8 + rtb_Gain13;

  /* Sum: '<S4>/Sum' incorporates:
   *  Gain: '<S4>/Gain'
   *  Sum: '<S4>/Diff'
   *  UnitDelay: '<S4>/UD'
   *
   * Block description for '<S4>/Sum':
   *
   *  Add in CPU
   *
   * Block description for '<S4>/Diff':
   *
   *  Add in CPU
   *
   * Block description for '<S4>/UD':
   *
   *  Store in Global RAM
   */
  rtb_Sum_e = (rtDW.UD_DSTATE_a - rtb_UnitDelay4) * 0.8 + rtb_UnitDelay4;

  /* Gain: '<S1>/Gain4' incorporates:
   *  Sum: '<S1>/Add2'
   */
  rtb_UnitDelay4 = (rtb_Sum + rtb_Sum_e) * 19.0;

  /* Sum: '<S3>/Sum' incorporates:
   *  Gain: '<S3>/Gain'
   *  Sum: '<S3>/Diff'
   *  UnitDelay: '<S3>/UD'
   *
   * Block description for '<S3>/Sum':
   *
   *  Add in CPU
   *
   * Block description for '<S3>/Diff':
   *
   *  Add in CPU
   *
   * Block description for '<S3>/UD':
   *
   *  Store in Global RAM
   */
  rtb_Sum_a = (rtDW.UD_DSTATE_g - rtb_UnitDelay4) * 0.97 + rtb_UnitDelay4;

  /* Sum: '<S1>/Add4' incorporates:
   *  Gain: '<S1>/Gain6'
   *  UnitDelay: '<S1>/Unit Delay'
   */
  rtb_Add4 = 0.02 * rtb_Sum_a + rtDW.UnitDelay_DSTATE;

  /* Gain: '<S1>/Gain5' incorporates:
   *  Sum: '<S1>/Add1'
   */
  rtb_UnitDelay4 = (rtb_Sum - rtb_Sum_e) * 2.0;

  /* Sum: '<S5>/Sum' incorporates:
   *  Gain: '<S5>/Gain'
   *  Sum: '<S5>/Diff'
   *  UnitDelay: '<S5>/UD'
   *
   * Block description for '<S5>/Sum':
   *
   *  Add in CPU
   *
   * Block description for '<S5>/Diff':
   *
   *  Add in CPU
   *
   * Block description for '<S5>/UD':
   *
   *  Store in Global RAM
   */
  rtb_Sum_j = (rtDW.UD_DSTATE_n - rtb_UnitDelay4) * 0.98 + rtb_UnitDelay4;

  /* Sum: '<S1>/Add5' incorporates:
   *  Gain: '<S1>/Gain7'
   *  UnitDelay: '<S1>/Unit Delay1'
   */
  rtb_Add5 = 0.02 * rtb_Sum_j + rtDW.UnitDelay1_DSTATE;

  /* Gain: '<S1>/Transf Angle distance' */
  rtb_UnitDelay4 = 10.0 * rtb_Add5;

  /* Sum: '<S1>/Add6' */
  rtb_Gain13 = rtb_Add4 + rtb_UnitDelay4;

  /* Sum: '<S1>/Add7' */
  rtb_UnitDelay4 = rtb_Add4 - rtb_UnitDelay4;

  /* Gain: '<S1>/Gain9' */
  rtb_Gain9 = 50.0 * rtb_Gain13;

  /* Gain: '<S1>/Gain10' */
  rtb_Gain10 = 50.0 * rtb_UnitDelay4;

  /* Gain: '<S1>/Gain14' */
  rtb_UnitDelay4 *= 30.525030525030527;

  /* Gain: '<S1>/Gain13' */
  rtb_Gain13 *= 30.525030525030527;

  /* Update for UnitDelay: '<S1>/Unit Delay2' incorporates:
   *  Sum: '<S1>/Add8'
   *  UnitDelay: '<S1>/Unit Delay3'
   */
  rtDW.UnitDelay2_DSTATE = rtb_Gain9 - rtDW.UnitDelay3_DSTATE;

  /* Update for UnitDelay: '<S1>/Unit Delay5' incorporates:
   *  Sum: '<S1>/Add9'
   *  UnitDelay: '<S1>/Unit Delay4'
   */
  rtDW.UnitDelay5_DSTATE = rtb_Gain10 - rtDW.UnitDelay4_DSTATE;

  /* Update for UnitDelay: '<S2>/UD'
   *
   * Block description for '<S2>/UD':
   *
   *  Store in Global RAM
   */
  rtDW.UD_DSTATE = rtb_Sum;

  /* Update for UnitDelay: '<S4>/UD'
   *
   * Block description for '<S4>/UD':
   *
   *  Store in Global RAM
   */
  rtDW.UD_DSTATE_a = rtb_Sum_e;

  /* Update for UnitDelay: '<S3>/UD'
   *
   * Block description for '<S3>/UD':
   *
   *  Store in Global RAM
   */
  rtDW.UD_DSTATE_g = rtb_Sum_a;

  /* Update for UnitDelay: '<S1>/Unit Delay' */
  rtDW.UnitDelay_DSTATE = rtb_Add4;

  /* Update for UnitDelay: '<S5>/UD'
   *
   * Block description for '<S5>/UD':
   *
   *  Store in Global RAM
   */
  rtDW.UD_DSTATE_n = rtb_Sum_j;

  /* Update for UnitDelay: '<S1>/Unit Delay1' */
  rtDW.UnitDelay1_DSTATE = rtb_Add5;

  /* Update for UnitDelay: '<S1>/Unit Delay3' */
  rtDW.UnitDelay3_DSTATE = rtb_Gain9;

  /* Update for UnitDelay: '<S1>/Unit Delay4' */
  rtDW.UnitDelay4_DSTATE = rtb_Gain10;

  /* Outport: '<Root>/Registre codeur G' incorporates:
   *  Quantizer: '<S1>/Codeur G'
   */
  rtY.RegistrecodeurG = rt_roundd(rtb_Gain13);

  /* Outport: '<Root>/Registre codeur D' incorporates:
   *  Quantizer: '<S1>/Codeur D'
   */
  rtY.RegistrecodeurD = rt_roundd(rtb_UnitDelay4);

  /* End of Outputs for SubSystem: '<Root>/plateformer_robot' */
}

/* Model initialize function */
void plateformer_robot_initialize(void)
{
  /* (no initialization code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
