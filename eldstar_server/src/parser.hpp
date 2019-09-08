#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

#include "socket.hpp"
#include "game/world.hpp"


namespace eldstar {

class socket_parser {
    public:
        socket_parser() : reader_thread(&socket_parser::run, this), stop_thread(false), target(std::make_shared<game::world>()) {
            // Wait until the thread has started up before exiting the constructor
            std::unique_lock<std::mutex> lock(mutex);
            startup_unlock.wait(lock);
        }

        ~socket_parser() {
            stop();
        }

        void run() {
            tcp_socket sock;

            {
                std::unique_lock<std::mutex> lock(mutex);
                startup_unlock.notify_all();
            }

            while (!stop_thread) {
                std::unique_ptr<tcp_client_session> client;

                // Accept a client connection
                try {
                    client = std::unique_ptr<tcp_client_session>(new tcp_client_session(sock, 1));
                } catch (socket_timeout) {
                    continue;
                }

                // Set up message buffer space
                std::string heap_buffer;
                char stack_buffer[8193];
                int terminator = 8192;

                // Buffer waiting data
                while ((terminator = client->receive(stack_buffer, 8192)) > 0) {
                    // Add to the contiguous heap buffer
                    heap_buffer.append(stack_buffer, terminator);

                    // If we see a line break, parse the instruction and remove it from the buffer
                    size_t found;
                    while ((found = heap_buffer.find("\n")) != std::string::npos) {
                        handle_line(heap_buffer.substr(0, found));
                        heap_buffer.erase(0, found + 1);
                    }
                }
            }
        }

        void handle_line(std::string line) {
            // Ignore blank lines
            if (line.length() == 0)
                return;
            // Remove carriage return if present
            if (line.back() == '\r')
                line.pop_back();

            char* ptr = const_cast<char*>(line.c_str());
            char code = ptr[0];
            ptr++;

            switch (code) {
                case 'e': {
                    long frame = parse_long(ptr);
                    std::string user_agent(ptr);

                    target->frame = frame;
                    target->description = user_agent + " - Frame " + std::to_string(frame);

                    std::lock_guard<std::mutex> lock(mutex);

                    buffered_worlds.push_back(target);
                    target = std::make_shared<game::world>();

                    return;
                }
                case 'g': {
                    long id = parse_long(ptr, 16);
                    float coords[12];

                    for (int i = 0; i < 12; i++)
                        coords[i] = parse_float(ptr);

                    long active = parse_long(ptr);

                    target->add_triangle(
                        id,
                        gl::triangle(
                            glm::vec3(coords[0], coords[1], coords[2]),
                            glm::vec3(coords[3], coords[4], coords[5]),
                            glm::vec3(coords[6], coords[7], coords[8]),
                            glm::vec3(coords[9], coords[10], coords[11]),
                            gl::color(1.0f, active == 1 ? 0.9f : 0.25f)
                        )
                    );

                    return;
                }
                case 'w': {
                    long id = parse_long(ptr, 16);
                    float data[9];

                    for (int i = 0; i < 9; i++)
                        data[i] = parse_float(ptr);

                    target->add(id, game::world_object(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]));

                    return;
                }
                case 'd': {
                    long id = parse_long(ptr, 16);
                    float data[6];

                    for (int i = 0; i < 6; i++)
                        data[i] = parse_float(ptr);

                    target->add(id, game::dynamic_object(data[0], data[1], data[2], data[3], data[4], data[5]));

                    return;
                }
                case 'i': {
                    long id = parse_long(ptr, 16);
                    float data[3];

                    for (int i = 0; i < 3; i++)
                        data[i] = parse_float(ptr);

                    target->add(id, game::item_object(data[0], data[1], data[2]));

                    return;
                }
                case 'm': {
                    float data[4];

                    for (int i = 0; i < 4; i++)
                        data[i] = parse_float(ptr);

                    target->mario = game::dynamic_object(data[0], data[1], data[2], data[3]);

                    return;
                }
                case 'c': {
                    float data[8];

                    for (int i = 0; i < 8; i++)
                        data[i] = parse_float(ptr);

                    target->camera_side_pan = data[0];
                    target->camera_position = glm::vec3(data[1], data[2], data[3]);
                    target->camera_target = glm::vec3(data[4], data[5], data[6]);
                    target->camera_yaw = data[7];

                    return;
                }
                default: return;
            }
        }

        long parse_long(char*& pointer, int base = 10) {
            return strtol(pointer, &pointer, base);
        }

        float parse_float(char*& pointer) {
            return strtof(pointer, &pointer);
        }

        std::shared_ptr<game::world> get_world() {
            std::lock_guard<std::mutex> lock(mutex);

            if (buffered_worlds.size() == 0)
                return std::shared_ptr<game::world>();

            std::shared_ptr<game::world> first = buffered_worlds.front();
            buffered_worlds.pop_front();

            return first;
        }

        void stop() {
            stop_thread = true;
            reader_thread.join();
        }

    private:
        std::thread reader_thread;

        std::mutex mutex;
        std::condition_variable startup_unlock;

        bool stop_thread;

        std::shared_ptr<game::world> target;
        std::deque<std::shared_ptr<game::world>> buffered_worlds;
};

} // eldstar
