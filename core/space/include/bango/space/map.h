#pragma once

#include <list>
#include <functional>
#include <stdio.h>
#include <string>
#include <math.h>

namespace bango { namespace space {

    // base object
    struct entity
    {
        explicit entity(int x, int y, std::string& name) : m_x(x), m_y(y), m_name(name) {}
        explicit entity(int x, int y, std::string&& name) : m_x(x), m_y(y), m_name(name) {}
        
        int m_x;
        int m_y;
        std::string m_name;

        constexpr static char STATIC   =0;
        constexpr static char DYNAMIC  =1;
        constexpr static char NOTIFABLE=2;

        virtual char type() const = 0;

        int distance(const entity* e) const { return distance(e->m_x, e->m_y); }
        int distance(int x, int y) const { return sqrt(pow(m_x - x, 2) + pow(m_y - y, 2)); }
    };

    // immovable object
    class static_entity : public entity
    {
        using entity::entity;

        virtual char type() const override { return entity::STATIC; }
    };

    // movable object
    class dynamic_entity : public entity
    {
        using entity::entity;

        virtual char type() const override { return entity::DYNAMIC; }
    };

    // in practice - human actor
    class notifable_entity : public entity
    {
        using entity::entity;

        virtual char type() const override { return entity::NOTIFABLE; }
    };

    struct tile
    {
        // TODO: Insert/Remove is O(n) -> optimize
        std::list<static_entity*> m_statics;
        std::list<dynamic_entity*> m_dynamics;
        std::list<notifable_entity*> m_notifables;

        size_t size() const { return m_statics.size() + m_dynamics.size() + m_notifables.size(); }
    };

    class map
    {
        /*
        
        TN -    tile_length
                height
                width

        |0|2|4| <- TN         TN  ^
        |1|0|3|               TN  |     |TN|=Height/Width
        |0|2|5|               TN  |
        |1|3|6|               TN  v

        */

        const int m_width;
        const int m_height;
        const int m_tile_len;
        const int m_sight;

        tile** m_tiles;

        std::function<void(notifable_entity*, entity*)> m_on_appear = [](notifable_entity* ne, entity* e) {
            printf("untracked onappear: %s-%s[%d]\n", ne->m_name.c_str(), e->m_name.c_str(), ne->distance(e));
        };

        std::function<void(notifable_entity*, entity*)> m_on_disappear = [](notifable_entity* ne, entity* e) {
            printf("untracked ondisappear: %s-%s[%d]\n", ne->m_name.c_str(), e->m_name.c_str(), ne->distance(e));
        };

        std::function<void(notifable_entity*, entity*, int, int)> m_on_move = [](notifable_entity* ne, entity* e, int new_x, int new_y) {
            printf("untracked onmove: %s-%s[%d;%d->%d;%d]\n", ne->m_name.c_str(), e->m_name.c_str(), e->m_x, e->m_y, new_x, new_y);
        };

    public:
        map(const int width, const int height, const int tile_len, const int sight):
            m_width(width), m_height(height), m_tile_len(tile_len), m_sight(sight)
        {
            m_tiles = new tile*[m_width * m_height];
            for (int i = 0; i < m_width * m_height; i++)
                m_tiles[i] = new tile;
        }

        ~map()
        {
            for (int i = 0; i < m_width * m_height; i++)
                delete m_tiles[i];
            delete[] m_tiles;
        }

        void on_appear(std::function<void(notifable_entity*, entity*)> callback)
        {
            m_on_appear = callback;
        }

        void on_disappear(std::function<void(notifable_entity*, entity*)> callback)
        {
            m_on_disappear = callback;
        }

        void on_move(std::function<void(notifable_entity*, entity*, int, int)> callback)
        {
            m_on_move = callback;
        }

        tile* at(entity* e, int x_offset=0, int y_offset=0) const
        {
            return at(e->m_x / m_tile_len + x_offset, e->m_y / m_tile_len + y_offset);
        }

        tile* at(int i, int j) const
        {
            return m_tiles[i * m_height + j];
        }

        bool is_nearby_tile(entity* e, int i, int j) const { return is_nearby_tile(e->m_x, e->m_y, i, j); }
        bool is_nearby_tile(int x, int y, int i, int j) const
        {
            int anchor_x = i * m_tile_len;
            int anchor_y = j * m_tile_len;

            if (x + m_sight < anchor_x)
                return false;
            if (x - m_sight >= anchor_x + m_tile_len)
                return false;
            if (y + m_sight < anchor_y)
                return false;
            if (y - m_sight >= anchor_y + m_tile_len)
                return false;
            return true;
        }

        void for_each_in_sight(entity* e, std::function<void(entity*)> callback) const { for_each_in_sight(e->m_x, e->m_y, callback); }
        void for_each_in_sight(int x, int y, std::function<void(entity*)> callback) const
        {
            // O(n^2) where N=MapLength/TileLength
            // TODO: Optimize
            for (int i = 0; i < m_width; i++)
            {
                for (int j = 0; j < m_height; j++)
                {
                    if (is_nearby_tile(x, y, i, j))
                    {
                        for (const auto& ne : at(i, j)->m_statics)
                        {
                            // TODO: Add margin
                            if (ne->distance(x, y) <= m_sight)
                                callback(ne);
                        }

                        for (const auto& ne : at(i, j)->m_dynamics)
                        {
                            // TODO: Add margin
                            if (ne->distance(x, y) <= m_sight)
                                callback(ne);
                        }

                        for (const auto& ne : at(i, j)->m_notifables)
                        {
                            // TODO: Add margin
                            if (ne->distance(x, y) <= m_sight)
                                callback(ne);
                        }
                    }
                }
            }
        }

        void for_each_notifable_in_sight(entity* e, std::function<void(notifable_entity*)> callback) const { for_each_notifable_in_sight(e->m_x, e->m_y, callback); }
        void for_each_notifable_in_sight(int x, int y, std::function<void(notifable_entity*)> callback) const
        {
            // O(n^2) where N=MapLength/TileLength
            // TODO: Optimize
            for (int i = 0; i < m_width; i++)
            {
                for (int j = 0; j < m_height; j++)
                {
                    if (is_nearby_tile(x, y, i, j))
                    {
                        for (const auto& ne : at(i, j)->m_notifables)
                        {
                            // TODO: Add margin
                            if (ne->distance(x, y) <= m_sight)
                                callback(ne);
                        }
                    }
                }
            }
        }

        void insert(static_entity* e)       //{ at(e)->m_statics.push_back(e); }
        {
            for_each_notifable_in_sight(e, [&](notifable_entity* s) {
                m_on_appear(s, e);
            });

            at(e)->m_statics.push_back(e);
        }

        void insert(dynamic_entity* e)      //{ at(e)->m_dynamics.push_back(e); }
        {
            for_each_notifable_in_sight(e, [&](notifable_entity* s) {
                m_on_appear(s, e);
            });

            at(e)->m_dynamics.push_back(e);
        }

        void insert(notifable_entity* e)    //{ at(e)->m_notifables.push_back(e); }
        {
            for_each_in_sight(e, [&](entity* s) {
                m_on_appear(e, s);
                
                if (s->type() == entity::NOTIFABLE)
                    m_on_appear((notifable_entity*)s, e);
            });

            at(e)->m_notifables.push_back(e);
        }

        void remove(static_entity* e)       //{ at(e)->m_statics.remove(e); }
        {
            at(e)->m_statics.remove(e);

            for_each_notifable_in_sight(e, [&](notifable_entity* s) {
                m_on_disappear(s, e);
            });
        }

        void remove(dynamic_entity* e)      //{ at(e)->m_dynamics.remove(e); }
        {
            at(e)->m_dynamics.remove(e);

            for_each_notifable_in_sight(e, [&](notifable_entity* s) {
                m_on_disappear(s, e);
            });
        }

        void remove(notifable_entity* e)    //{ at(e)->m_notifables.remove(e); }
        {
            at(e)->m_notifables.remove(e);

            for_each_notifable_in_sight(e, [&](notifable_entity* s) {
                m_on_disappear(s, e);
            });
        }

        bool tile_changed(entity* e, int new_x, int new_y) const
        {
            return !(e->m_x / m_tile_len == new_x / m_tile_len && e->m_y / m_tile_len == new_y / m_tile_len);
        }

        void move(dynamic_entity* e, int new_x, int new_y)
        {
            // Events
            for_each_notifable_in_sight(e, [&](notifable_entity* s) {
                if (s->distance(new_x, new_y) > m_sight)
                    m_on_disappear(s, e);

                if (s->distance(new_x, new_y) <= m_sight)
                    m_on_move(s, e, new_x, new_y);
            });

            for_each_notifable_in_sight(new_x, new_y, [&](notifable_entity* s) {
                if (s->distance(e) > m_sight)
                    m_on_appear(s, e);
            });

            // Actual move
            auto changed = tile_changed(e, new_x, new_y);

            if (changed)
                at(e)->m_dynamics.remove(e);

            e->m_x = new_x;
            e->m_y = new_y;

            if (changed)
                at(e)->m_dynamics.push_back(e);
        }

        void move(notifable_entity* e, int new_x, int new_y)
        {
            // Events
            for_each_in_sight(e, [&](entity* s) {
                if (e == s) return;

                if (s->distance(new_x, new_y) > m_sight)
                {
                    m_on_disappear(e, s);
                    if (s->type() == entity::NOTIFABLE)
                        m_on_disappear((notifable_entity*)s, e);
                }

                if (s->type() == entity::NOTIFABLE && s->distance(new_x, new_y) <= m_sight)
                    m_on_move((notifable_entity*)s, e, new_x, new_y);
            });

            for_each_in_sight(new_x, new_y, [&](entity* s) {
                if (e == s) return;

                if (s->distance(e) > m_sight)
                {
                    m_on_appear(e, s);

                    if (s->type() == entity::NOTIFABLE)
                        m_on_appear((notifable_entity*)s, e);
                }
            });

            // Actual move
            auto changed = tile_changed(e, new_x, new_y);

            if (changed)
                at(e)->m_notifables.remove(e);

            e->m_x = new_x;
            e->m_y = new_y;

            if (changed)
                at(e)->m_notifables.push_back(e);
        }

        void dump() const
        {
            for (int i = 0; i < m_width; i++)
            {
                printf("|");
                for (int j = 0; j < m_height; j++)
                    printf("%d|", (int) at(i, j)->size());
                printf("\n");
            }
        }
    };

}}