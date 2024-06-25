#pragma once

#include "MoerHandler.h"
#include "Scene.h"

bool isMoerFileDialogOpen();

MoerHandler& getMoerHandler();

void showMoerControlWindow(std::shared_ptr<Scene> scene);
