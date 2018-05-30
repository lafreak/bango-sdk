#pragma once

struct InitItem : public db_object<InitItem>
{
    unsigned int 
        Index,
        Class,
        Kind,
        Level,
        Endurance;

    unsigned int index() const { return Index; }

    virtual void set(lisp::var param) override
    {
        switch (FindAttribute(param.pop()))
        {
            case A_INDEX:           Index         = param.pop(); break;
            case A_LEVEL:           Level         = param.pop(); break;
            case A_ENDURANCE:       Endurance     = param.pop(); break;
            case A_CLASS:           Class         = FindAttribute(param.pop());
                                    Kind          = FindAttribute(param.pop());
                                    break;
        }
    }
};