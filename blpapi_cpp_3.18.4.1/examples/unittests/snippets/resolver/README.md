
# Resolver Test Examples

There are several interactions that are particular to resolvers:
 * Service registration
 * Permission requests

## Mocking ProviderSession

It is possible to create a `MockProviderSession` to be able to mock
interactions with a `blp::ProviderSession` object. An example is provided
in `tests/mockProviderSession.h`.

This `MockProviderSession` can then be used in place of `ProviderSession` by
reference or pointer to set expectations.

For example, given an event handler, `MyEventHandler`, it can be tested as the
following:

```cpp

namespace blp = BloombergLP::blpapi;

bool MyEventHandler::processEvent(const blp::Event&      event,
                                  blp::ProviderSession  *session);

TEST(MyTest, firstTest)
{
    MyEventHandler      testHandler;
    MockProviderSession mockSession;
    blp::Event          testEvent; // See next section

    // Expected actions
    EXPECT_CALL(mockSession, registerService(_, _, _))
            .WillOnce(Return(true));

    testHandler.processEvent(testEvent, &mockSession);
}

```

#### Creating Test Events

In order to be able to test `EventHandler`s or the output of
`session.nextEvent()`, the application should be able to generate custom test
events / messages.

Some samples are provided in this repository (under `tests` folder) for
demonstrating how events are generated using `TestUtil`.
