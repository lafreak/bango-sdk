#pragma once

#include <list>
#include <functional>
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <map>

#include  <iostream>

//! Limits amount of elements in container.
//! When this amount is exceeded quad division occurs.
//! Affects performance!
#define QUADTREE_MAX_NODES 32 // http://prntscr.com/jk95a7

#define DUPLICATES_SAFE

namespace bango { namespace space {

    //! Position on 2D space.
    struct point { int x, y; };
    //! Rectangle on 2D space represented by 2 corner points - bottom left & top right.
    struct square { 
        point bottom_left; int width;
        
        int left() const    { return bottom_left.x; }
        int right() const   { return bottom_left.x+width; }
        int bottom() const  { return bottom_left.y; }
        int top() const     { return bottom_left.y+width; }
    };

    //! Base entity managed by Quadtree represented by single point in space. 
    //! Knows how to calculate distance between self and objects in 2D space.
    struct quad_entity
    {
        int m_x, m_y;
        unsigned char m_type;
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
    //! This structure manages group of objects for each smallest subdivided part of space.
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

#ifdef DUPLICATES_SAFE
    private:
        std::map<std::pair<int,int>,size_t> m_distinct_sizes;
    public:
        void _insert(const quad_entity* entity)
        {
            m_distinct_sizes[std::make_pair(entity->m_x, entity->m_y)]++;
            insert(entity);
        }
        void _remove(const quad_entity* entity)
        {
            auto it = m_distinct_sizes.find(std::make_pair(entity->m_x, entity->m_y));
            if (it == m_distinct_sizes.end())
                throw std::runtime_error("no single entity exist on this position");
            if (--it->second == 0) // BUG: is second a reference?
                m_distinct_sizes.erase(it);
            remove(entity);
        }
        void _merge(const T* container)
        {
            container->for_each([&](const quad_entity* entity) {
                m_distinct_sizes[std::make_pair(entity->m_x, entity->m_y)]++;
            });
            merge(container);
        }
        size_t _total_memory() const { return sizeof(m_distinct_sizes)+m_distinct_sizes.size()*(sizeof(std::pair<int,int>)+sizeof(size_t))+total_memory(); }
        size_t distinct_size() const { return m_distinct_sizes.size(); }
#endif
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

        const size_t m_max_container_entities;

    public:
        quad(square boundary, const size_t max_container_entities=QUADTREE_MAX_NODES, quad* parent=nullptr)
            : m_max_container_entities(max_container_entities)
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

#ifdef DUPLICATES_SAFE
        size_t distinct_size() const {
            if (!is_leaf()) {
                return m_top_left->distinct_size()+m_top_right->distinct_size()+m_bottom_left->distinct_size()+m_bottom_right->distinct_size();
            } else {
                return m_container->distinct_size();
            }
        }
#endif

        const quad* root() const {
            if (is_root())
                return this;
            return m_parent->root();
        }

        void insert(const quad_entity*);
        const quad_entity* search(point p) const;
        void query(point, int, const std::function<void(const T*)>&);
        void remove(const quad_entity*);
        void merge();
        bool is_leaf() const { return m_container != nullptr; }
        bool is_root() const { return m_parent == nullptr; }
        bool in_boundary(point p) const {
            return 
                p.x >= m_boundary.left() &&
                p.x <= m_boundary.right() && // [x, y]? should be [x,y)
                p.y >= m_boundary.bottom() &&
                p.y <= m_boundary.top();
        }
        bool in_boundary(square b) const {
            return
                b.left()    <= m_boundary.right() &&
                b.right()   >= m_boundary.left() &&
                b.top()     >= m_boundary.bottom() &&
                b.bottom()  <= m_boundary.top();
        }
        bool in_boundary(point p, int radius) const {
            // TODO: Circle->Square is bad idea, fix?
            return in_boundary({{p.x-radius, p.y-radius}, radius*2});
        }
        bool in_boundary(const quad_entity* p) const { 
            return in_boundary(point{p->m_x, p->m_y});
        }
        bool can_subdivide() const { 
            return m_boundary.right()-m_boundary.left() > 1;
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
            throw std::out_of_range("not in boundary");

        if (!is_leaf())
            return inner(entity)->insert(entity);

        // do not need to subdivide
#ifdef DUPLICATES_SAFE
        if (m_container->distinct_size() < m_max_container_entities)//QUADTREE_MAX_NODES) // constant
            return m_container->_insert(entity);
#else
        if (m_container->size() < m_max_container_entities)//QUADTREE_MAX_NODES) // constant
            return m_container->insert(entity);
#endif

        if (!can_subdivide())
#ifdef DUPLICATES_SAFE
            return m_container->_insert(entity);
#else
            throw std::runtime_error("cannot subdivide anymore");
#endif

        // subdivision
        // TODO: Fix overlapping [0, 6], [6, 12] -> [0, 5], [6, 11]?
        m_top_left = new quad({
            point {
                m_boundary.left(),
                (m_boundary.top()-m_boundary.bottom())/2+m_boundary.bottom()
            },
            m_boundary.width/2
        }, m_max_container_entities, this);
        m_top_right = new quad({
            point {
                (m_boundary.right()-m_boundary.left())/2+m_boundary.left(),
                (m_boundary.top()-m_boundary.bottom())/2+m_boundary.bottom()
            }, 
            m_boundary.width/2, 
        }, m_max_container_entities, this);
        m_bottom_left = new quad({
            m_boundary.bottom_left, 
            m_boundary.width/2
        }, m_max_container_entities, this);
        m_bottom_right = new quad({
            point {
                (m_boundary.right()-m_boundary.left())/2+m_boundary.left(),
                m_boundary.bottom()
            }, 
            m_boundary.width/2
        }, m_max_container_entities, this);

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
            return inner(entity)->remove(entity);

#ifdef DUPLICATES_SAFE
        m_container->_remove(entity);
#else
        m_container->remove(entity);
#endif
        
        if (!is_root())
            m_parent->merge(); // try to merge
    }

    template<class T>
    void quad<T>::merge()
    {
        if (is_leaf())
            throw std::logic_error("merge quad is leaf");
        // Brother doesnt need to be leaf!
        //if (!m_top_left->is_leaf())
        //    throw std::logic_error("merge quad child is not leaf");
#ifdef DUPLICATES_SAFE
        if (distinct_size() > m_max_container_entities)//QUADTREE_MAX_NODES)
#else
        if (size() > m_max_container_entities)//QUADTREE_MAX_NODES)
#endif
            return;

        m_container = new T;
#ifdef DUPLICATES_SAFE
        m_container->_merge(m_top_left       ->m_container);
        m_container->_merge(m_top_right      ->m_container);
        m_container->_merge(m_bottom_left    ->m_container);
        m_container->_merge(m_bottom_right   ->m_container);
#else
        m_container->merge(m_top_left       ->m_container);
        m_container->merge(m_top_right      ->m_container);
        m_container->merge(m_bottom_left    ->m_container);
        m_container->merge(m_bottom_right   ->m_container);
#endif
        
        
        delete m_top_left;
        delete m_top_right;
        delete m_bottom_left;
        delete m_bottom_right;

        m_top_left=m_top_right=m_bottom_left=m_bottom_right=nullptr;

        if (!is_root())
            m_parent->merge();
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
                m_boundary.left(),
                m_boundary.bottom(),
                m_boundary.right(),
                m_boundary.top(),
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
                sizeof(m_top_left)+sizeof(m_top_right)+sizeof(m_bottom_left)+sizeof(m_bottom_right)+
                sizeof(m_max_container_entities);

        if (!is_leaf())
            return result+m_top_left->total_memory()+m_top_right->total_memory()+m_bottom_left->total_memory()+m_bottom_right->total_memory();

#ifdef DUPLICATES_SAFE
        return result+m_container->_total_memory();   
#else
        return result+m_container->total_memory();   
#endif
    }

}}