const int ROLE_LISTEN   = 1 << 0;
const int ROLE_TALK     = 1 << 1;
const int ROLE_MODERATE = 1 << 2;

TEST(AuthorizbleTest, RequiredRoles)
{
    authorizable user;

    EXPECT_EQ(false, user.authorized(ROLE_LISTEN));
    EXPECT_EQ(false, user.authorized(ROLE_TALK));
    EXPECT_EQ(false, user.authorized(ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_TALK));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_TALK | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_TALK | ROLE_MODERATE));

    user.assign(ROLE_LISTEN);

    EXPECT_EQ(true, user.authorized(ROLE_LISTEN));
    EXPECT_EQ(false, user.authorized(ROLE_TALK));
    EXPECT_EQ(false, user.authorized(ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_TALK));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_TALK | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_TALK | ROLE_MODERATE));

    user.assign(ROLE_TALK | ROLE_MODERATE);

    EXPECT_EQ(true, user.authorized(ROLE_LISTEN));
    EXPECT_EQ(true, user.authorized(ROLE_TALK));
    EXPECT_EQ(true, user.authorized(ROLE_MODERATE));
    EXPECT_EQ(true, user.authorized(ROLE_LISTEN | ROLE_TALK));
    EXPECT_EQ(true, user.authorized(ROLE_LISTEN | ROLE_MODERATE));
    EXPECT_EQ(true, user.authorized(ROLE_TALK | ROLE_MODERATE));
    EXPECT_EQ(true, user.authorized(ROLE_LISTEN | ROLE_TALK | ROLE_MODERATE));

    user.deny(ROLE_TALK);

    EXPECT_EQ(true, user.authorized(ROLE_LISTEN));
    EXPECT_EQ(false, user.authorized(ROLE_TALK));
    EXPECT_EQ(true, user.authorized(ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_TALK));
    EXPECT_EQ(true, user.authorized(ROLE_LISTEN | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_TALK | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(ROLE_LISTEN | ROLE_TALK | ROLE_MODERATE));
}

TEST(AuthorizbleTest, RestrictedRoles)
{
    authorizable user;

    EXPECT_EQ(true, user.authorized(0, ROLE_LISTEN));
    EXPECT_EQ(true, user.authorized(0, ROLE_TALK));
    EXPECT_EQ(true, user.authorized(0, ROLE_MODERATE));
    EXPECT_EQ(true, user.authorized(0, ROLE_LISTEN | ROLE_TALK));
    EXPECT_EQ(true, user.authorized(0, ROLE_LISTEN | ROLE_MODERATE));
    EXPECT_EQ(true, user.authorized(0, ROLE_TALK | ROLE_MODERATE));
    EXPECT_EQ(true, user.authorized(0, ROLE_LISTEN | ROLE_TALK | ROLE_MODERATE));

    user.assign(ROLE_LISTEN);

    EXPECT_EQ(false, user.authorized(0, ROLE_LISTEN));
    EXPECT_EQ(true, user.authorized(0, ROLE_TALK));
    EXPECT_EQ(true, user.authorized(0, ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(0, ROLE_LISTEN | ROLE_TALK));
    EXPECT_EQ(false, user.authorized(0, ROLE_LISTEN | ROLE_MODERATE));
    EXPECT_EQ(true, user.authorized(0, ROLE_TALK | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(0, ROLE_LISTEN | ROLE_TALK | ROLE_MODERATE));

    user.assign(ROLE_TALK | ROLE_MODERATE);

    EXPECT_EQ(false, user.authorized(0, ROLE_LISTEN));
    EXPECT_EQ(false, user.authorized(0, ROLE_TALK));
    EXPECT_EQ(false, user.authorized(0, ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(0, ROLE_LISTEN | ROLE_TALK));
    EXPECT_EQ(false, user.authorized(0, ROLE_LISTEN | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(0, ROLE_TALK | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(0, ROLE_LISTEN | ROLE_TALK | ROLE_MODERATE));

    user.deny(ROLE_TALK);

    EXPECT_EQ(false, user.authorized(0, ROLE_LISTEN));
    EXPECT_EQ(true, user.authorized(0, ROLE_TALK));
    EXPECT_EQ(false, user.authorized(0, ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(0, ROLE_LISTEN | ROLE_TALK));
    EXPECT_EQ(false, user.authorized(0, ROLE_LISTEN | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(0, ROLE_TALK | ROLE_MODERATE));
    EXPECT_EQ(false, user.authorized(0, ROLE_LISTEN | ROLE_TALK | ROLE_MODERATE));
}