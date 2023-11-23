#include <iostream>
#include <thread>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace std;

class CommandServer {
public:
    CommandServer(int port) : port(port) {}

    void startServer() {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            cerr << "Error creating socket" << endl;
            return;
        }

        sockaddr_in serverAddress{AF_INET, INADDR_ANY, htons(port)};
        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1 ||
            listen(serverSocket, 1) == -1) {
            cerr << "Error binding or listening on socket" << endl;
            close(serverSocket);
            return;
        }

        cout << "Server is waiting for a connection on port " << port << "..." << endl;

        clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == -1) {
            cerr << "Error accepting connection" << endl;
            close(serverSocket);
            return;
        }

        while (true) {
            char buffer[1024];
            ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0) {
                break;
            }

            buffer[bytesRead] = '\0';
            cout << "Received from user: " << buffer << endl;

            double linear_vel, angular_vel;
            sscanf(buffer, "%lf %lf", &linear_vel, &angular_vel);

            processCommand(linear_vel, angular_vel);
        }

        close(clientSocket);
        close(serverSocket);
    }

    virtual void processCommand(double linear_vel, double angular_vel) = 0;

protected:
    int serverSocket;
    int clientSocket;
    int port;
};

class MotionCommandServer : public CommandServer {
public:
    MotionCommandServer(int port) : CommandServer(port) {}

    void processCommand(double linear_vel, double angular_vel) override {
        if (linear_vel > 0.0) {
            cout << "Moving the robot forward with linear velocity: " << linear_vel << endl;
            // Implement forward motion logic here
        } else if (linear_vel < 0.0) {
            cout << "Moving the robot backward with linear velocity: " << -linear_vel << endl;
            // Implement backward motion logic here
        } else if (angular_vel != 0.0) {
            cout << "Rotating the robot with angular velocity: " << angular_vel << endl;
            // Implement rotation logic here
        }
    }
};

class ScanListener {
public:
    ScanListener(int port, double stop_distance_cm) : port(port), stop_distance_cm(stop_distance_cm) {}

    void startListener() {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            cerr << "Error creating socket" << endl;
            return;
        }

        sockaddr_in serverAddress{AF_INET, INADDR_ANY, htons(port)};
        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1 ||
            listen(serverSocket, 1) == -1) {
            cerr << "Error binding or listening on socket" << endl;
            close(serverSocket);
            return;
        }

        cout << "Scan listener is waiting for a connection on port " << port << "..." << endl;

        clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == -1) {
            cerr << "Error accepting connection" << endl;
            close(serverSocket);
            return;
        }

        double previous_distance = -1.0;
        double start_angle = -1.0;
        double end_angle = -1.0;

        while (true) {
            char buffer[1024];
            ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0) {
                break;
            }

            buffer[bytesRead] = '\0';
            double distance = atof(buffer);

            processDistance(distance);

            if (previous_distance != -1.0) {
                if (distance - previous_distance > 5.0) {
                    start_angle = distance;
                } else if (previous_distance - distance > 5.0 && start_angle != -1.0) {
                    end_angle = distance;
                }

                if (start_angle != -1.0 && end_angle != -1.0) {
                    double diameter = end_angle - start_angle;
                    cout << "Estimated diameter of the circle: " << diameter << " cm" << endl;
                    start_angle = -1.0;
                    end_angle = -1.0;
                }
            }

            previous_distance = distance;

            if (distance <= stop_distance_cm) {
                cout << "Stop point reached at distance: " << distance << " cm" << endl;
            }
        }

        close(clientSocket);
        close(serverSocket);
    }

private:
    int serverSocket;
    int clientSocket;
    int port;
    double stop_distance_cm;

    void processDistance(double distance) {
        cout << "Received distance data from the sensor: " << distance << " cm" << endl;

        if (distance <= stop_distance_cm) {
            cout << "Stop point reached at distance: " << distance << " cm" << endl;
        }
    }
};

int main() {
    
    thread motionServerThread(&MotionCommandServer::startServer, &cmdVelServer);
    thread scanListenerThread(&ScanListener::startListener, &scanListener);

    motionServerThread.join();
    scanListenerThread.join();

    return 0;
}