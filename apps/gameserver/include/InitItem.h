#pragma once

struct InitItem
{
    unsigned int 
        Index,
        Class,
        Kind,
        Level,
        Endurance;

    unsigned int index() const { return Index; }

    struct Container : public db<InitItem, Container>
    {
        Container() 
        { 
            m_attributes = {
                {"index",       A_INDEX},
                {"class",       A_CLASS},
                {"level",       A_LEVEL},
                {"endurance",   A_ENDURANCE},

                {"weapon", IC_WEAPON},
                {"defense", IC_DEFENSE},
                {"ornament", IC_ORNAMENT},
                {"general", IC_GENERAL},
                {"quest", IC_QUEST},
                {"money", IC_MONEY},
                {"transform", IC_TRANSFORM},
                {"pet", IC_PET},
                {"limitedpet", IC_PET},
                {"yinyang", IC_YINYANG},
                {"ride", IC_RIDE},

                {"etc", 	    ISC_ETC },
                {"fish", 		ISC_FISH },
                {"sword", 		ISC_SWORD },
                {"sword2h", 	ISC_SWORD2HAND },
                {"wand", 		ISC_WAND },
                {"bow", 		ISC_BOW },
                {"shield", 		ISC_SHIELD },
                {"helmet", 		ISC_HELMET },
                {"upperarmor", 	ISC_UPPERARMOR },
                {"lowerarmor", 	ISC_LOWERARMOR },
                {"gauntlet", 	ISC_GAUNTLET },
                {"boots", 		ISC_BOOTS },
                {"ring", 		ISC_RING },
                {"trinket", 	ISC_TRINKET },
                {"refresh", 	ISC_REFRESH },
                {"charm", 		ISC_CHARM },
                {"gem", 		ISC_GEM },
                {"coin", 		ISC_COIN },
                {"repair", 		ISC_REPAIR },
                {"necklace", 	ISC_NECKLACE },
                {"resistnecklace2", ISC_RESISTNECKLACE2 },
                {"cocoon", 		ISC_COCOON },
                {"mask", 		ISC_MASK },
                {"standard", 	ISC_STANDARD },
                {"dagger",		ISC_DAGGER },
                {"yinyangmirror", ISC_YINYANGMIRROR },
                {"taegeuk", 	ISC_TAEGEUK },
                {"trigramSlot1", ISC_TRIGRAM1 },
                {"trigramSlot2", ISC_TRIGRAM2 },
                {"trigramSlot3", ISC_TRIGRAM3 },
                {"trigramSlot4", ISC_TRIGRAM4 },
                {"trigramSlot5", ISC_TRIGRAM5 },
                {"trigramSlot6", ISC_TRIGRAM6 },
                {"trigramSlot7", ISC_TRIGRAM7 },
                {"trigramSlot8", ISC_TRIGRAM8 },
                {"egg", 		ISC_EGG },
                {"enchantnecklace", ISC_ENCHANTNECKLAKE },
                {"cost",		ISC_COSTUME },

                // Player Class
                {"knight",		PC_KNIGHT },
                {"mage", 		PC_MAGE },
                {"archer", 		PC_ARCHER },
                {"thief", 		PC_THIEF },
                {"shaman", PC_SHAMAN },
            };
        }

        enum ATTRIBUTES
        {
            A_NONE,
            A_INDEX,
            A_CLASS,
            A_LEVEL,
            A_ENDURANCE,
        };

        virtual void process(InitItem& e, lisp::var param) override
        {
            switch (attribute(param.pop()))
            {
                case A_INDEX:           e.Index         = param.pop(); break;
                case A_LEVEL:           e.Level         = param.pop(); break;
                case A_ENDURANCE:       e.Endurance     = param.pop(); break;
                case A_CLASS:           e.Class         = attribute(param.pop());
                                        e.Kind          = attribute(param.pop());
                                        break;
            }
        }

    };

    static InitItem*    Find(unsigned int index)    { return Container::find(index); }
    static bool         Load(const char* name)      { return Container::load(name); }
};