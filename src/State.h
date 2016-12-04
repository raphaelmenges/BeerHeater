#ifndef STATE_H_
#define STATE_H_

struct State
{
    float temperature;
    float velocityX;
    float velocityY;
    float velocityZ;
};

namespace state
{
    const State fallback =
            {
                    1.f,
                    0.f,
                    0.f,
                    0.f
            };
}

#endif // STATE_H_
