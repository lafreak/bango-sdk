#pragma once

#include <bango/network/packet.h>

class DBListener
{
public:
    static void OnLogin(bango::network::packet&);
    static void OnAuthorized(bango::network::packet&);
    static void OnSecondaryLogin(bango::network::packet&);
    static void OnPlayerInfo(bango::network::packet&);
    static void OnDeletePlayerInfo(bango::network::packet&);
    static void OnNewPlayerAnswer(bango::network::packet&);
    static void OnLoadPlayer(bango::network::packet&);
    static void OnLoadItems(bango::network::packet&);
    static void OnLoadSkills(bango::network::packet&);
    static void OnUpdateItemIID(bango::network::packet&);
};