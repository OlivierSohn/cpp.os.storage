#pragma once

namespace imajuscule
{
    class OSAbstraction
    {
    protected:
        virtual ~OSAbstraction() {}

    public:
        static const OSAbstraction * get();
        static OSAbstraction * edit();
        
        static void registerAbstraction(OSAbstraction *);
        static void unregisterAbstraction(OSAbstraction *);

        virtual void PauseInteractions(bool) = 0;

        virtual void * getNativeWindowHandle() = 0;

        virtual bool getCursorPos(int&x, int&y) const = 0;
        
        enum direction { DIRECTION_NONE = 0, DIRECTION_UP = 1, DIRECTION_DOWN = 2, DIRECTION_LEFT = 4, DIRECTION_RIGHT = 8 };
        virtual bool getDirectionBits( int & ) = 0;
        
        virtual bool hasKeyboard() const {return true;}
    private:
        static OSAbstraction * m_registered;
    };
    
    class TestOSAbstraction : public OSAbstraction {
    public:
        TestOSAbstraction() : OSAbstraction() {}
        
        void PauseInteractions(bool b) override {
            interaction_paused = b;
        }
        
        void * getNativeWindowHandle() override { return 0; }
        
        bool getCursorPos(int &, int &) const override {
            return false;
        }
        
        bool getDirectionBits(int &) override {
            return false;
        }
    private:
        bool interaction_paused = false;
    };
}
