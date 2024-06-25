#include "MoerControl.h"

#include <ImGuiFileDialog.h>

#include "IconsFontAwesome6.h"
#include "imgui_stdlib.h"
static MoerHandler moerHandler;
static bool openFileDialogForMoer = false;
static ImGuiFileDialog moerFileDialog;

MoerHandler& getMoerHandler() { return moerHandler; }
bool isMoerFileDialogOpen() { return openFileDialogForMoer; }
static void HelpMarker(const char* desc) {
   ImGui::TextDisabled("(?)");
   if (ImGui::BeginItemTooltip()) {
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
      ImGui::TextUnformatted(desc);
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
   }
}

int calJsonFileCount(const std::string& path) {
   int jsonFileCount = 0;
   for (const auto& entry : std::filesystem::directory_iterator(path)) {
      if (entry.path().extension() == ".json") {
         jsonFileCount++;
      }
   }
   return jsonFileCount;
}

void showMoerFileDialog() {
#ifdef _WIN32
   moerFileDialog.OpenDialog("moerDialog", "Choose Moer executable", ".exe",
                             moerHandler.getMoerPath());
#else
   moerFileDialog.OpenDialog("moerDialog", "Choose Moer executable", "",
                             moerPath);
#endif
   if (moerFileDialog.Display("moerDialog", ImGuiWindowFlags_None)) {
      if (moerFileDialog.IsOk()) {
         moerHandler.getMoerPath() = moerFileDialog.GetFilePathName();
         moerHandler.setMoerPathAndWrite(moerHandler.getMoerPath());
      }
      moerFileDialog.Close();
      openFileDialogForMoer = false;
   }
}

void showControl(std::shared_ptr<Scene> scene) {
   static int jsonCount, launchResult = 0;

   ImGui::BeginDisabled(moerHandler.getMoerPath().empty() ||
                        scene->camera == nullptr);

   if (ImGui::Button(ICON_FA_PLAY " Start")) {
      try {
         jsonCount = calJsonFileCount(scene->workingDir);
         launchResult = moerHandler.executeMoer(moerHandler.getMoerPath(),
                                                scene->workingDir);
      } catch (std::exception& e) {
         launchResult = -1;
      }
      // moerHandler.setRenderResultPicture(moerHandler.getLatestHdrFile("."));
   }
   ImGui::SameLine();
   ImGui::Button(ICON_FA_SQUARE " End");
   ImGui::SameLine();
   ImGui::Button(ICON_FA_GEAR " Config");

   ImGui::EndDisabled();

   ImGui::SameLine();

   if (moerHandler.isRenderJustCompleted() || moerHandler.isRendering()) {
      ImGui::ProgressBar(
          static_cast<float>(moerHandler.getRenderProgress()) / 100.f,
          ImVec2(0.0f, 0.0f));
   } else {
      ImGui::ProgressBar(0, ImVec2(0.0f, 0.0f));
   }
   ImGui::SameLine();
   HelpMarker(
       "After the progress bar disappears, please check the save path of the "
       "render result in the terminal output.");
   if (launchResult != 0) {
      ImGui::TextColored(ImVec4(1.0, 0, 0, 1.f), "Error when launching Moer.");
   } else if (jsonCount > 1) {
      ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f),
                         "Warning: Multiple json files are founded under "
                         "current scene directory. "
                         "Only 'scene.json' will be rendered by Moer.");
   }
}

void showMoerControlWindow(std::shared_ptr<Scene> scene) {
   ImGuiWindowFlags window_flags = 0;
   // auto main_viewport = ImGui::GetMainViewport();
   // ImGui::SetNextWindowPos(
   //     ImVec2(main_viewport->WorkPos.x,
   //            main_viewport->WorkPos.y + ImGui::GetFontSize() * 1.6),
   //     ImGuiCond_Once);

   ImGui::Begin("Moer Control Window", nullptr, window_flags);
   if (ImGui::Button(" " ICON_FA_FOLDER " ")) {
      openFileDialogForMoer = true;
   }
   ImGui::SameLine();
   ImGui::InputText("Moer Path", &moerHandler.getMoerPath());

   if (openFileDialogForMoer) {
      showMoerFileDialog();
   }
   showControl(scene);
   ImGui::End();
}