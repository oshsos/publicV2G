// UTF8 (ü)
/**
\file   cControlPilot.cpp

\author Dr. Olaf Simon (KEA)
\author KEA
\date   2024-07-16

\addtogroup G_V2G_ABSTRACTION
@{

\class cControlPilot
*/

#include "cControlPilot.h"

using namespace std;
using namespace Iso15118;

/**
 * @brief Converts an enCpState to a string.
 * @param state State to convert.
 * @return String representation.
 */
string Iso15118::enCpState_toString(enCpState state)
{
    switch (state)
    {
    case enCpState::A_Open: return "A-unplugged";
    case enCpState::B_Connected: return "B-connected";
    case enCpState::C_Ready: return "C-ready";
    case enCpState::D_ReadyVentilation: return "D-ready vent.";
    case enCpState::E_ShutDown: return "E-shut down";
    case enCpState::F_Error: return "F-error";
    case enCpState::Offline: return "Offline";
    default: return "<undefined>";
    }
}

/**
 * @brief Rounds any pwm duty cycle to the standardized enumeration.
 * @param pwm
 * @return
 */
enCpPwm Iso15118::enCpPwm_decode(int pwm)
{
    if (pwm > 95) return enCpPwm::Pwm_Off;
    else if (pwm > 45) return enCpPwm::Pwm_30A;
    else if (pwm > 35) return enCpPwm::Pwm_24A;
    else if (pwm > 27) return enCpPwm::Pwm_18A;
    else if (pwm > 21) return enCpPwm::Pwm_15A;
    else if (pwm > 13) return enCpPwm::Pwm_9A;
    else if (pwm > 7) return enCpPwm::Pwm_6A;
    else if (pwm > 2) return enCpPwm::Pwm_Iso15118;
    return enCpPwm::Pwm_Off;
}

/**
 * @brief enCpState_toString
 * @param state
 * @return
 */
std::string Iso15118::enCpPwm_toString(enCpPwm state)
{
    switch (state)
    {
    case enCpPwm::Pwm_6A: return "6A";
    case enCpPwm::Pwm_9A: return "9A";
    case enCpPwm::Pwm_15A: return "15A";
    case enCpPwm::Pwm_18A: return "18A";
    case enCpPwm::Pwm_24A: return "24A";
    case enCpPwm::Pwm_30A: return "30A";
    case enCpPwm::Pwm_Iso15118: return "Iso15118";
    case enCpPwm::Pwm_Off: return "None";
    default: return "<undefined>";
    }
}

/**
 * @brief String representation of EPpCode
 */
std::string Iso15118::enPpCode_toString(enPpCode code)
{
    switch (code)
    {
    case enPpCode::PP_13A: return "13A";
    case enPpCode::PP_20A: return "20A";
    case enPpCode::PP_32A: return "32A";
    case enPpCode::PP_63A: return "63A";
    default: return "<unknwon>";
    }
}

/// @brief Initializer
stCpValue::stCpValue() :
    state(enCpState::Offline),
    pwm(enCpPwm::Pwm_Off)
{
}

/// @brief Initializer
stCpValue::stCpValue(enCpState state, enCpPwm pwm) :
    state(state),
    pwm(pwm)
{
}

/// @brief Initializer
stCpValue::stCpValue(enCpState state) :
    state(state),
    pwm(enCpPwm::Pwm_Off)
{
}

/// @brief Initializer
stCpValue::stCpValue(enCpPwm pwm) :
    state(enCpState::Offline),
    pwm(pwm)
{
}

/// @brief Initializer
stCpValue::stCpValue(int pwm) :
    state(enCpState::Offline),
    pwm(enCpPwm_decode(pwm))
{
}

/// @brief Converts the content of stControlPilot to a string.
std::string stCpValue::toString()
{
    return "State = " + enCpState_toString(state) + ", Pwm = " + std::to_string((int)pwm);
}

#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * @brief Sets the callback instance
 * @param pInstance
 * @return
 */
bool cControlPilot::setCallback(iControlPilot* pInstance)
{
    return false;
}

/**
 * @brief Removes an instance from the callback list.
 * @param pInstance
 * @return
 */
bool cControlPilot::deleteCallback(iControlPilot* pInstance)
{
    return true;
}

#pragma GCC diagnostic warning "-Wunused-parameter"

bool cControlPilot::isInitialized()
{
    return true;
}

/** @} */
