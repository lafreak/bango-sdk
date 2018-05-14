#include "Player.h"

// Database has loaded player related data.
void Player::OnLoadPlayer(packet& p)
{
    p >> m_data >> m_name;

    get_session()->write(S2C_PROPERTY, "bsbwwwwwwddwwwwwbIwwwwwwbbbbbd",
        0, //Grade
        "\0", //GuildName
        0, //GRole
        GetContribute(),
        GetStrength(),
        GetHealth(),
        GetInteligence(),
        GetWisdom(),
        GetDexterity(),
        GetCurHP(),             //!
        GetCurHP(), //MaxHP     //!
        GetCurMP(),
        GetCurMP(), //MaxMP
        1, //Hit
        2, //Dodge
        3, //Defense            //!
        4, //Absorb             //!
        GetExp(),               //!
        5, //MinAttack
        6, //MaxAttack
        7, //MinMagic
        8, //MaxMagic
        GetPUPoint(),
        GetSUPoint(),           //!
        9, //ResistFire
        10, //ResistIce
        11, //ResistLitning
        12, //ResistCurse
        13, //ResistPalsy
        GetRage());

    short time = 1200; // 12:00
    get_session()->write(S2C_ANS_LOAD, "wdd", time, GetX(), GetY());
}

// Database has loaded items.
void Player::OnLoadItems(packet& p)
{
    get_session()->write(p.change_type(S2C_ITEMINFO));

    for (unsigned short i = 0, amount = p.pop<unsigned short>(); i < amount; i++)
        m_inventory.Insert(std::make_shared<Item>(p.pop<ITEMINFO>()));
}

// Client has loaded map and is ready to render.
void Player::GameStart(packet& p)
{
    auto unknown = p.pop<char>();
    auto height = p.pop<int>();

    // Map thing
    get_session()->write(S2C_CREATEPLAYER, "dsbdddwIwwwwwwwwbbIssdbdddIIbddb",
        GetPlayerID(), // CharacterID!
        GetName().c_str(),
        GetClass() | GAME_HERO,
        GetX(),
        GetY(),
        GetZ(),
        0, //Dir
        0, //GState
        0, //GI Weapon
        0, //GI Shield
        0, //GI Helmet
        0, //GI Upper
        0, //GI Lower
        0, //GI Gauntlet
        0, //GI Boots
        0, //GI Costume
        GetFace(),
        GetHair(),
        0, //MState
        "\0", //GuildClass
        "\0", //GuildName
        0, //GID
        0, //Flag
        0, //FlagItem
        0, //HonorGrade
        0, //HonorOption
        0, //GStateEx
        0, //MStateEx

        0, //Unk
        0, //Unk
        0, //Unk
        0  //Unk
        );
}

void Player::GameRestart()
{
}