
#include <TwoWireScheduler.h>

class VarioDataTwoWireScheduler
{
public:
    VarioDataTwoWireScheduler();
    void getData(double tmpAlti, double tmpTemp, double tmpAccel);

private:
    int compteurErrorMPU = 0;
};