TEST(AuthorizbleTest, Roles)
{
    authorizable user;

    const int ROLE_LISTEN   = 1 << 0;
    const int ROLE_TALK     = 1 << 1;
    const int ROLE_MODERATE = 1 << 2;

    EXPECT_EQ(false, user.authorized(ROLE_LISTEN));
    EXPECT_EQ(false, user.authorized(ROLE_TALK));
    EXPECT_EQ(false, user.authorized(ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_TALK));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_TALK | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_TALK | ROLE_MODERATE));

    user.grant(ROLE_LISTEN);

    EXPECT_EQ(true, user.authorized(ROLE_LISTEN));
    EXPECT_EQ(false, user.authorized(ROLE_TALK));
    EXPECT_EQ(false, user.authorized(ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_TALK));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_TALK | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_TALK | ROLE_MODERATE));

    user.grant(ROLE_TALK | ROLE_MODERATE);

    EXPECT_EQ(true, user.authorized(ROLE_LISTEN));
    EXPECT_EQ(true, user.authorized(ROLE_TALK));
    EXPECT_EQ(true, user.authorized(ROLE_MODERATE));
    EXPECT_EQ(true, user.authorized(ROLE_LISTEN | ROLE_TALK));
    EXPECT_EQ(true, user.authorized(ROLE_LISTEN | ROLE_MODERATE));
    EXPECT_EQ(true, user.authorized(ROLE_TALK | ROLE_MODERATE));
    EXPECT_EQ(true, user.authorized(ROLE_LISTEN | ROLE_TALK | ROLE_MODERATE));

    user.ban(ROLE_TALK);

    EXPECT_EQ(true, user.authorized(ROLE_LISTEN));
    EXPECT_EQ(false, user.authorized(ROLE_TALK));
    EXPECT_EQ(true, user.authorized(ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_TALK));
    EXPECT_EQ(true, user.authorized(ROLE_LISTEN | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_TALK | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_TALK | ROLE_MODERATE));
}