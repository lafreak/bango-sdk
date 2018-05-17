# Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`define `[`QUADTREE_MAX_NODES`](#quadtree_8h_1a2f110c5dada605918e987d3f740f2edb)            | Limits amount of elements in container. When this amount is exceeded quad division occurs. Affects performance!
`namespace `[`bango::space`](#namespacebango_1_1space) | 

## Members

#### `define `[`QUADTREE_MAX_NODES`](#quadtree_8h_1a2f110c5dada605918e987d3f740f2edb) 

Limits amount of elements in container. When this amount is exceeded quad division occurs. Affects performance!

# namespace `bango::space` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`class `[`bango::space::quad`](#classbango_1_1space_1_1quad) | 
`struct `[`bango::space::point`](#structbango_1_1space_1_1point) | Position on 2D space.
`struct `[`bango::space::quad_entity`](#structbango_1_1space_1_1quad__entity) | Base entity managed by Quadtree represented by single point in space. Knows how to calculate distance between self and objects in 2D space.
`struct `[`bango::space::quad_entity_container`](#structbango_1_1space_1_1quad__entity__container) | Container interface for quad leafs. This structure manages group of objects for each smallest sugdivided part of space.
`struct `[`bango::space::square`](#structbango_1_1space_1_1square) | Rectangle on 2D space represented by 2 corner points - bottom left & top right.

# class `bango::space::quad` 

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`quad`](#classbango_1_1space_1_1quad_1a3d3220cea13ab93b76e3306fbaaba19a)`(`[`square`](#structbango_1_1space_1_1square)` boundary,quad * parent)` | 
`public inline  `[`~quad`](#classbango_1_1space_1_1quad_1a0eb452c1dad4630f7081afcb6bd56180)`()` | 
`public void `[`dump`](#classbango_1_1space_1_1quad_1acaffa37c05e86811ee57ae3594f51215)`() const` | 
`public long long `[`total_memory`](#classbango_1_1space_1_1quad_1ab7d252138381a0a824474f7228742374)`() const` | 
`public inline size_t `[`size`](#classbango_1_1space_1_1quad_1a088fcc15ff39baec0fb6d38b0b90da08)`() const` | 
`public inline quad * `[`root`](#classbango_1_1space_1_1quad_1a11a8c714cb1b0478dc9dcb4e4ecb644f)`() const` | 
`public void `[`insert`](#classbango_1_1space_1_1quad_1a97beddbb0715af3c2a42f6daf73c7ec2)`(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` *)` | 
`public const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` * `[`search`](#classbango_1_1space_1_1quad_1a772aae36ef54d581ea022997413ce43a)`(`[`point`](#structbango_1_1space_1_1point)` p) const` | 
`public void `[`search`](#classbango_1_1space_1_1quad_1afa14951c3759b04b80275cfd7dcb14ec)`(`[`square`](#structbango_1_1space_1_1square)`,std::list< const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` *> &) const` | 
`public void `[`search`](#classbango_1_1space_1_1quad_1ad3c885b5fcd3a524ea938c4667423011)`(`[`point`](#structbango_1_1space_1_1point)`,int,std::list< const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` *> &) const` | 
`public void `[`query`](#classbango_1_1space_1_1quad_1a48f6419fddcec0198842e53893dc91bc)`(`[`point`](#structbango_1_1space_1_1point)`,int,const std::function< void(const T *)> &)` | 
`public void `[`remove`](#classbango_1_1space_1_1quad_1a8c3855e6ba8fd13b0c90afa3126ed19b)`(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` *)` | 
`public void `[`merge`](#classbango_1_1space_1_1quad_1aadd7f85a042e4a349655ffa388b6ab97)`()` | 
`public inline bool `[`is_leaf`](#classbango_1_1space_1_1quad_1aba7aa37c8a887700c11b7ea632165d65)`() const` | 
`public inline bool `[`is_root`](#classbango_1_1space_1_1quad_1a573f8ba1bc196f1242193ad7058a1988)`() const` | 
`public inline bool `[`in_boundary`](#classbango_1_1space_1_1quad_1af7240889a51b98aca8a987e60441a1d2)`(`[`point`](#structbango_1_1space_1_1point)` p) const` | 
`public inline bool `[`in_boundary`](#classbango_1_1space_1_1quad_1aa3a7d667fdd619f35167f49d7f29eb12)`(`[`square`](#structbango_1_1space_1_1square)` b) const` | 
`public inline bool `[`in_boundary`](#classbango_1_1space_1_1quad_1a8b06ed46704166e27b8dcf43e0e37f29)`(`[`point`](#structbango_1_1space_1_1point)` p,int radius) const` | 
`public inline bool `[`in_boundary`](#classbango_1_1space_1_1quad_1af6b7e7a285619a8f6aa6a11262a7363e)`(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` * p) const` | 
`public inline bool `[`can_subdivide`](#classbango_1_1space_1_1quad_1af9bd6f55ca5ae93a0660de138269ce16)`() const` | 
`public inline quad * `[`inner`](#classbango_1_1space_1_1quad_1a0a6cfb272a1f313cb4309883ff6b2fd4)`(`[`point`](#structbango_1_1space_1_1point)` p) const` | 
`public inline quad * `[`inner`](#classbango_1_1space_1_1quad_1a244a9deb78b40873e0eb9b8bf9acdce1)`(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` * e) const` | 

## Members

#### `public inline  `[`quad`](#classbango_1_1space_1_1quad_1a3d3220cea13ab93b76e3306fbaaba19a)`(`[`square`](#structbango_1_1space_1_1square)` boundary,quad * parent)` 

#### `public inline  `[`~quad`](#classbango_1_1space_1_1quad_1a0eb452c1dad4630f7081afcb6bd56180)`()` 

#### `public void `[`dump`](#classbango_1_1space_1_1quad_1acaffa37c05e86811ee57ae3594f51215)`() const` 

#### `public long long `[`total_memory`](#classbango_1_1space_1_1quad_1ab7d252138381a0a824474f7228742374)`() const` 

#### `public inline size_t `[`size`](#classbango_1_1space_1_1quad_1a088fcc15ff39baec0fb6d38b0b90da08)`() const` 

#### `public inline quad * `[`root`](#classbango_1_1space_1_1quad_1a11a8c714cb1b0478dc9dcb4e4ecb644f)`() const` 

#### `public void `[`insert`](#classbango_1_1space_1_1quad_1a97beddbb0715af3c2a42f6daf73c7ec2)`(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` *)` 

#### `public const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` * `[`search`](#classbango_1_1space_1_1quad_1a772aae36ef54d581ea022997413ce43a)`(`[`point`](#structbango_1_1space_1_1point)` p) const` 

#### `public void `[`search`](#classbango_1_1space_1_1quad_1afa14951c3759b04b80275cfd7dcb14ec)`(`[`square`](#structbango_1_1space_1_1square)`,std::list< const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` *> &) const` 

#### `public void `[`search`](#classbango_1_1space_1_1quad_1ad3c885b5fcd3a524ea938c4667423011)`(`[`point`](#structbango_1_1space_1_1point)`,int,std::list< const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` *> &) const` 

#### `public void `[`query`](#classbango_1_1space_1_1quad_1a48f6419fddcec0198842e53893dc91bc)`(`[`point`](#structbango_1_1space_1_1point)`,int,const std::function< void(const T *)> &)` 

#### `public void `[`remove`](#classbango_1_1space_1_1quad_1a8c3855e6ba8fd13b0c90afa3126ed19b)`(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` *)` 

#### `public void `[`merge`](#classbango_1_1space_1_1quad_1aadd7f85a042e4a349655ffa388b6ab97)`()` 

#### `public inline bool `[`is_leaf`](#classbango_1_1space_1_1quad_1aba7aa37c8a887700c11b7ea632165d65)`() const` 

#### `public inline bool `[`is_root`](#classbango_1_1space_1_1quad_1a573f8ba1bc196f1242193ad7058a1988)`() const` 

#### `public inline bool `[`in_boundary`](#classbango_1_1space_1_1quad_1af7240889a51b98aca8a987e60441a1d2)`(`[`point`](#structbango_1_1space_1_1point)` p) const` 

#### `public inline bool `[`in_boundary`](#classbango_1_1space_1_1quad_1aa3a7d667fdd619f35167f49d7f29eb12)`(`[`square`](#structbango_1_1space_1_1square)` b) const` 

#### `public inline bool `[`in_boundary`](#classbango_1_1space_1_1quad_1a8b06ed46704166e27b8dcf43e0e37f29)`(`[`point`](#structbango_1_1space_1_1point)` p,int radius) const` 

#### `public inline bool `[`in_boundary`](#classbango_1_1space_1_1quad_1af6b7e7a285619a8f6aa6a11262a7363e)`(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` * p) const` 

#### `public inline bool `[`can_subdivide`](#classbango_1_1space_1_1quad_1af9bd6f55ca5ae93a0660de138269ce16)`() const` 

#### `public inline quad * `[`inner`](#classbango_1_1space_1_1quad_1a0a6cfb272a1f313cb4309883ff6b2fd4)`(`[`point`](#structbango_1_1space_1_1point)` p) const` 

#### `public inline quad * `[`inner`](#classbango_1_1space_1_1quad_1a244a9deb78b40873e0eb9b8bf9acdce1)`(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` * e) const` 

# struct `bango::space::point` 

Position on 2D space.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public int `[`x`](#structbango_1_1space_1_1point_1a7306db53596f8f8c4aebe20fa52605bb) | 
`public int `[`y`](#structbango_1_1space_1_1point_1a9a4625aad68d1e7449af7e8be48a4349) | 

## Members

#### `public int `[`x`](#structbango_1_1space_1_1point_1a7306db53596f8f8c4aebe20fa52605bb) 

#### `public int `[`y`](#structbango_1_1space_1_1point_1a9a4625aad68d1e7449af7e8be48a4349) 

# struct `bango::space::quad_entity` 

Base entity managed by Quadtree represented by single point in space. Knows how to calculate distance between self and objects in 2D space.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public int `[`m_x`](#structbango_1_1space_1_1quad__entity_1acc2bfb8d7d954b6007eb6faba899353e) | 
`public int `[`m_y`](#structbango_1_1space_1_1quad__entity_1a72834ea1910a7935b0d31c436ce93214) | 
`public inline int `[`distance`](#structbango_1_1space_1_1quad__entity_1af4cca60fce39389dbe4e247f23535502)`(`[`point`](#structbango_1_1space_1_1point)` p) const` | #### Returns
`public inline int `[`distance`](#structbango_1_1space_1_1quad__entity_1a85750b7f2e239d8485dda580274ec91d)`(int x,int y) const` | #### Parameters
`public inline int `[`distance`](#structbango_1_1space_1_1quad__entity_1adb108e7b2ea82fb020bf673803e89c3a)`(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` * qe) const` | #### Returns

## Members

#### `public int `[`m_x`](#structbango_1_1space_1_1quad__entity_1acc2bfb8d7d954b6007eb6faba899353e) 

#### `public int `[`m_y`](#structbango_1_1space_1_1quad__entity_1a72834ea1910a7935b0d31c436ce93214) 

#### `public inline int `[`distance`](#structbango_1_1space_1_1quad__entity_1af4cca60fce39389dbe4e247f23535502)`(`[`point`](#structbango_1_1space_1_1point)` p) const` 

#### Returns
Distance between self and 2D point given as parameter.

#### `public inline int `[`distance`](#structbango_1_1space_1_1quad__entity_1a85750b7f2e239d8485dda580274ec91d)`(int x,int y) const` 

#### Parameters
* `x` Coordinate X 

* `y` Coordinate Y 

#### Returns
Distance between self and pair of numbers representing 2D point given as paramter.

#### `public inline int `[`distance`](#structbango_1_1space_1_1quad__entity_1adb108e7b2ea82fb020bf673803e89c3a)`(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` * qe) const` 

#### Returns
Distance between self and entity given as parameter.

# struct `bango::space::quad_entity_container` 

Container interface for quad leafs. This structure manages group of objects for each smallest sugdivided part of space.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public void `[`insert`](#structbango_1_1space_1_1quad__entity__container_1a298f0af5ebcfe9e10a5eac0ea85a62c6)`(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` * entity)` | Should implement entity insertion operation.
`public void `[`remove`](#structbango_1_1space_1_1quad__entity__container_1a596311429b72a07a0f0640a9aeb37040)`(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` * entity)` | Should implement entity removal operation.
`public void `[`merge`](#structbango_1_1space_1_1quad__entity__container_1a412dfe39039e2569630844820a669212)`(const T * container)` | Should move all entities from container given as parameter to self.
`public size_t `[`size`](#structbango_1_1space_1_1quad__entity__container_1ad9118b74f9cf973b70be7f27b8eb2f36)`() const` | Should calculate number of all elements in this container.
`public long long `[`total_memory`](#structbango_1_1space_1_1quad__entity__container_1ad03575ea64212c980c199d5c9bd730b7)`() const` | Should calculate total memory reserved by this container.
`public void `[`for_each`](#structbango_1_1space_1_1quad__entity__container_1a5239a547fa58805651fe3b9f77089ae5)`(const std::function< void(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity) *)`> &&) const` | Should iterate for each entity in this container and run callback given as parameter.

## Members

#### `public void `[`insert`](#structbango_1_1space_1_1quad__entity__container_1a298f0af5ebcfe9e10a5eac0ea85a62c6)`(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` * entity)` 

Should implement entity insertion operation.

#### `public void `[`remove`](#structbango_1_1space_1_1quad__entity__container_1a596311429b72a07a0f0640a9aeb37040)`(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity)` * entity)` 

Should implement entity removal operation.

#### `public void `[`merge`](#structbango_1_1space_1_1quad__entity__container_1a412dfe39039e2569630844820a669212)`(const T * container)` 

Should move all entities from container given as parameter to self.

#### `public size_t `[`size`](#structbango_1_1space_1_1quad__entity__container_1ad9118b74f9cf973b70be7f27b8eb2f36)`() const` 

Should calculate number of all elements in this container.

#### `public long long `[`total_memory`](#structbango_1_1space_1_1quad__entity__container_1ad03575ea64212c980c199d5c9bd730b7)`() const` 

Should calculate total memory reserved by this container.

#### `public void `[`for_each`](#structbango_1_1space_1_1quad__entity__container_1a5239a547fa58805651fe3b9f77089ae5)`(const std::function< void(const `[`quad_entity`](#structbango_1_1space_1_1quad__entity) *)`> &&) const` 

Should iterate for each entity in this container and run callback given as parameter.

# struct `bango::space::square` 

Rectangle on 2D space represented by 2 corner points - bottom left & top right.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`point`](#structbango_1_1space_1_1point)` `[`bottom_left`](#structbango_1_1space_1_1square_1af694041a90e643d845116fd6268e1f02) | 
`public `[`point`](#structbango_1_1space_1_1point)` `[`top_right`](#structbango_1_1space_1_1square_1a8b3fb2799fecd3bc5a90a139336c8d88) | 

## Members

#### `public `[`point`](#structbango_1_1space_1_1point)` `[`bottom_left`](#structbango_1_1space_1_1square_1af694041a90e643d845116fd6268e1f02) 

#### `public `[`point`](#structbango_1_1space_1_1point)` `[`top_right`](#structbango_1_1space_1_1square_1a8b3fb2799fecd3bc5a90a139336c8d88) 

Generated by [Moxygen](https://sourcey.com/moxygen)