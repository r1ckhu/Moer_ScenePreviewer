#define STB_IMAGE_IMPLEMENTATION
#include "MoerHandler.h"

#include <fstream>
#include <iostream>
#include <thread>

#include "stb_image.h"
#include "subprocess.h"

const char* moerSettingsFileName = "MoerSettings.json";
const char* moerPathSettingName = "moerPath";
MoerHandler::MoerHandler() : moerState(MOER_STATE::NONE), renderProgress(0) {
   if (std::filesystem::exists(moerSettingsFileName)) {
      std::ifstream f(moerSettingsFileName);
      moerSettings = Json::parse(f);
   } else {
      moerSettings[moerPathSettingName] = ".";
   }
}

void MoerHandler::setMoerPathAndWrite(const std::string& moerPath) {
   moerSettings[moerPathSettingName] = moerPath;
   std::ofstream o(moerSettingsFileName);
   o << moerSettings;
}

std::string MoerHandler::getMoerPath() {
   return moerSettings[moerPathSettingName];
}

bool MoerHandler::isRendering() { return moerState == MOER_STATE::RENDERING; }

bool MoerHandler::isRenderJustCompleted() {
   return moerState == MOER_STATE::RENDER_JUST_COMPLETED;
}

int MoerHandler::getRenderProgress() { return renderProgress.load(); }

static GLuint imageTexture;

void MoerHandler::setRenderResultPicture(
    const std::filesystem::path picturePath) {
   if (glIsTexture(imageTexture)) {
      glDeleteTextures(1, &imageTexture);
   }

   int width, height, componentNr;

   std::string imagePath = picturePath.string();
   stbi_set_flip_vertically_on_load(true);
   unsigned char* data =
       stbi_load(imagePath.c_str(), &width, &height, &componentNr, 0);
   if (data == nullptr) {
      std::cerr << "Set Render Result Picture Failed." << std::endl;
      return;
   }
   glGenTextures(1, &imageTexture);

   glBindTexture(GL_TEXTURE_2D, imageTexture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   GLuint internalFormat = (componentNr == 3) ? GL_RGB : GL_RGBA;
   glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                internalFormat, GL_UNSIGNED_BYTE, data);
   stbi_image_free(data);
   if (moerState == MOER_STATE::RENDER_JUST_COMPLETED) {
      moerState = MOER_STATE::NONE;
   }
}

void MoerHandler::setRenderResultPicture() {
   auto latestHDRFilePath = getLatestHdrFile(".");
   std::cout << "Set Moer render result picture to: " << latestHDRFilePath
             << std::endl;
   this->setRenderResultPicture(getLatestHdrFile("."));
}

void MoerHandler::handleMoerStdOutput(struct subprocess_s* moerProcess) {
   renderProgress.store(0);
   char buffer[1024];
   bool isShowingRenderProgress = false;
   while (subprocess_alive(moerProcess)) {
      if (subprocess_read_stdout(moerProcess, buffer, sizeof(buffer))) {
         if (!isShowingRenderProgress) {
            std::cout << buffer << std::endl;
         } else {
            int progress = std::atoi(buffer);
            if (progress != 0) {
               renderProgress.store(progress);
               if (progress == 100) {
                  isShowingRenderProgress = false;
               }
            }
         }
         if (std::strstr(buffer, "start rendering") != nullptr) {
            isShowingRenderProgress = true;
         }
         memset(buffer, 0, sizeof(buffer));
      }
   }
   // Just set moerState and return
   // Can not call setRenderResultPicture(), because this is not the main thread
   // Its OpenGL context is different from main thread
   moerState = MOER_STATE::RENDER_JUST_COMPLETED;
}

int MoerHandler::executeMoer(const std::filesystem::path& moerPath,
                             const std::filesystem::path& scenePath) {
   std::string sceneName = scenePath.filename().string();
   std::string moerExecutableName = moerPath.filename().string();
   std::string moerPathStr = moerPath.string();
   std::string scenePathStr = scenePath.string();
   setMoerPathAndWrite(moerPathStr);
   const char* command_line[] = {moerPathStr.c_str(), scenePathStr.c_str(),
                                 NULL};
   int result = subprocess_create(
       command_line,
       subprocess_option_inherit_environment | subprocess_option_enable_async,
       &moerProcess);
   if (0 != result) {
      std::cerr << "Error when launching Moer." << std::endl;
      return result;
   }
   static std::thread moerOutputThread(&MoerHandler::handleMoerStdOutput, this,
                                       &moerProcess);
   moerOutputThread.detach();
   moerState = MOER_STATE::RENDERING;
   return 0;
}

void MoerHandler::killMoer() {
   if (subprocess_alive(&moerProcess)) {
      subprocess_terminate(&moerProcess);
      subprocess_destroy(&moerProcess);
   }
}

std::filesystem::path MoerHandler::getLatestHdrFile(
    const std::filesystem::path& directory) {
   std::filesystem::path latest_hdr_file;
   std::filesystem::file_time_type latest_modification_time;

   try {
      for (const auto& entry : std::filesystem::directory_iterator(directory)) {
         if (entry.is_regular_file() && entry.path().extension() == ".hdr") {
            const auto modification_time =
                std::filesystem::last_write_time(entry);
            if (modification_time > latest_modification_time) {
               latest_modification_time = modification_time;
               latest_hdr_file = entry.path();
            }
         }
      }

      if (!latest_hdr_file.empty()) {
         return latest_hdr_file;
      } else {
         std::cerr << "No hdr file found. Has Moer rendering completed?"
                   << std::endl;
      }
   } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << "Error accessing directory: " << e.what() << std::endl;
   }
   return latest_hdr_file;
}

std::optional<unsigned int> MoerHandler::getRenderResultTextureID() {
   if (glIsTexture(imageTexture)) {
      return imageTexture;
   }
   return std::nullopt;
}
