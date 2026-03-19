// verkettetes Anlegen von Instanzen des cDebug-Objektes erhält man eine rekursive Ausgabe der Fehlerquelle ausgehend vom Hauptprogramm inclusive der
// Angabe der auslösenden Objektinstanzen.
//cLxTcpServer::cLxTcpServer() :
//    dbg(&debugLogger, "cLxTcpServer")
//{
//    cDebug dbg(&this->dbg, "Constructor");
//    bzero(&localAddress, sizeof(localAddress));
//    dbg.printf(enDebugLevel_Info, "Successfully initialized\n");
//}
//int main()
//{
//    cDebug::dbg()->setDebugLevel();
//    cDebug dbg(main"); Eine globale instanz macht eigentlich keinen Sinn, es ist verständlicher, diese explizit anzulegen. Dann können auch abgeleitete cDebug Objekte mit anderen Funktionalitäten generiert werden.
//debugLogger.setDebugLevel(); Als globale Funktionen bereitstellen, so dass man die instanz debugLogger nicht kennen muss
//server.dbg.setInstanceName("server", &dbg);
// Zeitangabe ergänzen (wenn hardwareunabhängig möglich)
// Fehlerklassen erläutern
// static function wie globale Funktionen example
// Callback einbauen
// später states einbauen


#include <stdio.h>
#include <cstdarg>

#include "cDebug.h"

using namespace LibCpp;

cDebug* pDbgGlobal = nullptr;
//cDebug* LibCpp::pDebug = &dbgGlobal;

/**
 * @brief Converts a enDebugLevel to a string
 * @param level
 * @return
 */
std::string enDebugLevel_toString(enDebugLevel level)
{
    switch(level)
    {
    case enDebugLevel_Debug: return "DEBUG";
    case enDebugLevel_Logging: return "LOGGING";
    case enDebugLevel_Error: return "ERROR";
    case enDebugLevel_Info: return "INFO";
    case enDebugLevel_Fatal: return "FATAL";
    case enDebugLevel_None: return "NONE";
    case enDebugLevel_Successor: return "SUCCESSOR";
    default: return "<undefined>";
    };
}

// Creates a global cDebug instance without an successor instance (Mainly to be used as base instance for an independent thread.)
cDebug::cDebug()
{
    if (!pDbgGlobal)
    {
        pDbgGlobal = (cDebug*)1;
        pDbgGlobal = new(cDebug)("cDebug");
    }
    debugLevel = enDebugLevel_Successor;
    pSuccessor = nullptr;
    typeName = "";
    instanceName = "";
    newLine = false;
}

cDebug::cDebug(const char* typeName, enDebugLevel level)
{
    if (!pDbgGlobal)
    {
        pDbgGlobal = (cDebug*)1;
        pDbgGlobal = new(cDebug)("cDebug");
    }
    pSuccessor = pDbgGlobal;
    debugLevel = level;
    pSuccessor->setDebugLevel(level);
    this->typeName = typeName;
    instanceName = "";
    newLine = false;
}

// Creates a dependent cDebug instance. If no dependence is specified by 'pSuccesser' the standard global instance 'pDebug' points to is used.
cDebug::cDebug(const char* typeName, cDebug* pSuccessor, enDebugLevel level)
{
    if (!pDbgGlobal)
    {
        pDbgGlobal = (cDebug*)1;
        pDbgGlobal = new(cDebug)("cDebug");
    }
    this->typeName = typeName;
    instanceName = "";
    newLine = false;
    cDebug::setDebugLevel(level);
    if (pSuccessor && pSuccessor!=(cDebug*)1)
        this->pSuccessor = pSuccessor;
    else
    {
        if (this == pDbgGlobal || pDbgGlobal==(cDebug*)1)
        {
            this->pSuccessor = nullptr; // pDebug is not initialized yet.
            instanceName = "dbgGlobal";
        }
        else
        {
            this->pSuccessor = pDbgGlobal;
        }
    }
}

cDebug& cDebug::instance()
{
    if (!pDbgGlobal)
    {
        pDbgGlobal = (cDebug*)1;
        pDbgGlobal = new(cDebug)("cDebug");
    }
    return *pDbgGlobal;
}

void cDebug::printf(enDebugLevel level, const char* message, ...)
{
    enDebugLevel printLevel = getDebugLevel();
    if (level<printLevel) return;

//    bool newLine = message[0] == '\n';
//    if (newLine) message++;

    printf_recursive(level);//, newLine);

    va_list arg_ptr;
    va_start(arg_ptr, message);
    ::printf(": ");
    if (message[0] == '\n')
    {
        ::printf("\n-----> ");
        ::vprintf(message+1, arg_ptr);
    }
    else
        ::vprintf(message, arg_ptr);
    ::printf("\n");
    va_end(arg_ptr);
    ::fflush(stdout);
}

void cDebug::printf_recursive(enDebugLevel level)//, bool newLine)
{
    if (pSuccessor && (this != pDbgGlobal))
    {
        pSuccessor->printf_recursive(level); //, newLine);
        ::printf("::%s", typeName);
        if (instanceName[0]!=0)
            ::printf(" %s ", instanceName);
        return;
    }

    if (debugLevel == enDebugLevel_Successor) debugLevel = enDebugLevel_None;

    if (this->newLine) // || newLine)
        ::printf("\n");

    switch (level)
    {
    case enDebugLevel_Debug:    ::printf("DEBUG: "); break;
    case enDebugLevel_Logging:  ::printf("LOG  : "); break;
    case enDebugLevel_Error:    ::printf("ERROR: "); break;
    case enDebugLevel_Info :    ::printf("INFO : "); break;
    case enDebugLevel_Fatal:    ::printf("FATAL: "); break;
    default: return;
    }
    if (typeName[0]!=0)
        ::printf("::%s", typeName);
    if (instanceName[0]!=0)
        ::printf(" %s ", instanceName);
}

void cDebug::setDebugLevel(enDebugLevel level)
{
    if (pSuccessor == pDbgGlobal)
        if (pDbgGlobal->debugLevel == enDebugLevel_Successor)
            pDbgGlobal->setDebugLevel(level);
    debugLevel = level;
}

void cDebug::setNewLineOutput(bool newLine)
{
    this->newLine = newLine;
}
