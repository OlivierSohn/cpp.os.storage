#pragma once

namespace imajuscule
{
    class OSAbstraction
    {
    public:
        OSAbstraction();
        
        virtual ~OSAbstraction();
        
        static const OSAbstraction * get();
        static OSAbstraction * edit();
        
        void registerAbstraction(OSAbstraction *);
        void unregisterAbstraction(OSAbstraction *);

        virtual void PauseInteractions(bool) = 0;

        virtual void * getNativeWindowHandle() = 0;

        static void pushModifier();
        static void popModifier();
        static bool hasModifier();
    private:
        static OSAbstraction * m_registered;
        
        static int nModifiers;
    };
}