#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#ifndef SIMULATOR

#include "main.h"
#include "cmsis_os.h"

extern osSemaphoreId saveFiniSemHandle;

#endif

Model::Model() : modelListener(0)
{

}

void Model::tick()
{
#ifndef SIMULATOR
    // Saving finished
    if (osSemaphoreWait(saveFiniSemHandle, 0) == osOK) {
        if (modelListener) {
            modelListener->saveCompleted();
        }
    }
#endif

}
