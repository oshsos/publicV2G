#ifndef CCALLBACK_H
#define CCALLBACK_H

#include <vector>

namespace LibCpp
{

template <typename iClass>
class cCallback
{
public:
    virtual bool setUpdateCallback(iClass* pInstance);              ///> Sets an instance to be called at message receptions.
    virtual bool deleteUpdateCallback(iClass* pInstance = nullptr); ///> Removes an instance from the callback list.

protected:
    std::vector<iClass*> callbacks;                                 ///> List of registered callback instances
};

}

// ------------------ template implementation --------------
/**
 * @brief Registeres a callback instance for receiving update events
 * This method and its corresponding member 'callbacks' is implemented in a nonthreadsave manner but overriding
 * is usually not necessary, despite you want to create warning messages in case of misuse. Thread save
 * implementations are mostly not intended
 * as message processing would slow down (despite you want to throw warnings on misusage).
 * @param pInstance
 * @return
 */
template <typename iClass>
bool LibCpp::cCallback<iClass>::setUpdateCallback(iClass* pInstance)
{
    if (!pInstance) return true;
    deleteUpdateCallback(pInstance);          // Avoids the existence of the same instance twice in the list
    callbacks.push_back(pInstance);
    return true;
}

/**
 * @brief Unregisteres a callback instance for receiving update events
 * See LibCpp::cFramePort::setCallback for documentaion of method behavior.\n
 * Calling this method parameterless (pInstance = nullptr) shall clear all registered callback instances.
 * @param pInstance
 * @return Confirms, the instance is not part of the list (either not any more or never has been)
 */
template <typename iClass>
bool LibCpp::cCallback<iClass>::deleteUpdateCallback(iClass* pInstance)
{
    if (!pInstance)
    {
        callbacks.clear();
    }
    for (int i = (int)callbacks.size() - 1; i >= 0; i--)
        if( callbacks[i]==pInstance )
            callbacks.erase( callbacks.begin()+i );
    return true;
}

#endif
