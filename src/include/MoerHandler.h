#pragma once
#include <atomic>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <thread>

#include "OpenGLHeader.h"
#include "subprocess.h"

using Json = nlohmann::json;

enum class MOER_STATE { NONE, RENDERING, RENDER_JUST_COMPLETED };

class MoerHandler {
  public:
   MoerHandler();
   bool isRendering();
   bool isRenderJustCompleted();
   int getRenderProgress();
   int executeMoer(const std::filesystem::path& moerPath,
                   const std::filesystem::path& scenePath);
   void killMoer();
   void setRenderResultPicture(const std::filesystem::path picturePath);
   void setRenderResultPicture();
   std::optional<unsigned int> getRenderResultTextureID();
   std::string& getMoerPath();
   void setMoerPathAndWrite(const std::string& moerPath);
   void setMoerPath(const std::string& moerPath);
   void writeMoerPath();
   std::filesystem::path getLatestHdrFile(
       const std::filesystem::path& directory);

  private:
   void handleMoerStdOutput(struct subprocess_s* moerProcess);

  private:
   std::atomic<MOER_STATE> moerState;
   std::atomic<int> renderProgress;
   GLuint imageTextureID;
   Json moerSettings;
   struct subprocess_s moerProcess;
   std::thread moerOutputThread;
   bool moerThreadRunning = false;
   std::string moerPath;
};