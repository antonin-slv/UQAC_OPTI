

// polynome interface
class Calculator
{
public:
    virtual float calculate(float x, float y) = 0;
};

class PolyDeg3 : public Calculator
{
public:
    float p00,p10 ,p01,p20,p11,p02,p30,p21,p12,p03 = 0.0f;

    virtual float calculate(float x, float y) override
    {
        return p00 + p10*x + p01*y + p20*x*x + p11*x*y + p02*y*y + p30*x*x*x
                    + p21*x*x*y + p12*x*y*y + p03*y*y*y;
    }
};