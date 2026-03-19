#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define STDCONFIGNAME "standard"
// This is the standard configuration being set if no configuration file (config.xml) is present
#define STACKTYPE Iso15118::enStackType::secc
//#define STACKTYPE "evcc"

//#define INTERFACE_NAME "WLAN"
#define INTERFACE_NAME "Ethernet"
//#define INTERFACE_NAME "Ethernet 3"
//#define INTERFACE_NAME "Ethernet 6"

//#define SCHEMAID 10  // Codec Iso an dSpace
#define SCHEMAID 3  // Codec Iso an Codico-stack
//#define SCHEMAID 2  // Codec Din an Codico-stack / ISO an IONIQ5
//#define SCHEMAID 1  // Codec Din an IONIQ5, ID.3
//#define SCHEMAID 0  // Codec Iso an ID.3

#define TRANSFERMODE iso1EnergyTransferModeType_AC_three_phase_core
#define PROVIDETLS false


//#define EASOURCE_IP_ADDRESS "192.168.178.74"
//#define EASOURCE_IP_ADDRESS "192.168.0.3"
//#define EASOURCE_IP_ADDRESS "192.168.0.5"
#define EASOURCE_IP_ADDRESS "192.168.0.6"

#define VOLTAGE_FAKTOR 1
#define CURRENT_FAKTOR 1

#include <string>

#include "../types.h"
#include "../cExiDin70121openV2g.h"

/** \brief Definitions of hardware types
 *  These types are used to define the power electronics hardware the stack operates with.
 */
enum class enHardwareType
{
    NONE,               ///< No hardware is available
    EA_SOURCE,          ///< A power source of EA is controlled
    ADVANTECH           ///< An Advantech I/O interface module is controlled
};

/** \brief Struct holding the content of the configuration file
 *  The struct is preinitialized with standard values.
 */
typedef struct stConfiguration
{
    std::string                 instance        = std::string(STDCONFIGNAME) + " " + (STACKTYPE == Iso15118::enStackType::secc ? std::string("secc") : std::string("evcc"));    ///< The name of the configuration instance actually used from the configuration file
    Iso15118::enStackType       stackType       = STACKTYPE;            ///< Supply or vehicle operation.
    std::string                 interfaceName   = INTERFACE_NAME;       ///< Ethernet interface to be used
    int                         schemaId        = SCHEMAID;             ///< obsolete
    std::string                 eaSourceIp      = EASOURCE_IP_ADDRESS;  ///< IP address used for control communication to a EA-source, if this source is configured.
    bool                        provideTls      = PROVIDETLS;           ///< Using TLS for V2G communication
    iso1EnergyTransferModeType  transferMode    = TRANSFERMODE;         ///< Transfer mode (e.g. AC or DC) the hardware the stack is connected to is supports.
    enHardwareType              hardware        = enHardwareType::NONE; ///< The type of power electronics hardware that is used.
} stConfiguration;

void writeXML(stConfiguration& config);
bool readXML(stConfiguration& config);

#endif
