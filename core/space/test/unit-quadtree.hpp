TEST(QuadTree, Insert)
{
    static_entity e1{1,65,"A"};
    static_entity e2{65,65,"B"};
    static_entity e3{2,2,"C"};

    quad q({
        point{0, 0},
        point{128, 128}
    });

    q.insert(&e1);
    q.insert(&e2);
    q.insert(&e3);

    q.remove(&e2);

    //printf("%d\n", (int) q.size());
    q.dump();
}