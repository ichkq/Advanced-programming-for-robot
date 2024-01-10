#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <ctype.h>

#define RCVBUFSIZE 10000  /* Size of receive buffer */
#define MAX_ENTRIES 20
#define MAX_NAME_LENGTH 50
#define MAX_VALUE_LENGTH 50

int sock;                        /* Socket descriptor */
struct sockaddr_in echoServAddr; /* Echo server address */
unsigned short echoServPort ;     /* Echo server port */
char *servIP = "127.0.0.1";                    /* Server IP address for test  */
// char *servIP = "192.168.100.5X";                    /* Server IP address turtlebot(change X) */
char *echoString = "connection working";                /* String to send to echo server */
char *echoScan = "scan data";                /* String to send to echo server */ 
char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
unsigned int echoStringLen;      /* Length of string to echo */
int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() and total bytes read */

void DieWithError(char *errorMessage);  /* Error handling function */

void connection(int port){
    echoServPort = port;
    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    echoServAddr.sin_port        = htons(echoServPort); /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");
}

void message( char *argv ){
    echoStringLen = strlen(argv);          /* Determine input length */
    /* Send the string to the server */
    if (send(sock, argv, echoStringLen, 0) != echoStringLen)
        DieWithError("send() sent a different number of bytes than expected");
}

// void receive(){
//     /* Receive the same string back from the server */
//     totalBytesRcvd = 0;
//     while (totalBytesRcvd < echoStringLen)
//     {
//         /* Receive up to the buffer size (minus 1 to leave space for a null terminator) bytes from the sender */
//         if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
//             DieWithError("recv() failed or connection closed prematurely");
//         totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
//         echoBuffer[bytesRcvd] = '\0';  /* Terminate the string! */
//         printf("%s", echoBuffer);      /* Print the echo buffer */
//     }

//     printf("\n");    /* Print a final linefeed */
// }

char *receive() {
    /* Allocate memory for the buffer to store received data */
    char *receivedData = (char *)malloc(RCVBUFSIZE * sizeof(char));
    if (receivedData == NULL) {
        DieWithError("Memory allocation failed");
    }

    /* Receive data from the server */
    totalBytesRcvd = 0;
    while (totalBytesRcvd < echoStringLen)
    {
        /* Receive up to the buffer size (minus 1 to leave space for a null terminator) bytes from the sender */
        if ((bytesRcvd = recv(sock, receivedData + totalBytesRcvd, RCVBUFSIZE - totalBytesRcvd - 1, 0)) <= 0) {
            DieWithError("recv() failed or connection closed prematurely");
        }
        totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
        receivedData[totalBytesRcvd] = '\0';  /* Terminate the string! */
    }

    printf("%s\n", receivedData);    /* Print the received buffer */
    printf("\n");    /* Print a final linefeed */

    return receivedData; // Return the received data
}


char *scan() {
    // Connexion au serveur
    connection(9997); // besoin de mettre le numéro de port correct
    //message("Each laser scan is a single scan line. The sensor_msgs/LaserScan message contains the following information: # Single scan from a planar laser range-finder Header header # stamp: The acquisition time of the first ray in the scan. # frame_id: The laser is assumed to spin around the positive Z axis # (counterclockwise, if Z is up) with the zero angle forward along the x axis float32 angle_min: 1.57 float32 angle_max: 1.57 float32 angle_increment: 0.0175 float32 time_increment: 0.001 float32 scan_time: 0.0333 float32 range_min: 0.0 float32 range_max: 100.0 float32[] ranges: [1.0, 2.5, 3.7, 5.2, 10.0] float32[] intensities: []"); // juste pour le test
    char *receivedData = receive(); // Recevoir les données
    return receivedData; // Retourner les données reçues
}

char *odom(){
    connection(9998); // need to put 9998
    //message("header: seq: 4733 stamp: secs: 263 nsecs: 344000000 frame_id: \"odom\" child_frame_id: \"base_footprint\" pose: pose: position: x: 8.37434741746112 y: 1.4092180198976625 z: -0.0010020649388042466 orientation: x: 0.002962980545313219 y: -0.0024686385514236936 z: -0.7677161134726355 w: -0.6407785075082201 covariance: [1e-05, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1e-05, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1000000000000.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1000000000000.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1000000000000.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.001] twist: twist: linear: x: 0.30825907948217207 y: -1.4623052624194999e-05 z: 0.0 angular: x: 0.0 y: 0.0 z: 0.2536419139200637 covariance: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]"); // just for the test
    char *receivedData = receive(); // Recevoir les données
    return receivedData; // Retourner les données reçues
}

void cmd_vel(int linear, float angular) {

    char command[100]; // Creating a character array to store the command
    // Formatting the command with linear and angular values
    snprintf(command, sizeof(command), "---Start---{\"linear\":%d , \"angular\":%f}___END___", linear, angular);

    message(command); // Sending the formatted command to the message function
}

void filterLaserScanData(const char *message, char *dataNames[], char *dataValues[], int *numEntries) {

    *numEntries = 0;

    const char *ptr = message;

    while (*ptr != '\0') {
        const char *nameStart = strstr(ptr, "float32 ");
        const char *valueStart = strstr(ptr, ": ");

        if (nameStart != NULL && valueStart != NULL) {
            nameStart += strlen("float32 ");

            int nameLength = strcspn(nameStart, ":") + 1;
            dataNames[*numEntries] = (char *)malloc((nameLength + 1) * sizeof(char));
            strncpy(dataNames[*numEntries], nameStart, nameLength);
            dataNames[*numEntries][nameLength - 1] = '\0';

            valueStart += strlen(": ");

            int valueLength = strcspn(valueStart, " ") + 1;
            dataValues[*numEntries] = (char *)malloc((valueLength + 1) * sizeof(char));
            strncpy(dataValues[*numEntries], valueStart, valueLength);
            dataValues[*numEntries][valueLength - 1] = '\0';

            (*numEntries)++;
        }

        ptr = strstr(ptr, "float32 ");
        if (ptr != NULL) {
            ptr += strlen("float32 ");
        } else {
            break;
        }
    }
}

void filterOdometryData(const char *message, char *dataNames[], char *dataValues[], int *numEntries) {

    *numEntries = 0;

    const char *ptr = message;

    while (*ptr != '\0') {
        const char *nameStart = strstr(ptr, ": ");
        if (nameStart != NULL) {
            nameStart += 2; // Move past ": "

            int nameLength = strcspn(nameStart, ":");
            dataNames[*numEntries] = (char *)malloc((nameLength + 1) * sizeof(char));
            strncpy(dataNames[*numEntries], nameStart, nameLength);
            dataNames[*numEntries][nameLength] = '\0';

            const char *valueStart = strchr(nameStart, '\n');
            if (valueStart != NULL) {
                valueStart += 1; // Move past newline character

                int valueLength = strcspn(valueStart, "\n");
                dataValues[*numEntries] = (char *)malloc((valueLength + 1) * sizeof(char));
                strncpy(dataValues[*numEntries], valueStart, valueLength);
                dataValues[*numEntries][valueLength] = '\0';

                (*numEntries)++;
            }
        }

        ptr = strchr(ptr, '\n');
        if (ptr != NULL) {
            ptr += 1; // Move past newline character
        } else {
            break;
        }
    }
}

void filterData(const char *message, char *dataNames[], char *dataValues[], int *numEntries) {
    *numEntries = 0;

    const char *ptr = message;

    while (*ptr != '\0') {
        const char *nameEnd = strchr(ptr, ':');
        const char *valueStart = NULL;

        if (nameEnd != NULL) {
            const char *nameStart = nameEnd;
            while (nameStart > message && !isspace(*(nameStart - 1))) {
                nameStart--; // Move to the start of the name
            }

            // Find the start of the value skipping the ":"
            valueStart = nameEnd + 1;
            while (*valueStart != '\0' && isspace(*valueStart)) {
                valueStart++; // Move to the start of the value
            }

            const char *valueEnd = valueStart;
            while (*valueEnd != '\0' && !isspace(*valueEnd) && *valueEnd != ':' && *valueEnd != ',') {
                valueEnd++; // Move to the end of the value
            }

            int nameLength = nameEnd - nameStart;
            int valueLength = valueEnd - valueStart;

            // Allocate memory and copy the name
            dataNames[*numEntries] = (char *)malloc((nameLength + 1) * sizeof(char));
            strncpy(dataNames[*numEntries], nameStart, nameLength);
            dataNames[*numEntries][nameLength] = '\0';

            // Allocate memory and copy the value
            dataValues[*numEntries] = (char *)malloc((valueLength + 1) * sizeof(char));
            strncpy(dataValues[*numEntries], valueStart, valueLength);
            dataValues[*numEntries][valueLength] = '\0';

            (*numEntries)++;
        }

        ptr = nameEnd; // Move to the end of the name:value pair
        if (ptr != NULL) {
            ptr += 1; // Move to the next character after ":"
        } else {
            break;
        }
    }
}




// Fonction pour déplacer le robot sur une distance donnée
void moveRobot_linear(double distance) {

    double totalDistance = 0.0;
    double speed = 1.0; 

    cmd_vel(speed,0); 
    receive();

    while (totalDistance < distance) {
        usleep(100000); // Attendre 0.1 seconde (à ajuster selon le robot et l'environnement)

        totalDistance += speed * 0.1; // 0.1 est le temps de chaque itération

        if (totalDistance >= distance) {
            // Arrêter le robot (envoyer une commande d'arrêt)
            cmd_vel(0,0); 
            printf("Stop: ");
            receive();
            break;
        }
    }
}

// Function to make the robot move along a circular path with a specified radius
void moveRobot_circular(double radius) {

    double speed = 1.0; // Adjust the speed as needed
    double angularSpeed = speed / radius; // Calculating angular speed based on radius

    double circumference = 2.0 * 3.14159 * radius; // Calculating the circumference of the circle

    // Moving the robot with calculated angular speed to make it follow a circular path
    cmd_vel(speed, angularSpeed);
    receive();

    double distanceTravelled = 0.0;

    // Simulate the robot's movement in a circle until it completes one full rotation
    while (distanceTravelled < circumference) {
        usleep(100000); // Waiting for 0.1 second (adjust as needed for the robot and environment)

        // Calculating the distance travelled by the robot in a circular path
        distanceTravelled += speed * 0.1; // 0.1 is the time of each iteration

        // Stopping the robot when it completes one full rotation (one circumference)
        if (distanceTravelled >= circumference) {
            // Sending a stop command to the robot
            cmd_vel(0, 0);
            printf("Stop: ");
            receive();
            break;
        }
    }
}

void move_path(){

    connection(9999); // Establishing a connection with the robot


    double dist;
    printf("Enter linear distance: ");
    scanf("%lf", &dist);

    double rad;
    printf("Enter the radius: ");
    scanf("%lf", &rad);

    moveRobot_linear(dist);
    sleep(2); // Waiting for 0.1 second (adjust as needed for the robot and environment)

    moveRobot_circular(rad);
    sleep(2); // Waiting for 0.1 second (adjust as needed for the robot and environment)

    moveRobot_linear(dist);

}


int main() {

    //to connect to the robot : in one terminal write :
    //gcc TCPEchoClient.c DieWithError.c -o TCPEchoClient
    //then after the part of echoserver:
    //./TCPEchoClient 

    //I'm not sure for the echoserer ask the professor but try this in a seconde terminal:
    //gcc TCPEchoServer.c DieWithError.c HandleTCPClient.c -o TCPEchoServer
    //./TCPEchoServer 9999
    //./TCPEchoServer 9998
    //./TCPEchoServer 9997


    //Try all of these in this order but not at the same time
    // the listenner of the scan
    scna();

    // the odom listenner
    odom();

    // the moving linear function
    moveRobot_linear(1);

    // the moving circular function
    moveRobot_circular(1,1);

    // the moving path
    move_path();

    // if the scan or odom doesn't work don't work try to have their message and type



    return 0;
    close(sock);
    exit(0);
}

