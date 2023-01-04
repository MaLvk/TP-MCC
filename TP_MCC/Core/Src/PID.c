/**
  ******************************************************************************
  * @file    PID.c
  * @brief   PID program body
  ******************************************************************************
  */
#include "PID.h"

/**
  * @brief Fonction qui initialise les paramètres du PID.
  * @param pid pointeur vers une structure de PID qui contient les paramètres du PID
  * @retval None
  */
void PIDController_Init(PIDController *pid) {

	/* Clear controller variables */
	pid->integrator = 0.0f;
	pid->prevError  = 0.0f;

	pid->out = 0.0f;

}

/**
  * @brief Fonction qui met à jour les paramètres du PID.
  * @param pid pointeur vers une structure de PID qui contient les paramètres du PID
  * setpoint entrée que l'on souhaite
  * measurement valeur mesurée
  * @retval out Sortie du PID : un flottant entre 0 et 1 qui correspond au rapport cyclique du moteur.
  */
float PIDController_Update(PIDController *pid, float setpoint, float measurement) {

	/*
	* Error signal
	*/
    float error = setpoint - measurement;


	/*
	* Proportional
	*/
    float proportional = pid->Kp * error;


	/*
	* Integral
	*/
    pid->integrator = pid->integrator + 0.5f * pid->Ki * pid->T * (error + pid->prevError);

	/* Anti-wind-up via integrator clamping */
    if (pid->integrator > pid->limMaxInt) {

        pid->integrator = pid->limMaxInt;

    } else if (pid->integrator < pid->limMinInt) {

        pid->integrator = pid->limMinInt;

    }

/*
	* Compute output and apply limits
	*/
    pid->out = proportional + pid->integrator;

    if (pid->out > pid->limMax) {

        pid->out = pid->limMax;

    } else if (pid->out < pid->limMin) {

        pid->out = pid->limMin;

    }

	/* Store error and measurement for later use */
    pid->prevError       = error;

	/* Return controller output */
    return pid->out;

}
