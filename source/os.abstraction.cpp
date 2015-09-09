#include <cstddef> // NULL
#include "os.abstraction.h"
#include "os.log.h"

using namespace imajuscule;
OSAbstraction * OSAbstraction::m_registered = NULL;
int OSAbstraction::nModifiers = 0;

const OSAbstraction * OSAbstraction::get()
{
    A(m_registered);
    return m_registered;
}
OSAbstraction * OSAbstraction::edit()
{
    A(m_registered);
    return m_registered;
}

OSAbstraction::OSAbstraction()
{
    
}
OSAbstraction::~OSAbstraction()
{
    
}

void OSAbstraction::registerAbstraction(OSAbstraction * a)
{
    A(!m_registered);
    m_registered = a;
}
void OSAbstraction::unregisterAbstraction(OSAbstraction * a)
{
    A(m_registered == a);
    m_registered = NULL;
}

void OSAbstraction::pushModifier()
{
    nModifiers++;
}
void OSAbstraction::popModifier()
{
    nModifiers--;
    if(nModifiers<0)
    {
        A(0);
        nModifiers = 0;
    }
}
bool OSAbstraction::hasModifier()
{
    return (nModifiers>0);
}

