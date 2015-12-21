#pragma once

namespace imajuscule
{
    class OSAbstraction
    {
    protected:
        OSAbstraction();
        
        virtual ~OSAbstraction();

    public:
        static const OSAbstraction * get();
        static OSAbstraction * edit();
        
        void registerAbstraction(OSAbstraction *);
        void unregisterAbstraction(OSAbstraction *);

        virtual void PauseInteractions(bool) = 0;

        virtual void * getNativeWindowHandle() = 0;

        virtual bool getCursorPos(int&x, int&y) const = 0;
        
        virtual bool hasKeyboard() const {return true;}
    private:
        static OSAbstraction * m_registered;
    };
}
