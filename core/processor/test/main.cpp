#include <bango/processor/db.h>

#include <iostream>

using namespace bango::processor;

struct InitItem
{
    unsigned int Index;
    unsigned int Name;

    unsigned int index() const { return Index; }

    struct Container : public db<InitItem, Container>
    {
        Container() 
        { 
            m_attributes = {
                {"index",   A_INDEX},
                {"name",    A_NAME},
            };
        }

        enum ATTRIBUTES
        {
            A_NONE,
            A_NAME,
            A_INDEX,
        };

        virtual void process(InitItem& item, lisp::var param) override
        {
            switch (attribute(param.pop()))
            {
                case A_INDEX:   item.Index =    param.pop(); break;
                case A_NAME:    item.Name =     param.pop(); break;
            }
        }

    };

    static InitItem*    Find(unsigned int index)    { return Container::find(index); }
    static bool         Load(const char* name)      { return Container::load(name); }

private:
};


int main(int argc, char **argv)
{
    InitItem::Load("InitItem.txt");
    InitItem* pItem = InitItem::Find(3);
    std::cout << pItem->Name << std::endl; // 258

    return 0;
}