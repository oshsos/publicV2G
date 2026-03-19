/** \mainpage KEA publicV2G ISO 15118 stack

This repository 'publicV2G' is provided by KEA, the 'Kompetenzzentrum für Elektronik und Antriebstechnik - Science to business GmbH -
Hochschule Osnabrück - https://kea-nds.de/'

\section SEC_INTRO Introduction

Charging of electric vehicles by using the 'Combined Charging System' standard (CCS) becomes more and more important for many kinds
of charging columns and wallboxes. CCS applies high bandwidth communication between the electrical supply and the
vehicle by power line communication (PLC) modulated on the control pilot signal. That way additional services like identification
and billing but also smart grid functionallity is provided. The appropriate
communication protocoll, also called 'vehicle to grid' (V2G) communication, is standardized within the ISO 15118-2 and -20.\n
Many use cases for type 2 AC charging also require or at least might be enhanced by using V2G communication. In many, if not all,
use cases a RFID identification device will become unnecessary and thus the charging process is simplified for the end user.

Due to the rising need of energy management solutions the use of V2G functionallity becomes important even for AC-charging wallboxes.
Companies being interested in providing V2G communication within their wallboxes might hesitate to implement this standard, as
the development takes much efford compared to a relatively small price of the wallbox itself. This is especially the case if the
wallbox just supplements the product portfolio and other business like energy generation or storage is intended

Although some public source stacks are available, they are implemented in high level programming languages and are not suited
for embedded microcontroller implementations in general. This topic is addressed by the KEA publicV2G ISO 15118 stack.

The use case of the ISO 15118 implemented in this stack is charging functionallity and basic vehicle identification through the vehicle
identification number. The stack is suitable for AC charging and DC charging as well. This stack especially enables the developer to
implement real physical charging of vehicles if proposed hardware is used. Assuming this hardware is available, charging will be
possible in under an hour within an laboratory environment.
The stack is useful for development and integration in an own software environment and to get used to the ISO 15118. Several
intermediate use cases from just exchanging messages from PC to PC up to real physical charging are available and described.\n

Due to the definition of clear software interfaces to superior control and to the physical charging hardware the stack can be
integrated to own software and other hardware quite easily.

The software provided within this repository is open source and free to be used for private purpose or professional evaluation
purpose. For professional usage, especially within products, you require a license from KEA (Kompetenzzentrum für Elektronik
und Antriebstechnik, Science to Business GmbH - Hochschule Osnabrück). See the file licence.txt for the license conditions.\n
Our philosophy is providing software with very little financial investment at the time of development. Licensing is based on
the number of sold products at affordable costs in the range of the communication hardware. This model of licensing has an advantage for small
companies as it enhances development time and reduces development costs. Be aware the provided code inherits at least several man-years
of development time and saves a lot of initial product costs.\n
The available code is functional but inherits limitations usually required for professional use. You can apply at KEA
to receive the advanced code.

This library is intended to be used for embedded systems and is thus developed in pure C++ in a hardware independent manner.
The software is set on top of a well defined object oriented hardware abstaction layer. Hardware layer implementations are provided for
Windows and Linux. Ask KEA for hardware support of microcontrollers.

\section SEC_GETTINGSTARTED Getting started with sample programs

The following subsections explain some development application use cases. These sample program applications are provided within the
repository as ready to go implementations.

\subsection SEC_INSTALLATION Installation

For installation simply download this repository. The repository contains source code, make-files for compilation
and already precompiled binaries.

On Windows it is required to install the WinPcap ethernet layer two packet capture driver. (Please, check the licence conditions.)

- pcap packet socket (Windows only)

The most easiest way is to install the Wireshark program,
as this will install the WinPcap driver automatically. You need to manually check a check box to install with WinPcap API compatibility.\n
It is also possible to install the driver directly (https://www.winpcap.org/install/),
although the support of this intallation option is no longer supported and may fail in future.

For further development it might be useful to install Open SSL.

- OpenSSL (see: LibCpp::cTls)

and the

- mingw32 compiler on Windows
- gnu c/c++ compiler on Linux

\subsection SEC_SAMPLE_MESSAGE Example program on message level

\subsubsection SEC_SAMPLE_MESSAGE_INTRO Introduction

The easiest way to test this implementation for getting used to it is to run the software on a PC with either the Windows or Linux
operation system. You may use a single PC running the supply equipment (SE or electric vehicle supply equipment EVSE) representing
the charging column with one program instance and the electric vehicle EV as a second program instance.\n
You also may use two PCs, one as SE and the second as EV. Messages are exchanged through an Ehternet adapter even if you use
a single computer.

\image html GettingStartedMessage.svg

\subsubsection SEC_SAMPLE_MESSAGE_COMPILATION Setup of the configuration file

After installation (\ref SEC_INSTALLATION) enter the Projekt folder 'WinDemoMessage' on Windows or 'LxDemoMessage' on a Linux operation system.
You find a precompiled executable for arm64/x64 machines within the 'bin' folder.

In case you want to compile yourself, a projekt file for the QT-Creator IDE ist provided but you can use any IDE or a simple text editor
and compile by using the 'fMakefile' make file on the command line. Check if all files within the 'bin' folder exept the config.xml are
deleted in advance of the compilation (e.g. by calling: mingw32-make -f fMakefile clean).

For compilation call

- mingw32-make -f fMakefile (Windows)
- make -f fMakefile (Linux)

The 'sources' file contains the list of required source code files.

\subsubsection SEC_SAMPLE_MESSAGE_CONFIG Setup of the configuration file

Before running the program a few configurations concernig your setup are required to be supplied. To do so edit the config.xml file e.g.
using a normal text editor. Edit the file located in the 'bin' folder if your run the program from the command line.
Use the file within the 'build' folder in case you run from the QT-Creator IDE. A sample file is located in the project folder.

The configuration file 'config.xml' contains several sets of predefined configurations. Each of them is defined in its own 'Configuration'
section. Each section is indicated by an 'instance' property which identifies the specific configuration by a name.

Following this description please never include the single quotation mark ', but allways include the double quotation mark "" to the
configuration file.

- <Configuration instance="standard secc">

The configuration actually used by the program is defined within the property 'Configuration' of the section 'IsoCharging'. Enter the name
of the instance property of that configuration you want to use. This will probably be either 'standard secc' or 'standard evcc'.

- <IsoCharging Configuration="standard secc">

The following sections are contained within each 'Configuration' section:

- \b StackType
The section 'StackType' defines whether a SE shall be simulated (enter 'secc' to that section) or an EV shall be simulated (enter 'evcc').
- \b Interface
Enter the interface name of your Ethernet adapter you want to use for communication between the PC of the SE and the PC of the EV. On Windows
this is typically 'Ethernet' on Linux something like 'eth0'. In case you are unsure abaut the interface names of your computer use a
command line and execute 'ipconfig' on Windows systems and 'if address' (formerly 'ifconfig') on Linux systems. These will show the available
adapters with their interface name an the corresponding IP-Addresses.
- \b TLS
This section either set to 'false' or 'true' defines whether TLS encryption is used or not. In case you want to use TLS see the \ref PAGE_TLS
for explanations.
- \b TransferMode
Use the type of charging connector you want to support with your test setup. In most cases this will be 'AC-3phase' for usual 400V three
phase charging or 'DC-extended' for DC fast charging applications.
- \b EASourceIP
This section defines which physically hardware connected to your PC will be controlled by the stack. In case the given IP-address cannot
be reached by the program, no available hardware is assumed. For the simple getting started examples no change is required in this section.
- \b SchemaID
This section is depreciated and not used.

\subsubsection SEC_SAMPLE_MESSAGE_RUN Run the experiment on message level

\b AC-charging

In case your run this sample program on a single computer, it is required to copy the 'bin' folder to a 'binSE' folder. Configure the
configuration files within both folders, one for the SE (<IsoCharging Configuration="standard secc">) and the other for the EV
(<IsoCharging Configuration="standard evcc">).

Now you can start the executables 'DemoMessage.exe', each one in its own terminal. Allow the fire wall to access the Ethernet.
Typically the SE will be started in advance of the EV, although this sequence is not required by this
stack implementation. Nevertheless, this sequence might be required if you run this stack against other stack implementations. In case you
want to do so, see \ref PAGE_SUPPORTEDSTACKS.\n
Check for error or informative messages being printed on the screen. Information to solve errors is being supplied with the debug output
messages. For this simple getting started experiments, errors will probably arise due to false configurations of a false position of
the configuration file. Also make sure, your chosen adapter is conneted to the Ethernet and the Ethernet link is active, even if
you run two program instances on a single computer.

After starting the SE program there should be only 'INFO' outputs. The last output should be 'Entered state AwaitSdp'.

At the start of the program a small menu is printed to explain how you can control the charging process through your keyboard. There is
nothing to do on the SE side. The EV side will automatically begin with the charging process which can be finished by a keyboard command
according to the printed menu.

The exchanged messages between the EV and the SE will be printed so you can follow them and investigate proper operation.

The charging process is in operation in case '...' is displayed in the debug output. This does mean the previous message exchange
is running periodically without delivering periodic output. (You can change this inside the code by switching to 'debug' outputs.)

Press 's' on the EV side to friendly finish the charging session. You will see the appropriate messages.

\b DC-charging

To switch to a DC charging procedure change the appropriate configuration within the configuration file.
Use the sample configuration "DC secc" and "DC evcc" (<IsoCharging Configuration="DC secc">). Take care to adjust
the ethernet interface configuration within the used configuration sets.

Afterwards run the programs, the behavior will be similar to the previously discussed AC-charging.

\subsection SEC_SAMPLE_SIGNAL Usage on signal level

The sample application on signal level uses the power line communication (PLC) physical layer for transmitting the
ethernet message between the supply and the vehicle. It also includes the physical control pilot operation and
detection.

\image html GettingStartedSignal.svg

This sample setup already requires specific hardware. The expression 'signal level' expresses that the communication between the PC simulating
SE and EV is executed over real 'Power Line Communication' (PLC). For this setup an external modem converting Ethernet messages to PLC
messages is required. The provided code supports the '8devices' carryer board (evaluation board) for the 'White BEET' PLC controller subprint
distributed by 'CODICO'. You require one board configured as EV and one board configured as EVSE. There are differences on software and
hardware as well. The board requires the SLAC/Bridging Mode to be compatible to this stack. See https://www.8devices.com/products/white-beet,
https://www.codico.com/de/wb-carrier-board-ei-1-1-evse-embedded-iso15118-sw-stack-ev. Contact CODICO sales for further information.

Simply connect the PLC-board to the PC by an Ethernet cable. Take care to use the right board hardware (EV or EVSE) with the appropriately
configured stack on the PC side. There is no configuration to be made on the PLC-board. It is also not necessary to configure IP-Addresses on the PC
Ethernet Adapter as PC and PLC-board communicate on layer 2 messages using MAC-addresses without internet protocoll (IP).  The stack will
identify the PLC-board addresses automatically.The V2G communication uses IPv6 local link addresses also not requiring apdapter configuration.

Set the interface of the config file to the adapter the codoco board is connected to.

The sample project for signal level usage is located in the 'WinPrjPub' or 'LxPrjPub' folder. For basic software usage and compilation
you can follow the explanations within the section \ref SEC_SAMPLE_MESSAGE accordingly to run this experiment.

Please, first connect the PE and CP line between the EV codico board and the EVSE codico board.

Now start the executable 'IsoCharging.exe' withing the 'bin' folder. At Linux you need to start the program with 'sudo' rights.\n
Check whether all outputs are 'INFO' outputs with one exeption. At Linux you will get an error message after about 4 seconds. The software tries to
connect to a physical voltage source (cEAsource) which is not present. This message can be ignored. In case you get further 'ERROR' outputs
you need to check the reasons. Most probably there is a false config-file in use, the ethernet adapter is not properly set or you have chosen the
false codico hardware board. Take care the evse configuration of the software fits to the EVSE type codico board and vise versa.\n
If everything went well your last output should be: 'Send setCmPwm 5%'. If the output is 100% disconnect and reconnect the CP line.

Now start the executable 'IsoCharging.exe' on the second computer being configured as 'evse'. Also check the outputs
for errors. The charging process does start immediately. You can stop the process by typing 's'.

To initiate a further charging process type 'q' to quit the program on the EV side and restart the program.

If you like, check the procedure for DC charging also, by changing the config files.

\subsection SEC_SAMPLE_OPERATIONAL Usage on operational level

The 'Operational level' setup uses the stack together with physical power electronics. Therefore, this setup builds an emulation
system for charging colums or vehicles. By using this setup you can charge a real electric vehicle using this stack and corresponding
hardware for a SE emulation and vice versa. Assuming a suitable EA power source (offered by Tektronix) is being available together with a
CCS charging plug, this setup enables you to charge a real electric vehicle within under an hour of work.

\image html GettingStartedOperational.svg

You also can use such an emulation setup e.g. as a vehicle in order to support your develop of a supply equipment.

The documentation of this setup is still in progress. Contact KEA in case you need support.

\section SEC_LICENSE Licence and Disclaimer






/*
\defgroup G_publicV2G The KEA publicV2G project


- \ref Page_M
Main_Windows_Public

\section SEC_USECASES Use cases

Apart from the sample programs there are some typical use cases the ISO 15118 publicV2G stack software can be used for.

- \ref Page_IsoStackCtrlProtocol

This is a use case where the stack software is beeing executed on a seperate controler hardware
or executable and the supply equipment control accesses the stack through messages. Either by ethernet as in
the given example or any other physics, transport levels and protocols.





The publicV2G stack enables the initiation of an ISO 15118 charging process.

For introductiory general information see:

- [Overview](../README.md)

\section SEC_CONTENT Content

Provided source code libraries are:

- \ref Page_LibIso15118
- \ref Page_LibCpp

*/
