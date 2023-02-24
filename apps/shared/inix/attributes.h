#pragma once

#include <inix/common.h>

#include <map>
#include <string>

namespace inix
{
    struct ci_less
    {
        // case-independent (ci) compare_less binary function
        struct nocase_compare
        {
            bool operator()(const unsigned char &c1, const unsigned char &c2) const
            {
                return tolower(c1) < tolower(c2);
            }
        };
        bool operator()(const std::string &s1, const std::string &s2) const
        {
            return std::lexicographical_compare(s1.begin(), s1.end(), // source range
                                                s2.begin(), s2.end(), // dest range
                                                nocase_compare());    // comparison
        }
    };

    static std::map<std::string, int, ci_less> g_attributes = {
        // non inix
        {"endurance",   A_ENDURANCE},

        //inix
        {"weapon",      IC_WEAPON},
        {"defense",     IC_DEFENSE},
        {"ornament",    IC_ORNAMENT},
        {"general",     IC_GENERAL},
        {"quest",       IC_QUEST},
        {"money",       IC_MONEY},
        {"transform",   IC_TRANSFORM},
        {"pet",         IC_PET},
        {"limitedpet",  IC_PET},
        {"yinyang",     IC_YINYANG},
        {"ride",        IC_RIDE},

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
        {"shaman",      PC_SHAMAN },
        {"all",         PC_ALL },

        {"soul",        ISC_SOUL},

		{ "ability",A_ABILITY},
		{ "absorb", A_ABSORB},
		{ "action", A_ACTION},
		{ "active", A_ACTIVE},
		{ "aspeed", A_ASPEED},
		{ "attack", A_ATTACK},
		{ "attheight", A_ATTHEIGHT},
		{ "attack_direction", A_ATTACK_DIRECTION},
        { "blood", A_BLOOD},
		{ "bmpname", A_BMPNAME},
		{ "bone", A_BONE},
		{ "buy", A_BUY},
		{ "changeprefix", A_CHANGEPREFIX},
		{ "charming", A_CHARMING},
		{ "class", A_CLASS},
		{ "cocoon", A_COCOON},
		{ "code", A_CODE},
		{ "combo", A_COMBO},
		{ "compare", A_COMPARE},		
		{ "condition", A_CONDITION},
		{ "config", A_CONFIG},
		{ "cooltime", A_COOLTIME},
        { "cooltime2", A_COOLTIME2},
		{ "cooltime3", A_COOLTIME3},
		{ "correction", A_CORRECTION},		
		{ "country", A_COUNTRY},
		{ "damage", A_DAMAGE},
		{ "damage_direction", A_DAMAGE_DIRECTION},
		{ "damagetype", A_DAMAGETYPE},		
		//{ "defense", A_DEFENSE},
		{ "delaytime", A_DELAYTIME},
		{ "desc", A_DESC},
		{ "dex", A_DEX},
		{ "dodge", A_DODGE},
		{ "effect", A_EFFECT},
		{ "exist", A_EXIST},
		{ "explosion", A_EXPLOSION},
        { "fadein", A_FADEIN},
        { "filename", A_FILENAME},
		{ "flag", A_FLAG},
        { "framerate", A_FRAMERATE},
        { "fx", A_FX},
		{ "gatename", A_GATENAME},
		{ "grade", A_GRADE},
		{ "height", A_HEIGHT},
		{ "hit", A_HIT},
		{ "honoritem", A_HONOR},
		{ "hp", A_HP},
		{ "hth", A_HTH},
		{ "image", A_IMAGE},
		{ "imageex", A_IMAGEEX},
		{ "index", A_INDEX},
		{ "int", A_INT},
		{ "ip", A_IP},
        { "item", A_ITEM},
		{ "key", A_KEY},
		{ "level", A_LEVEL},
		{ "limit", A_LIMIT},
		{ "loop", A_LOOP},
		{ "magic", A_MAGICATTACK},
		{ "map_bottom", A_MAPBOTTOM},		
		{ "map_left", A_MAPLEFT},
		{ "mapname", A_MAPNAME},		
		{ "maxlevel", A_MAXLEVEL},
		{ "maxparamvalue", A_MAXPARAMVALUE},
		{ "maxprotect", A_MAXPROTECT},		
		{ "motion", A_MOTION},
		{ "movemotion", A_MOVEMOTION},
		{ "mp", A_MP},
		{ "mspeed", A_SPEED},
		{ "name", A_NAME},
        { "namebone", A_NAMEBONE},
        { "nameex", A_NAMEEX},
        { "normalmotion", A_NORMAL_MOTION},
        { "object", A_OBJECT},        
		{ "page", A_PAGE},
		{ "param", A_PARAMETER},
		{ "parameter", A_PARAMETER},
		{ "part", A_PART},
		{ "particle", A_PARTICLE},
        { "pay", A_PAY},
		{ "pileskill", A_PILESKILL},		
		{ "plural", A_PLURAL},
		{ "port", A_PORT},
        { "property", A_PROPERTY},
		{ "protect", A_PROTECT},
		{ "range", A_RANGE},
		{ "rank", A_RANK},
		{ "resistcurse", A_RESISTCURSE},
		{ "resistfire", A_RESISTFIRE},
		{ "resistice", A_RESISTICE},
		{ "resistlitning", A_RESISTLITNING},
		{ "resistpalsy", A_RESISTPALSY},
		{ "revival", A_REVIVAL},
		{ "scale", A_SCALE},
        { "script", A_SCRIPT},
		{ "sell", A_SELL},
		{ "servernumber", A_SERVERNUMBER},
		{ "size", A_SIZE},
		{ "skill", A_SKILL},
		{ "sound", A_SOUND},
		{ "specialty", A_SPECIALTY},
		{ "str", A_STR},
		{ "subdesc", A_SUBDESC},
		{ "target", A_TARGET},
        { "targetinfo", A_TARGETINFO},        
		{ "targetkind", A_TARGETKIND},
        { "targetting", A_TARGETTING},
        { "taskquest", A_TASKQUEST},
		{ "test", A_TEST},
		{ "total", A_TOTAL},
        { "trigger", A_TRIGGER},
		{ "type", A_TYPE},
		{ "use", A_USEABLE},
		{ "wear", A_WEARABLE},
		{ "width", A_WIDTH},
		{ "wis", A_WIS},

        // InitNPC
        { "kind", A_KIND},
        { "shape", A_SHAPE},
        { "html", A_HTML},
        { "map", A_MAP},
        { "xy", A_XY},
        { "dir", A_DIR},

        // InitItem
        { "ridingtype", A_RIDINGTYPE},

        // InitMonster
        { "race", A_RACE},
        { "ai", A_AI},
        { "sight", A_SIGHT},
        { "exp", A_EXP},
        { "resist", A_RESIST},

        // GenMonster
        { "area", A_AREA},
        { "max", A_MAX},
        { "cycle", A_SPAWNCYCLE},
        { "rect", A_RECT},

        // ItemGroup
        {"group", A_GROUP},
        {"itemgroup", A_ITEMGROUP}

    };

    static std::map<unsigned int, int> g_wearables = {
        // Weapons
        {ISC_SWORD, WS_WEAPON},
        {ISC_SWORD2HAND, WS_WEAPON},
        {ISC_WAND, WS_WEAPON},
        {ISC_BOW, WS_WEAPON},
        {ISC_DAGGER, WS_WEAPON},
        {ISC_SOUL, WS_WEAPON},
        
        // Shields
        {ISC_SHIELD, WS_SHIELD},

        // Armors
        {ISC_HELMET, WS_HELMET},
        {ISC_UPPERARMOR, WS_UPPERARMOR},
        {ISC_LOWERARMOR, WS_LOWERARMOR},
        {ISC_GAUNTLET, WS_GAUNTLET},
        {ISC_BOOTS, WS_BOOTS},
        
        // Accessories
        {ISC_RING, WS_RING},
        {ISC_NECKLACE, WS_NECKLACE},
        {ISC_RESISTNECKLACE2, WS_RESISTNECKLACE2},
        {ISC_TRINKET, WS_TRINKET},

        // Taegeuks

        // Animal
        {ISC_COCOON, WS_TRANSFORM},

        // Pet
        {ISC_EGG, WS_PET},

        // Flag

        // Battle Horse


/*
        {"weapon",      IC_WEAPON},
        {"defense",     IC_DEFENSE},
        {"ornament",    IC_ORNAMENT},
        {"general",     IC_GENERAL},
        {"quest",       IC_QUEST},
        {"money",       IC_MONEY},
        {"transform",   IC_TRANSFORM},
        {"pet",         IC_PET},
        {"limitedpet",  IC_PET},
        {"yinyang",     IC_YINYANG},
        {"ride",        IC_RIDE},

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
*/
    };
}

static int FindAttribute(const char* attribute) 
{
    auto result = inix::g_attributes.find(attribute);
    if (result != inix::g_attributes.end())
        return result->second;
    return -1;
}

static int FindWearId(int kind)
{
    auto result = inix::g_wearables.find(kind);
    if (result != inix::g_wearables.end())
        return result->second;
    return -1;
}