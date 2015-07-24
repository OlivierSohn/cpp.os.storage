#pragma once

namespace imajuscule
{
    class OSAbstraction
    {
    public:
        OSAbstraction();
        
        virtual ~OSAbstraction();
        
        static OSAbstraction * get();
        
        void registerAbstraction(OSAbstraction *);
        void unregisterAbstraction(OSAbstraction *);

        virtual void PauseInteractions(bool) = 0;

        virtual void * getNativeWindowHandle() = 0;

    private:
        static OSAbstraction * m_registered;
    };
}