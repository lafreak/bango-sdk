#pragma once

#include <bango/network/writable.h>
#include <bango/network/authorizable.h>

#include <inix.h>

class User : public bango::network::writable, public bango::network::authorizable
{
public:
    constexpr static int CAN_REQUEST_PRIMARY    = (1 << 0);
    constexpr static int CAN_REQUEST_SECONDARY  = (1 << 1);

    constexpr static int LOBBY      = (1 << 2);
    constexpr static int LOADING    = (1 << 3);
    constexpr static int INGAME     = (1 << 4);

    constexpr static int AUTHORIZED = (1 << 5);

    User(const bango::network::taco_client_t& client) : bango::network::writable(client)
    {
        // BUG: Not thread safe.
        static unsigned int g_max_uid=0;
        m_credentials.UID = g_max_uid++;
    }

    struct CREDENTIALS 
    {
        std::int32_t    AID; // AccountID   - Player DB Table Index
        std::uint32_t   UID; // UserID      - DB/Game Server Unique User Identifier
    } m_credentials;

    unsigned int        GetUID()        const { return m_credentials.UID; }
    int                 GetAID()        const { return m_credentials.AID; }
    const CREDENTIALS&  GetCredentials()const { return m_credentials; }

    void            SetAID(int value)   { m_credentials.AID = value; }

    // Network I/O Endpoints
    void OnConnect          (bango::network::packet& p);
    void OnCodeAnswer       (bango::network::packet& p);
    void OnLogin            (bango::network::packet& p);
    void OnSecondaryLogin   (bango::network::packet& p);
    void OnNewPlayer        (bango::network::packet& p);
    void OnDeletePlayer     (bango::network::packet& p);
    void OnRestorePlayer    (bango::network::packet& p);
    void OnLoadPlayer       (bango::network::packet& p);
};