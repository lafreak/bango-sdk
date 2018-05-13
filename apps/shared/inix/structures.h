#pragma once
#pragma pack(push, 1)

struct FUSEINFO
{
    unsigned char Level;
    unsigned short Meele, Magic, Defense, Absorb;
    unsigned char Dodge, Hit, HP, MP;
    unsigned char Stat[5];
};

// 63 bytes
struct ITEMINFO
{
    unsigned short Index;
    int IID;
    unsigned char Prefix;
    unsigned int Info;
    unsigned int Num;
    unsigned char MaxEnd;
    unsigned char CurEnd;
    unsigned char SetGem;
    unsigned char XAttack;
    unsigned char XMagic;
    unsigned char XDefense;
    unsigned char XHit;
    unsigned char XDodge;
    unsigned short ProtectNum;
    unsigned char WeaponLevel;
    unsigned char CorrectionAddNum;
    unsigned char Unknown1;
    unsigned char RemainingSeconds;//??
    unsigned char RemainingMinutes;//??
    unsigned int RemainingHours;//??
    FUSEINFO FuseInfo;
    unsigned char Shot;
    unsigned short Perforation;
    unsigned int GongA;
    unsigned int GongB;
};

// 65 bytes
struct PLAYERINFO
{
    int AccountID;
    int PlayerID;
    unsigned char Class;
    unsigned char Job;
    unsigned char Level;
    unsigned short Strength;
    unsigned short Health;
    unsigned short Inteligence;
    unsigned short Wisdom;
    unsigned short Dexterity;
    unsigned int CurHP;
    unsigned int CurMP;
    unsigned long Exp;
    unsigned short PUPoint;
    unsigned short SUPoint;
    unsigned short Contribute;
    unsigned int Rage;
    unsigned int X;
    unsigned int Y;
    unsigned int Z;
    unsigned char Face;
    unsigned char Hair;
};

#pragma pack(pop)