#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#ifndef SIMULATOR

#include "main.h"
#include "cmsis_os2.h"

extern osSemaphoreId_t saveFiniSemHandle;

#endif

Model::Model() : modelListener(0)
{
#ifndef SIMULATOR
    if (osOK == osSemaphoreAcquire(saveFiniSemHandle, 0)) {
        modelListener->saveCompleted();
    }
#endif
}

void Model::tick()
{
#ifndef SIMULATOR
    // Saving finished
    if (osSemaphoreAcquire(saveFiniSemHandle, 0) == osOK) {
        if (modelListener) {
            modelListener->saveCompleted();
        }
    }
#endif

}
