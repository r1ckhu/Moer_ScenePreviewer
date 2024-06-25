#include "Camera.h"

PinHoleCamera::PinHoleCamera(Point3f lookFrom, Point3f lookAt, Vector3f up,
                             float xFov, Vector2i resolution) {
   setActualValue(lookFrom, lookAt, up, xFov, resolution);
}

PinHoleCamera::PinHoleCamera(const Json& cameraJson) {
   const Json& transform_json = cameraJson.at("transform");
   Vector3f lookAt, lookFrom, up;
   if (transform_json.contains("position")) {
      auto position = transform_json.at("position");
      lookFrom = Vector3f(position[0], position[1], position[2]);
   } else {
      lookFrom = Vector3f(0, 0, 0);
   }

   if (transform_json.contains("look_at")) {
      auto look_at = transform_json.at("look_at");
      lookAt = Vector3f(look_at[0], look_at[1], look_at[2]);
   } else {
      lookAt = Vector3f(0, 1, 0);
   }

   if (transform_json.contains("up")) {
      auto _up = transform_json.at("up");
      up = Vector3f(_up[0], _up[1], _up[2]);
   } else {
      up = Vector3f(0, 0, 1);
   }

   float xFov = 45.f;
   if (cameraJson.contains("fov")) {
      xFov = cameraJson.at("fov");
   }

   Vector2i resolution;
   if (cameraJson.contains("resolution")) {
      auto _resolution = cameraJson.at("resolution");
      if (_resolution.is_array()) {
         resolution = Vector2i(_resolution[0], _resolution[1]);
      } else {
         resolution = Vector2i(_resolution, _resolution);
      }
   } else {
      resolution = Vector2i(512, 512);
   }
   setActualValue(lookFrom, lookAt, up, xFov, resolution);
}

void PinHoleCamera::setActualValue(Point3f lookFrom, Point3f lookAt,
                                   Vector3f up, float xFov,
                                   Vector2i resolution) {
   float aspectRatio = float(resolution[0]) / resolution[1];
   float distToFilm = 1.0f / tan(xFov * Transform::PI_F / 360);
   view = Transform::getView(lookFrom, lookAt - lookFrom, up);

   float nearPlane = distToFilm;
   float farPlane = 100.f;
   projection = Transform::getOpenGLPerspective(
       Transform::AngleDegreeValue(xFov), aspectRatio, nearPlane, farPlane);

   this->right = view.row(0).head<3>().normalized();
   this->up = view.row(1).head(3).normalized();
   this->vecLookAt = -view.row(2).head<3>().normalized();
   this->pointLookAt = lookAt;
   this->cameraPosition = lookFrom;
   this->xFov = xFov;
   this->model = Matrix4f::Identity();
   this->aspectRatio = aspectRatio;

   ini_pointLookAt = pointLookAt;
   ini_vecLookAt = vecLookAt;
   ini_cameraPosition = cameraPosition;
   ini_up = up;
   ini_xFov = xFov;
}

void PinHoleCamera::adjustYawAndPitch(float deltaYaw, float deltaPitch) {
   Eigen::AngleAxisf yawRotation(deltaYaw, up);
   vecLookAt = yawRotation * vecLookAt;
   right = vecLookAt.cross(up);

   Eigen::AngleAxisf pitchRotation(deltaPitch, right);
   vecLookAt = pitchRotation * vecLookAt;
   up = right.cross(vecLookAt);

   up.normalize();
   right.normalize();
   vecLookAt.normalize();

   pointLookAt = cameraPosition + vecLookAt;
}

void PinHoleCamera::reCalculateAllMatrices() {
   /*
   Recalculate all the matrices: view, projection. (model is fixed)

   This function is called exery frame because camera properties can be changed
   at any time through manual input or keyboard movement.

   Could slow down the performace, but who cares? :)
   */

   /*------------View Matrix------------*/
   vecLookAt = pointLookAt - cameraPosition;
   view = Transform::getView(cameraPosition, vecLookAt, up);

   /*------------Projection Matrix------------*/
   float distToFilm = 1.0f / tan(xFov * Transform::PI_F / 360);
   float nearPlane = distToFilm;
   float farPlane = 100.f;
   projection = Transform::getOpenGLPerspective(
       Transform::AngleDegreeValue(xFov), aspectRatio, nearPlane, farPlane);

   this->right = view.row(0).head<3>().normalized();
   this->up = view.row(1).head(3).normalized();
   this->vecLookAt = -view.row(2).head<3>().normalized();
}

void PinHoleCamera::update(Shader shader) {
   reCalculateAllMatrices();
   shader.setMatrix4f("model", model);
   shader.setMatrix4f("view", view);
   shader.setMatrix4f("projection", projection);
   shader.setVector3f("viewPos", cameraPosition);
}

void PinHoleCamera::resetToInitValue() {
   this->up = ini_up;
   this->vecLookAt = ini_vecLookAt;
   this->pointLookAt = ini_pointLookAt;
   this->cameraPosition = ini_cameraPosition;
   this->xFov = ini_xFov;
}

void PinHoleCamera::resetPosition() {
   this->cameraPosition = ini_cameraPosition;
}
void PinHoleCamera::resetLookAtPoint() { this->pointLookAt = ini_pointLookAt; }
void PinHoleCamera::resetXFov() { this->xFov = ini_xFov; }