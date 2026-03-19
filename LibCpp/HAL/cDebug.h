#ifndef CDEBUG_H
#define CDEBUG_H

#include <string>

namespace LibCpp
{

// NOTE    Liefert noch mehr Ausgaben als Debug und wird hauptsächlich in Codeabschnitten eingesetzt, die im Normalbetrieb durchlaufen werden. Ziel ist die detailreicht Unterstützung einer Fehlersuche.
// DEBUG   Liefert wird hauptsächlich in Codeabschnitten eingesetzt, die im Normalbetrieb nicht durchlaufen werden oder dort zentrale Durchlaufpunkte darstellen, die Ursache von nachfolgenden Fehlern sind. Ziel sind ausführliche Daten, die eine spätere Fehlerdiagnose innerhalb von logging-Dateien zulassen.
// LOGGING Wird im Grundsatz wie DEBUG eingesetzt, aber auf wesentliche Informationen beschränkt, so dass die Größe von logging-Dateien begrenzt werden kann.
// ERROR   Dient der Anzeige von Fehlern, die für den Anwender die Funktion des Programmes deutlich einschränken, durch das Programm aber in einer vorgesehenen Weise behandelt werden.
// WARNING Dient der Anzeige von Informationen, die für den Anwender zu Informationszwecken vorgesehen sind und in Programmabschnitten durchlaufen werden, die eine Handlung des Anwenders erfordern, um den Normalbetrieb wiederherzustellen.
// INFO    Dient der Anzeige von Informationen, die für den Anwender zu Informationszwecken vorgesehen sind und im normalen Programmablauf vorkommen.
// FATAL   Fehler, die zu einem Abbruch des Programmes führen

enum enDebugLevel
{
    enDebugLevel_Debug,
    enDebugLevel_Logging,
    enDebugLevel_Info,
    enDebugLevel_Error,
    enDebugLevel_Fatal,
    enDebugLevel_None,
    enDebugLevel_Successor
};

std::string enDebugLevel_toString(enDebugLevel level);

class cDebug
{
public:
    cDebug();
    cDebug(const char* typeName, enDebugLevel level);
    cDebug(const char* typeName, cDebug* pSuccessor = 0, enDebugLevel level = enDebugLevel_Successor);
    static cDebug& instance();
    inline void setInstanceName(const char* instanceName, cDebug* pSuccessor = nullptr, enDebugLevel level = enDebugLevel_Successor) {if (pSuccessor) this->pSuccessor = pSuccessor; this->instanceName = instanceName; if (level != enDebugLevel_Successor) setDebugLevel(level);};
    inline const char* getInstanceName() {return instanceName;};
    virtual void setDebugLevel(enDebugLevel level = enDebugLevel_Debug);
    virtual void setNewLineOutput(bool newLine = true);
    inline enDebugLevel getDebugLevel() {if (debugLevel == enDebugLevel_Successor && pSuccessor) return pSuccessor->getDebugLevel(); else return debugLevel;};
    virtual void printf(enDebugLevel level, const char* message, ...);

protected:
    virtual void printf_recursive(enDebugLevel level); //, bool newLine = false);

protected:    
    cDebug* pSuccessor;
    const char* typeName;
    const char* instanceName;
    enDebugLevel debugLevel;
    bool newLine;
};

}

#endif // CDEBUG_H
