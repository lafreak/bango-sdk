#pragma once

enum S2C_PROTOCOL
{
    S2C_SECOND_LOGIN = 37,
    S2C_ANS_LOGIN = 149,
    S2C_CODE = 125,
    S2C_CLOSE = 66,
    S2C_EVENT_FISHING = 80,
    S2C_RESULT_CODE = 99,
    S2C_STORAGEINFO = 1,
    S2C_GIVEUP_QUEST = 18,
    S2C_QUESTINFO = 0,
    S2C_SHOWOFFITEM = 16,
    S2C_CLOSETRADE = 4,
    S2C_PLAYERINFO = 114,
    S2C_DELPLAYERINFO = 141,
    S2C_ANS_LOAD = 104,
    S2C_ANS_START = 21,
    S2C_ACTION = 137,
    S2C_PLAYER_ANIMATION = 168,
    S2C_PROPERTY = 145,
    S2C_CREATEPLAYER = 107,
    S2C_ANS_NEWPLAYER = 161,
    S2C_ANS_RESTART = 116,
    S2C_ANS_GAMEEXIT = 46,
    S2C_RIDING = 198,
    S2C_MESSAGE = 146,
    S2C_SIEGEGUNCONTROL = 174,
    S2C_MOVEPLAYER_ON = 15,
    S2C_MOVEPLAYER_END = 151,
    S2C_MOVEDASHPLAYER_ON = 118,
    S2C_MOVEDASHPLAYER_END = 171,
    S2C_REMOVEMONSTER = 119,
    S2C_REMOVEPLAYER = 135,
    S2C_REMOVENPC = 155,
    S2C_MOVEMONSTER_ON = 152,
    S2C_MOVEMONSTER_END = 173,
    S2C_CHATTING = 109,
    S2C_MOVEBEFORE = 139,
    S2C_UPDATEPROPERTY = 79,
    S2C_CREATENPC = 134,
    S2C_TELEPORT = 69,
    S2C_SENDHTML = 47,
    S2C_INSERTITEM = 133,
    S2C_INSERT_SITEM = 10,
    S2C_UPDATEITEMINFO = 157,
    S2C_UPDATEITEMNUM = 25,
    S2C_UDPATE_SITEMNUM = 77,
    S2C_REMOVEITEM = 87,
    S2C_TRASHITEM = 7,
    S2C_ITEMINFO = 124,
    S2C_UPDATEITEMEND = 89,
    S2C_EFFECT = 110,
    S2C_PUTONITEM = 159,
    S2C_PUTOFFITEM = 170,
    S2C_ATTACH_PET = 35,
    S2C_NOTICE = 169,
    S2C_SHORTCUT = 123,
    S2C_CREATEMONSTER = 147,
    S2C_ITEMSHOP = 186,
    S2C_ASKPARTY = 162,
    S2C_PARTYINFO = 56,
    S2C_UPDATEPARTY = 72,
    S2C_PARTYMEMPOS = 132,
    S2C_ATTACK = 144,
    S2C_SKILLUP = 128,
    S2C_SKILL = 120,
    S2C_EFFECT_EX = 76,
    S2C_SKILLINFO = 160,
    S2C_INFODIE = 143,
    S2C_RESTOREPLAYER = 94,
    S2C_ALIVE = 117,
    S2C_MAPSTATE = 122,
    S2C_OVERLAPSKILL = 115,
    S2C_MSTATE = 81,
    S2C_UPDATEMPROPERTY = 154,
    S2C_MESSAGEV = 9,
    S2C_GSTATE = 126,
    S2C_CREATEITEM = 153,
    S2C_SKILLADDVALUE = 5,
    S2C_SAVEQUEST = 54,
    S2C_ASKPVP = 92,
    S2C_MLM = 26,
    S2C_ASKTRADE = 71,
    S2C_OPENTRADE = 136,
    S2C_DATELIMITTIME = 138,
    S2C_FRD = 14,
    S2C_MAIL = 106,
    S2C_BATTLEFIELD = 130,
    S2C_TOWERCRASH = 96,
    S2C_BATTLEFIELD_3_2 = 88,
    S2C_LOADPROTECT = 148,
    S2C_NEWYEARTHROW = 85,
    S2C_ANS_HONOR_INFO = 103,
    S2C_ANS_BUYITEM_HONOR_FAILED = 167,
    S2C_ENCHANT_FAILED = 33,
    S2C_ANS_SKILLUP_OVERRUN = 172,
    S2C_ANS_SHOWDOWN = 158,
    S2C_REQ_SHOWDOWN = 82,
    S2C_NOTICE_2 = 163,
    S2C_ATTACK_STOP = 97,
    S2C_MSTATE_EX = 50,
    S2C_GSTATE_EX = 68,
    S2C_PET = 164,
    S2C_GETSTALLINFO = 113,
    S2C_GUILD = 65,
    S2C_ITEMLIST = 142,
    S2C_MESSAGEPK = 83,
    S2C_SKILLREDISTRIBUTE = 165,
    S2C_PKBULLETININFO = 166,
    S2C_ARMY_MEMBER_INFO = 129,
    S2C_ARMY_MEMBER_POS = 73,
    S2C_ARMYCHATTING = 111,
    S2C_DASHSKILL = 59,
    S2C_SIEGEGUNSET = 13,
    S2C_EVENT_SNOWFALL_COUNT = 112,
    S2C_SETMYTELPT = 131,
    S2C_ISLAND_SNOW = 108,
    S2C_RESTORE_EXP = 20,
    S2C_OWN_TELEPORT = 30,
    S2C_GETMYTELPT = 12,
    S2C_ANS_NPC = 197,
    S2C_MSTATEEX_ON_MASTER = 156,
    S2C_CUSTOM = 201,
    S2C_SETITEM = 180
};

enum C2S_PROTOCOL
{
	C2S_CONNECT = 9,
    C2S_ANS_CODE = 4,
    C2S_LOGIN = 8,
    C2S_SECOND_LOGIN = 10,
    C2S_LOADPLAYER = 5,
    C2S_DELPLAYER = 3,
    C2S_NEWPLAYER = 6,
    C2S_RESTOREPLAYER = 7,
    C2S_START = 14,
    C2S_GETSTALLINFO = 119,
    C2S_RESTART = 141,
    C2S_GAMEEXIT = 56,
    C2S_CHATTING = 42,
    C2S_MOVE_ON = 95,
    C2S_MOVE_END = 147,
    C2S_REST = 49,
    C2S_SKILL = 118,
    C2S_UPDATEPROPERTY = 160,
    C2S_TELEPORT = 122,
    C2S_ASKNPC = 166,
    C2S_TRASHITEM = 98,
    C2S_USEITEM = 65,
    C2S_PUTONITEM = 101,
    C2S_PUTOFFITEM = 58,
    C2S_PLAYER_ANIMATION = 47,
    C2S_SHORTCUT = 136,
    C2S_ASKPARTY = 91,
    C2S_ANS_ASKPARTY = 18,
    C2S_LEAVEPARTY = 89,
    C2S_EXILEPARTY = 171,
    C2S_REVIVAL = 72,
    C2S_ATTACK = 17,
    C2S_TARGET = 178,
    C2S_ALIVE = 2,
    C2S_LEARNSKILL = 62,
    C2S_SKILLUP = 174,
    C2S_PRESKILL = 82,
    C2S_PICKUPITEM = 175,
    C2S_DROPITEM = 108,
    C2S_SAVEREVIVALPT = 74,
    C2S_QUEST = 79,
    C2S_GIVEUP_QUEST = 102,
    C2S_TRANSFORM = 139,
    C2S_TRANSFORMSKILL = 77,
    C2S_BUYITEM = 121,
    C2S_SELLITEM = 126,
    C2S_ASKPVP = 43,
    C2S_ANS_ASKPVP = 31,
    C2S_BLESS = 81,
    C2S_INITSTAT = 131,
    C2S_ASKTRADE = 112,
    C2S_ANS_ASKTRADE = 130,
    C2S_TRADEITEM = 68,
    C2S_SHOWOFFITEM = 142,
    C2S_CANCELTRADE = 87,
    C2S_NPC = 25,
    C2S_SETSTALLINFO = 54,
    C2S_STORAGEINFO = 85,
    C2S_PUTINSTORAGE = 37,
    C2S_PUTOUTSTORAGE = 125,
    C2S_SWITCHSTALL = 33,
    C2S_ITEMLIST = 51,
    C2S_MLM = 52,
    C2S_GETITEMMONSTER = 158,
    C2S_CANCELITEMMONSTER = 23,
    C2S_ANS_REVIVALSKILL = 173,
    C2S_COOKING = 59,
    C2S_SETMYTELPT = 151,
    C2S_FRD = 165,
    C2S_MAIL = 57,
    C2S_GUILD = 70,
    C2S_BUYITEMATSTALL = 154,
    C2S_ACTION = 22,
    C2S_PKBULLETININFO = 44,
    C2S_PKSTATUS = 128,
    C2S_ENCHANTITEM = 26,
    C2S_UPGRADEITEM = 34,
    C2S_SKILLREDISTRIBUTE = 127,
    C2S_UPPER_CHANCE_STONE = 104,
    C2S_MIXING = 148,
    C2S_CHANGEPLAYERNAME = 94,
    C2S_CHANGEGUILDNAME = 162,
    C2S_DANJI = 156,
    C2S_QUESTJOB_END = 40,
    C2S_SIEGEGUN = 161,
    C2S_SIEGEGUNSET = 28,
    C2S_SIEGEGUNCONTROL = 164,
    C2S_FIREBALL = 117,
    C2S_HONOR_INFO = 168,
    C2S_KALSHOP_INIT = 184,
    C2S_DICE = 191
};

enum S2D_PROTOCOL
{
	// Account Packets
	S2D_LOGIN,
	S2D_CREATE_SECONDARY,
	S2D_CHANGE_SECONDARY,
	S2D_SECONDARY_LOGIN,
	S2D_DISCONNECT,
	S2D_DELPLAYER,
	S2D_NEWPLAYER,
	S2D_LOADPLAYER,
	S2D_RESTOREPLAYER,
	S2D_RESTART,

	// Non-account packets
	S2D_UPDATEPROPERTY,
	S2D_MAX_IID,
	S2D_INSERTITEM,
	S2D_UPDATEITEMNUM,
	S2D_SAVEALLPROPERTY,
	S2D_TRASHITEM,
	//S2D_PUTONITEM,
	//S2D_PUTOFFITEM,
	S2D_UPDATEITEMINFO,
	S2D_SHORTCUT,
    S2D_SKILLUP,
    S2D_INSERTSKILL
};

enum D2S_PROTOCOL
{
	D2S_LOGIN,
	D2S_SEC_LOGIN,
	D2S_PLAYER_INFO,
	D2S_DELPLAYERINFO,
	D2S_ANS_NEWPLAYER,
	D2S_LOADPLAYER,
	D2S_MAX_IID,
	D2S_LOADITEMS,
	D2S_SHORTCUT,
    D2S_SKILLINFO,

	D2S_AUTHORIZED,
	D2S_UPDATEITEMIID,
};