/**/


#ifndef	__MOTOR_H
#define		__MOTOR_H

typedef enum{
	MotorSelect_Vertical = 0,
	MotorSelect_Horizontal,
} MotorSelect_t;


int motorSetStop(MotorSelect_t motor);
int motorSetPhase_A_1000(MotorSelect_t motor);
int motorSetPhase_B_1200(MotorSelect_t motor);
int motorSetPhase_C_0200(MotorSelect_t motor);
int motorSetPhase_D_0230(MotorSelect_t motor);
int motorSetPhase_E_0030(MotorSelect_t motor);
int motorSetPhase_F_0034(MotorSelect_t motor);
int motorSetPhase_G_0004(MotorSelect_t motor);
int motorSetPhase_H_1004(MotorSelect_t motor);


#endif		/* end of file */

