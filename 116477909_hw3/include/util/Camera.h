/// STOP. You should not modify this file unless you KNOW what you are doing.

#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>


class Camera
{
public:
    // Defines several possible options for camera movement.
    // Used as abstraction to stay away from pWindow-system specific input methods.
    enum Movement: int
    {
        kUp,
        kDown,
        kLeft,
        kRight,
        kForward,
        kBackWard
    };

    // Default camera values
    static constexpr float kDefaultYaw = -90.0f;
    static constexpr float kDefaultPitch = 0.0f;
    static constexpr float kDefaultSpeed = 2.5f;
    static constexpr float kDefaultSensitivity = 0.1f;
    static constexpr float kDefaultZoom = 45.0f;

public:
    Camera() = delete;

    explicit Camera(glm::vec3 position = {0.0f, 0.0f, 0.0f},
                    glm::vec3 up = {0.0f, 1.0f, 0.0f},
                    float yaw = kDefaultYaw,
                    float pitch = kDefaultPitch)
            : position(position),
              front({}),
              up(up),
              right({}),
              worldUp(up),
              yaw(yaw),
              pitch(pitch),
              movementSpeed(kDefaultSpeed),
              mouseSensitivity(kDefaultSensitivity),
              zoom(kDefaultZoom)
    {
        updateCameraVectors();

#ifdef DEBUG_CAMERA
        std::cout << "position = " << this->position << '\n'
                  << "   front = " << this->front << '\n'
                  << "   right = " << this->right << '\n'
                  << "      up = " << this->up << '\n'
                  << " worldUp = " << this->worldUp << '\n';
#endif  // DEBUG_CAMERA
    }

    Camera(const Camera &) = default;
    Camera(Camera &&) = default;
    Camera & operator=(const Camera &) = default;
    Camera & operator=(Camera &&) = default;

    ~Camera() = default;

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix.
    [[nodiscard]] glm::mat4 getViewMatrix() const
    {
        return glm::lookAt(position, position + front, up);
    }

    // Processes input received from any keyboard-like input system.
    // Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems).
    void processKeyboard(Movement direction, double deltaTime)
    {
        float displacement = movementSpeed * static_cast<float>(deltaTime);

        switch (direction)
        {
        case kUp:
        {
            position += up * displacement;
            break;
        }
        case kDown:
        {
            position -= up * displacement;
            break;
        }
        case kLeft:
        {
            position -= right * displacement;
            break;
        }
        case kRight:
        {
            position += right * displacement;
            break;
        }
        case kForward:
        {
            position += front * displacement;
            break;
        }
        case kBackWard:
        {
            position -= front * displacement;
            break;
        }
        default:
        {
            throw std::invalid_argument("unknown CameraMovement enumerator " +
                                        std::to_string(static_cast<int>(direction)));
        }
        }
    }

    // Processes input received from a mouse input system.
    // Expects the offset value in both the x and y direction.
    void processMouseMovement(double xoffset, double yoffset, bool constrainPitch = true)
    {
        yaw += static_cast<float>(xoffset) * mouseSensitivity;
        pitch += static_cast<float>(yoffset) * mouseSensitivity;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped.
        if (constrainPitch)
        {
            if (89.0f < pitch)
            {
                pitch = 89.0f;
            }

            if (pitch < -89.0f)
            {
                pitch = -89.0f;
            }
        }

        // Update Front, Right and Up Vectors using the updated Euler angles.
        updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event.
    // Only requires inputs on the vertical wheel-axis.
    void processMouseScroll(double yoffset)
    {
        zoom -= static_cast<float>(yoffset);

        if (zoom < 1.0f)
        {
            zoom = 1.0f;
        }

        if (zoom > 45.0f)
        {
            zoom = 45.0f;
        }
    }

public:
    // Camera attributes
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Euler angles
    float yaw;
    float pitch;

    // Camera options
    float movementSpeed;
    float mouseSensitivity;
    float zoom;

private:
    // Calculates the front vector from the Camera's (updated) Euler angles
    void updateCameraVectors()
    {
        front.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
        front.y = std::sin(glm::radians(pitch));
        front.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
        front = glm::normalize(front);

        // Normalize the vectors,
        // because their length gets closer to 0 the more you look up or down which results in slower movement.
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};


#include <chrono>

class Timer {
public:
    Timer() : beg_(clock_::now()) {}

    void reset() { beg_ = clock_::now(); }

    double elapsed() const {
        return std::chrono::duration_cast<second_> (clock_::now() - beg_).count();
    }

private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1> > second_;
    std::chrono::time_point<clock_> beg_;
};


class KeyFrameCamera
{
    struct KeyFrame
    {
        glm::vec3 k_Position;
        glm::quat K_rotation;
        float Time_from_start;
    };

    float Convert_time_to_T()
    {
        begin_frame_index = 0;
        float current_elapsed_Time = KeyFrameCamera_timer.elapsed();
        KeyFrame var2;

        for (KeyFrame var : Frames)
        {
            if (!(begin_frame_index + 1 >= Frames.size()))
            {
                var2 = Frames[begin_frame_index + 1];
            }
            else
            {
                is_at_last = true;
                continue;
            }

            if (current_elapsed_Time > var.Time_from_start && current_elapsed_Time < var2.Time_from_start)
            {
                break;
            }

            begin_frame_index++;
        }


        if (!is_at_last)
        {
            //amount of time between current keyFrames
            float Time_frame = Frames[begin_frame_index + 1].Time_from_start - Frames[begin_frame_index].Time_from_start;

            //difference between elapsed time and first frame;
            float time_difference = current_elapsed_Time - Frames[begin_frame_index].Time_from_start;

            //value between 0 and 1 thats used to calculate interpolation
            float T = time_difference / Time_frame;
            return T;
        }

        else
        {
            return 1.0f;
        }
    }

public:
    KeyFrameCamera(glm::vec3 start_pos, glm::quat start_rot)
        :lerped_position(start_pos), slerped_rotation(start_rot)
    {
        KeyFrame Instance;
        Instance.Time_from_start = 0;
        Instance.k_Position = start_pos;
        Instance.K_rotation = start_rot;

        Frames.push_back(Instance);
    }

    ~KeyFrameCamera()
    {

    }

    void pushBackFrame(glm::vec3 k_Position, glm::quat K_rotation, float Time_to_Run)
    {
        float time_from_start = 0;

        if (Frames.size() > 0)
            time_from_start = Frames[Frames.size() - 1].Time_from_start + Time_to_Run;

        else
            time_from_start = Time_to_Run;

        KeyFrame Instance;
        Instance.Time_from_start = time_from_start;
        Instance.k_Position = k_Position;
        Instance.K_rotation = K_rotation;

        Frames.push_back(Instance);
    }

    void InterPolate()
    {
        float T = 0;

        if (isAnimating)
        {


            T = this->Convert_time_to_T();




            if (!is_at_last)
            {
                //interpolates the position;
                this->lerped_position = glm::mix(Frames[begin_frame_index].k_Position, Frames[begin_frame_index + 1].k_Position, T);

                //interpolates the Rotation
                this->slerped_rotation = glm::slerp(Frames[begin_frame_index].K_rotation, Frames[begin_frame_index + 1].K_rotation, T);
            }
        }
    }

    void setBias(glm::mat4 B)
    {
        Bias = B;
    }

    void setBias(glm::vec3 B)
    {
        V_pos = B;
        Bias = glm::translate(glm::mat4(1.0f), V_pos);
    }


    void startKeyFrameCamera()
    {
        KeyFrameCamera_timer.reset();
        isAnimating = true;
    }

    glm::mat4 GetBias()
    {
        return Bias;
    }
    glm::mat4 GetView()
    {
        glm::mat4 Model;

        {
            InterPolate();
            //glm::mat4 Model =  glm::translate(glm::mat4(1.0f), lerped_position) * glm::translate(glm::mat4(1.0f), -1.0f * V_pos) * glm::mat4(slerped_rotation) * glm::translate(glm::mat4(1.0f), -1.0f * V_pos);
            Model = glm::translate(glm::mat4(1.0f), lerped_position) * glm::mat4(slerped_rotation);

            //inverse it For Camera 
            Model = glm::inverse(Model);
        }


        return Model;
    }



    glm::vec3 GetlerpedPosition()
    {
        return lerped_position;
    }

private:
    std::vector<KeyFrame> Frames;
    Timer KeyFrameCamera_timer;  bool isAnimating;
    int begin_frame_index = 0;
    bool is_at_last = false;


    glm::vec3 lerped_position;
    glm::quat slerped_rotation;

    glm::mat4 Bias = glm::mat4(1.0f);
    glm::vec3 V_pos = glm::vec3(0.0f);
};





#endif  // CAMERA_H
