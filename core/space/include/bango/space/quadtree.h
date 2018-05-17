#pragma once

#include <list>
#include <functional>
#include <stdio.h>
#include <math.h>
#include <algorithm>

//! Limits amount of elements in container.
//! When this amount is exceeded quad division occurs.
//! Affects performance!
#define QUADTREE_MAX_NODES 8

namespace bango { namespace space {

    //! Position on 2D space.
    struct point { int x, y; };
    //! Rectangle on 2D space represented by 2 corner points - bottom left & top right.
    struct square { point bottom_left, top_right; };

    //! Base entity managed by Quadtree represented by single point in space. 
    //! Knows how to calculate distance between self and objects in 2D space.
    struct quad_entity
    {
        int m_x, m_y;
        //! \return Distance between self and 2D point given as parameter.
        int distance(point p) const { return distance(p.x, p.y); }
        //! \param x Coordinate X
        //! \param y Coordinate Y
        //! \return Distance between self and pair of numbers representing 2D point given as paramter.
        int distance(int x, int y) const { return (int) sqrt(pow(m_x-x, 2)+pow(m_y-y, 2)); }
        //! \return Distance between self and entity given as parameter.
        int distance(const quad_entity* qe) const { return distance(qe->m_x, qe->m_y); }
    };

    //! Container interface for quad leafs. 
    //! This structure manages group of objects for each smallest sugdivided part of space.
    template<class T>
    struct quad_entity_container
    {
        //! Should implement entity insertion operation.
        virtual void                insert          (const quad_entity* entity)                             =0;
        //! Should implement entity removal operation.
        virtual void                remove          (const quad_entity* entity)                             =0;
        //! Should move all entities from container given as parameter to self.
        virtual void                merge           (const T* container)                                    =0;
        //! Should calculate number of all elements in this container.
        virtual size_t              size            () const                                                =0;
        //! Should calculate total memory reserved by this container.
        virtual long long           total_memory    () const                                                =0;
        //! Should iterate for each entity in this container and run callback given as parameter.
        virtual void                for_each        (const std::function<void(const quad_entity*)>&&) const =0;
    };

    template<class T>
    class quad
    {
        square m_boundary;
        T* m_container;

        quad* m_parent          = nullptr;

        quad* m_top_left        = nullptr;
        quad* m_top_right       = nullptr;
        quad* m_bottom_left     = nullptr;
        quad* m_bottom_right    = nullptr;

    public:
        quad(square boundary, quad* parent=nullptr)
        {
            m_boundary = boundary; // so much copy
            m_container = new T;
            m_parent = parent;
        }

        ~quad() 
        { 
            if (m_container)    delete m_container; 
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
                return m_container->size();
            }
        }

        quad* root() const {
            if (is_root())
                return (quad*) this;
            return m_parent->root();
        }

        void insert(const quad_entity*);
        const quad_entity* search(point p) const;
        void search(square, std::list<const quad_entity*>&) const;
        void search(point, int, std::list<const quad_entity*>&) const;
        void query(point, int, const std::function<void(const T*)>&);
        void remove(const quad_entity*);
        void merge();
        bool is_leaf() const { return m_container != nullptr; }
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
        bool in_boundary(const quad_entity* p) const { 
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
        quad* inner(const quad_entity* e) const {
            return inner(point{e->m_x, e->m_y});
        }
    };

    template<class T>
    void quad<T>::insert(const quad_entity* entity)
    {
        if (!in_boundary(entity))
            throw std::runtime_error("not in boundary");

        if (!is_leaf())
        {
            inner(entity)->insert(entity);
            return;
        }

        // do not need to subdivide
        if (m_container->size() < QUADTREE_MAX_NODES) // constant
        {
            m_container->insert(entity);
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

        //!!!
        //for (auto& en : m_tile->m_statics)
        //    inner(en)->insert(en);
        m_container->for_each([&](const quad_entity* qe) {
            inner(qe)->insert(qe);
        });

        inner(entity)->insert(entity);

        delete m_container;
        m_container = nullptr;
    }

    template<class T>
    void quad<T>::remove(const quad_entity* entity)
    {
        if (!in_boundary(entity))
            return;

        if (!is_leaf())
        {
            inner(entity)->remove(entity);
            return;
        }

        m_container->remove(entity);
        
        if (!is_root())
            m_parent->merge();
    }

    template<class T>
    void quad<T>::merge()
    {
        if (is_leaf())
            throw std::runtime_error("merge logic error");
        if (!m_top_left->is_leaf())
            throw std::runtime_error("merge logic error");
        if (size() > QUADTREE_MAX_NODES)
            return;

        m_container = new T;
        m_container->merge(m_top_left       ->m_container);
        m_container->merge(m_top_right      ->m_container);
        m_container->merge(m_bottom_left    ->m_container);
        m_container->merge(m_bottom_right   ->m_container);
        
        
        delete m_top_left;
        delete m_top_right;
        delete m_bottom_left;
        delete m_bottom_right;

        m_top_left=m_top_right=m_bottom_left=m_bottom_right=nullptr;

        if (!is_root())
            m_parent->merge();
    }

    template<class T>
    const quad_entity* quad<T>::search(point p) const
    {
        if (!in_boundary(p))
            return nullptr;
            
        if (!is_leaf())
            return inner(p)->search(p);

        const quad_entity* q = nullptr;

        m_container->for_each([&](const quad_entity* qe) {
            if (qe->m_x == p.x && qe->m_y == p.y)
                q = qe;
        });

        return q;
    }

    template<class T>
    void quad<T>::search(square b, std::list<const quad_entity*>& entities) const
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

        //!!!
        // for (auto& e : m_tile->m_statics)
        // {
        //     if (e->m_x >= b.bottom_left.x &&
        //         e->m_x <= b.top_right.x &&
        //         e->m_y >= b.bottom_left.y &&
        //         e->m_y <= b.top_right.y)
        //     entities.push_back(e);
        // }
        m_container->for_each([&](const quad_entity* qe) {
            if (qe->m_x >= b.bottom_left.x &&
                qe->m_x <= b.top_right.x &&
                qe->m_y >= b.bottom_left.y &&
                qe->m_y <= b.top_right.y)
                entities.push_back(qe);
        });
    }

    template<class T>
    void quad<T>::search(point p, int radius, std::list<const quad_entity*>& entities) const
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

        //!!!
        // for (auto& e : m_tile->m_statics)
        // {
        //     if (e->distance(p.x, p.y) <= radius)
        //         entities.push_back(e);
        // }
        m_container->for_each([&](const quad_entity* qe) {
            if (qe->distance(p.x, p.y) <= radius)
                entities.push_back(qe);
        });        
    }

    template<class T>
    void quad<T>::query(point p, int radius, const std::function<void(const T*)>& callback)
    {
        if (!in_boundary(p, radius))
            return;

        if (!is_leaf())
        {
            m_top_left      ->query(p, radius, callback);
            m_top_right     ->query(p, radius, callback);
            m_bottom_left   ->query(p, radius, callback);
            m_bottom_right  ->query(p, radius, callback);
            return;
        }

        callback(m_container);
    }

    template<class T>
    void quad<T>::dump() const
    {
        if (!is_leaf())
        {
            m_top_left      ->dump();
            m_top_right     ->dump();
            m_bottom_left   ->dump();
            m_bottom_right  ->dump();
            return;
        }

        if (m_container->size() > 0)
            printf("(%d;%d)->(%d;%d) - size(%d)\n", 
                m_boundary.bottom_left.x,
                m_boundary.bottom_left.y,
                m_boundary.top_right.x,
                m_boundary.top_right.y,
                (int) m_container->size());
    }

    // for my build:
    // m_boundary=16
    // std::list=24
    // pointers=8
    template<class T>
    long long quad<T>::total_memory() const 
    {
        long long result =
                sizeof(m_boundary)+sizeof(m_container)+
                sizeof(m_parent)+
                sizeof(m_top_left)+sizeof(m_top_right)+sizeof(m_bottom_left)+sizeof(m_bottom_right);

        if (!is_leaf())
            return result+m_top_left->total_memory()+m_top_right->total_memory()+m_bottom_left->total_memory()+m_bottom_right->total_memory();

        return result+m_container->total_memory();   
    }

}}