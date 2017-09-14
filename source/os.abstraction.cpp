
using namespace imajuscule;
OSAbstraction * OSAbstraction::m_registered = nullptr;

const OSAbstraction * OSAbstraction::get()
{
    Assert(m_registered);
    return m_registered;
}
OSAbstraction * OSAbstraction::edit()
{
    Assert(m_registered);
    return m_registered;
}

void OSAbstraction::registerAbstraction(OSAbstraction * a)
{
    Assert(!m_registered);
    m_registered = a;
}
void OSAbstraction::unregisterAbstraction(OSAbstraction * a)
{
    Assert(m_registered == a);
    m_registered = nullptr;
}
