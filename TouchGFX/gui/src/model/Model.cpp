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

    // In case we are waiting for saving to finish, check if it is done
    if (audio_save_flag == 1) {
        // Wait for save to finish
        if (osSemaphoreAcquire(saveFiniSemHandle, 0) == osOK) {
            audio_save_flag = 0;
            if (modelListener) {
                modelListener->saveCompleted();
            }
        }
    }
#endif

}
