// UTF8 (ü) //
/**
\file   cControlPilot.h

\author Dr. Olaf Simon (KEA)
\author KEA
\date   2024-07-16

\addtogroup G_V2G_ABSTRACTION
@{

\class cControlPilot

\brief Abstract class to interface to control pilot (CP) hardware

Control pilot states can be accessed for reading and writing. The class may
be used by EV or EVSE side as well although not each method or data content
is applicable on both sides. The main content is the control pilot state A-F
and the control pilot pulse width modulation duty cycle.
The class implementation usually require a background process to communicate
states and pulse width to and from the hardware.
**/

#ifndef CCONTROLPILOT_H
#define CCONTROLPILOT_H

#include <string>

namespace Iso15118
{

class cControlPilot;

/**
 * @brief Control pilot state machine states
 */
enum class enCpState
{
    A_Open,               ///< State A
    B_Connected,          ///< State B
    C_Ready,              ///< State C
    D_ReadyVentilation,   ///< State D
    E_ShutDown,           ///< State E
    F_Error,              ///< State F
    Offline               ///< Connection to the CP hardware is broken
};
std::string enCpState_toString(enCpState state); ///< Converts to a string representation

/**
 * @brief Control pilot pwm states
 */
enum class enCpPwm : int
{
    Pwm_6A = 10,                 ///< Maximum current command
    Pwm_9A = 16,                 ///< Maximum current command
    Pwm_15A = 25,                ///< Maximum current command
    Pwm_18A = 30,                ///< Maximum current command
    Pwm_24A = 40,                ///< Maximum current command
    Pwm_30A = 50,                ///< Maximum current command
    Pwm_Iso15118 = 5,            ///< High level communication
    Pwm_Off = 100                ///< No pwm generation
};
enCpPwm enCpPwm_decode(int pwm);                 ///< Generates an enum from an integer
std::string enCpPwm_toString(enCpPwm state);     ///< String representation

/// @brief Cable current capacity according to the proximity pin resistor coding
enum class enPpCode : int
{
    UNKNOWN    = 0,                 ///< Unknown cable current capacity
    PP_13A     = 13,                ///< 13 A
    PP_20A     = 20,                ///< 20 A
    PP_32A     = 32,                ///< 32 A
    PP_63A     = 63                 ///< 63 A
};
std::string enPpCode_toString(enPpCode code);    ///< String representation

/**
 * @brief Full description of the control pilot state
 * The full state of the control pilot consists of the state machine state and the pwm duty cycle.
 */
struct stCpValue
{
    enCpState       state;          ///< State machine state.
    enCpPwm         pwm;            ///< PWM duty cycle / %.

    stCpValue();
    stCpValue(enCpState state, enCpPwm pwm);    ///< Initializer
    stCpValue(enCpState state);                 ///< Initializer
    stCpValue(enCpPwm state);                   ///< Initializer
    stCpValue(int pwm);                         ///< Initializer
    std::string toString();                     ///< String representation
};

/**
 * @brief Interfacing class to be implemented by classes using control pilot callbacks.
 * The callback method may be called synchronously by the operate method or asynchronously by a background thread.
 */
class iControlPilot
{
public:
    virtual ~iControlPilot() = default;                                                                                       ///< Destructor
    virtual void OnControlPilot(cControlPilot* pControlPilot, stCpValue cpState, stCpValue cpLastState) = 0;    ///< Callback method to be implemented
};

/**
 * @brief Interfacing class to control the control pilot (low level communication)
 */
class cControlPilot
{
public:

    virtual stCpValue   get() = 0;                  ///< Retrieves the actual state
    virtual void        set(stCpValue value) = 0;   ///< Sets the desired state
    virtual stCpValue   getDemand() = 0;            ///< Retrieves the last set desired state
    virtual enPpCode    getPpCode() = 0;            ///< Retrieves the cable current capacity according to the PP-code
    virtual void        operate() = 0;              ///< Synchronous state and communication processing
    virtual bool        isInitialized();            ///< The Control Pilot device is ready to operate.

    virtual bool setCallback(iControlPilot* pInstance);               ///< Sets the callback instance
    virtual bool deleteCallback(iControlPilot* pInstance = nullptr);  ///> Removes an instance from the callback list.

    /// @brief Retrieves the actual CP state
    enCpState   getState()
    {
        return get().state;
    }

    /// @brief Sets the desired CP state
    void setState(enCpState state)
    {
        stCpValue cpState = getDemand();
        cpState.state = state;
        set(cpState);
    }

    /// @brief Retrieves the last set CP state
    enCpState getDemandState()
    {
        return getDemand().state;
    }

    /// @brief Retrieves the actual pwm duty cycle
    enCpPwm getPwm()
    {
        return get().pwm;
    }

    /// @brief Retrieves the actual pwm duty cycle as an integer value
    int getPwmValue()
    {
        return (int)get().pwm;
    }

    /// @brief Sets the desired pwm duty cycle
    void setPwm(enCpPwm pwm = enCpPwm::Pwm_Off)
    {
        stCpValue cpState = getDemand();
        cpState.pwm = pwm;
        set(cpState);
    }

    /// @brief Sets the desired pwm duty cycle
    void setPwm(int pwm = 100)
    {
        stCpValue cpState = getDemand();
        cpState.pwm = enCpPwm_decode(pwm);
        set(cpState);
    }

    /// @brief Retrieves the last set pwm duty cycle
    enCpPwm getDemandPwm()
    {
        return getDemand().pwm;
    }

    /// @brief Retrieves the last set pwm duty cycle
    int getDesiredPwmValue()
    {
        return (int)getDemand().pwm;
    }

private:
    stCpValue   value;
};

}

#endif

/** @} */
