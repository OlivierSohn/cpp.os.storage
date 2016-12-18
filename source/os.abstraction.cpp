
using namespace imajuscule;
OSAbstraction * OSAbstraction::m_registered = nullptr;

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

void OSAbstraction::registerAbstraction(OSAbstraction * a)
{
    A(!m_registered);
    m_registered = a;
}
void OSAbstraction::unregisterAbstraction(OSAbstraction * a)
{
    A(m_registered == a);
    m_registered = nullptr;
}
