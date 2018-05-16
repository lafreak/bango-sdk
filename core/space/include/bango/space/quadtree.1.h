#pragma once

#include <algorithm>

#include <bango/space/map.h>

#define QUADTREE_MAX_NODES 2

namespace bango { namespace space {

    struct point { int x, y; };
    struct square { point bottom_left, top_right; };

    class quad
    {
        square m_boundary;
        tile* m_tile;

        quad* m_parent          = nullptr;

        quad* m_top_left        = nullptr;
        quad* m_top_right       = nullptr;
        quad* m_bottom_left     = nullptr;
        quad* m_bottom_right    = nullptr;

    public:
        quad(square boundary, quad* parent=nullptr)
        {
            m_boundary = boundary; // so much copy
            m_tile = new tile;
            m_parent = parent;
        }

        ~quad() 
        { 
            if (m_tile)         delete m_tile; 
            if (m_top_left)     delete m_top_left;
            if (m_top_right)    delete m_top_right;
            if (m_bottom_left)  delete m_bottom_left;
            if (m_bottom_right) delete m_bottom_right;
        }

        void dump() const;
        long long total_memory() const;
        size_t size() const {
            if (!is_leaf()) {
                return m_top_left->size()+m_top_right->size()+m_bottom_left->size()+m_bottom_right->size();
            } else {
                return m_tile->size();
            }
        }

        void insert(static_entity*);
        static_entity* search(point p) const;
        void search(square, std::list<static_entity*>&) const;
        void search(point, int, std::list<static_entity*>&) const;
        void remove(static_entity*);
        void merge();
        bool is_leaf() const { return m_tile != nullptr; }
        bool is_root() const { return m_parent == nullptr; }
        bool in_boundary(point p) const {
            return 
                p.x >= m_boundary.bottom_left.x &&
                p.x <= m_boundary.top_right.x && // [x, y]? should be [x,y)
                p.y >= m_boundary.bottom_left.y &&
                p.y <= m_boundary.top_right.y;
        }
        bool in_boundary(square b) const {
            return
                b.bottom_left.x <= m_boundary.top_right.x &&
                b.top_right.x >= m_boundary.bottom_left.x &&
                b.top_right.y >= m_boundary.bottom_left.y &&
                b.bottom_left.y <= m_boundary.top_right.y;
        }
        bool in_boundary(point p, int radius) const {
            return
                p.x-radius <= m_boundary.top_right.x &&
                p.x+radius >= m_boundary.bottom_left.x &&
                p.y+radius >= m_boundary.bottom_left.y &&
                p.y-radius <= m_boundary.top_right.y;
        }
        bool in_boundary(entity* p) const { 
            return in_boundary(point{p->m_x, p->m_y});
        }
        bool can_subdivide() const { 
            return m_boundary.top_right.x-m_boundary.bottom_left.x > 1;
        }
        quad* inner(point p) const {
            if (m_top_left->in_boundary(p))     return m_top_left;
            if (m_top_right->in_boundary(p))    return m_top_right;
            if (m_bottom_left->in_boundary(p))  return m_bottom_left;
            if (m_bottom_right->in_boundary(p)) return m_bottom_right;
        }
        quad* inner(entity* e) const {
            return inner(point{e->m_x, e->m_y});
        }
    };

    void quad::insert(static_entity* entity)
    {
        if (!in_boundary(entity))
            throw std::runtime_error("not in boundary");

        if (!is_leaf())
        {
            inner(entity)->insert(entity);
            return;
        }

        // do not need to subdivide
        if (m_tile->size() < QUADTREE_MAX_NODES) // constant
        {
            m_tile->m_statics.push_back(entity);
            return;
        }

        // BUG: duplicate 2D position cause endless recursion
        if (!can_subdivide())
            throw std::runtime_error("cannot subdivide anymore");

        // subdivision
        // TODO: Fix overlapping [0, 6], [6, 12] -> [0, 5], [6, 11]?
        m_top_left = new quad({
            point {
                m_boundary.bottom_left.x,
                (m_boundary.top_right.y-m_boundary.bottom_left.y)/2+m_boundary.bottom_left.y
            }, 
            point {
                (m_boundary.top_right.x-m_boundary.bottom_left.x)/2+m_boundary.bottom_left.x,
                m_boundary.top_right.y
            }
        }, this);
        m_top_right = new quad({
            point {
                (m_boundary.top_right.x-m_boundary.bottom_left.x)/2+m_boundary.bottom_left.x,
                (m_boundary.top_right.y-m_boundary.bottom_left.y)/2+m_boundary.bottom_left.y
            }, 
            m_boundary.top_right
        }, this);
        m_bottom_left = new quad({
            m_boundary.bottom_left, 
            point {
                (m_boundary.top_right.x-m_boundary.bottom_left.x)/2+m_boundary.bottom_left.x,
                (m_boundary.top_right.y-m_boundary.bottom_left.y)/2+m_boundary.bottom_left.y
            }
        }, this);
        m_bottom_right = new quad({
            point {
                (m_boundary.top_right.x-m_boundary.bottom_left.x)/2+m_boundary.bottom_left.x,
                m_boundary.bottom_left.y
            }, 
            point {
                m_boundary.top_right.x,
                (m_boundary.top_right.y-m_boundary.bottom_left.y)/2+m_boundary.bottom_left.y
            }
        }, this);

        for (auto& en : m_tile->m_statics)
            inner(en)->insert(en);

        inner(entity)->insert(entity);

        delete m_tile;
        m_tile = nullptr;
    }

    void quad::remove(static_entity* entity)
    {
        if (!in_boundary(entity))
            return;

        if (!is_leaf())
        {
            inner(entity)->remove(entity);
            return;
        }

        m_tile->m_statics.remove(entity);
        
        if (!is_root())
            m_parent->merge();
    }

    void quad::merge()
    {
        if (is_leaf())
            throw std::runtime_error("merge logic error");
        if (!m_top_left->is_leaf())
            throw std::runtime_error("merge logic error");
        if (size() > QUADTREE_MAX_NODES)
            return;

        m_tile = new tile;
        m_tile->m_statics.merge(m_top_left      ->m_tile->m_statics);
        m_tile->m_statics.merge(m_top_right     ->m_tile->m_statics);
        m_tile->m_statics.merge(m_bottom_left   ->m_tile->m_statics);
        m_tile->m_statics.merge(m_bottom_right  ->m_tile->m_statics);
        
        delete m_top_left;
        delete m_top_right;
        delete m_bottom_left;
        delete m_bottom_right;

        m_top_left=m_top_right=m_bottom_left=m_bottom_right=nullptr;

        if (!is_root())
            m_parent->merge();
    }

    static_entity* quad::search(point p) const
    {
        if (!in_boundary(p))
            return nullptr;
            
        if (!is_leaf())
            return inner(p)->search(p);

        auto it = std::find_if(std::begin(m_tile->m_statics), std::end(m_tile->m_statics),
                        [&](const static_entity* e) { return e->m_x == p.x && e->m_y == p.y; });

        if (m_tile->m_statics.end() == it)
            return nullptr;
        return *it;
    }

    void quad::search(square b, std::list<static_entity*>& entities) const
    {
        if (!in_boundary(b))
            return;

        if (!is_leaf())
        {
            m_top_left      ->search(b, entities);
            m_top_right     ->search(b, entities);
            m_bottom_left   ->search(b, entities);
            m_bottom_right  ->search(b, entities);
            return;
        }

        for (auto& e : m_tile->m_statics)
        {
            if (e->m_x >= b.bottom_left.x &&
                e->m_x <= b.top_right.x &&
                e->m_y >= b.bottom_left.y &&
                e->m_y <= b.top_right.y)
            entities.push_back(e);
        }
    }

    void quad::search(point p, int radius, std::list<static_entity*>& entities) const
    {
        if (radius <= 0)
        {
            auto result = search(p);
            if (result) entities.push_back(result);
            return;
        }

        if (!in_boundary(p, radius))
            return;

        if (!is_leaf())
        {
            m_top_left      ->search(p, radius, entities);
            m_top_right     ->search(p, radius, entities);
            m_bottom_left   ->search(p, radius, entities);
            m_bottom_right  ->search(p, radius, entities);
            return;
        }

        for (auto& e : m_tile->m_statics)
        {
            if (e->distance(p.x, p.y) <= radius)
                entities.push_back(e);
        }
    }

    void quad::dump() const
    {
        if (!is_leaf())
        {
            m_top_left      ->dump();
            m_top_right     ->dump();
            m_bottom_left   ->dump();
            m_bottom_right  ->dump();
            return;
        }

        if (m_tile->size() > 0)
            printf("BL[%d][%d]TR[%d][%d] - size(%d)\n", 
                m_boundary.bottom_left.x,
                m_boundary.bottom_left.y,
                m_boundary.top_right.x,
                m_boundary.top_right.y,
                (int) m_tile->size());
    }

    // 16+8+
    // 8+
    // 8+8+8+8+
    // 24+24+24+(2*8)
    long long quad::total_memory() const 
    {
        long long result =
                sizeof(m_boundary)+sizeof(m_tile)+
                sizeof(m_parent)+
                sizeof(m_top_left)+sizeof(m_top_right)+sizeof(m_bottom_left)+sizeof(m_bottom_right);

        if (!is_leaf())
            return result+m_top_left->total_memory()+m_top_right->total_memory()+m_bottom_left->total_memory()+m_bottom_right->total_memory();

        return result+sizeof(m_tile->m_dynamics)+sizeof(m_tile->m_notifables)+sizeof(m_tile->m_statics)+m_tile->m_statics.size() * sizeof(static_entity*);      
    }

}}