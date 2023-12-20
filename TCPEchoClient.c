 #include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define RCVBUFSIZE 10000  /* Size of receive buffer */
#define MAX_ENTRIES 20
#define MAX_NAME_LENGTH 50
#define MAX_VALUE_LENGTH 50

int sock;                        /* Socket descriptor */
struct sockaddr_in echoServAddr; /* Echo server address */
unsigned short echoServPort ;     /* Echo server port */
char *servIP = "127.0.0.1";                    /* Server IP address (dotted quad) */
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
void receive(){
    /* Receive the same string back from the server */
    totalBytesRcvd = 0;
    while (totalBytesRcvd < echoStringLen)
    {
        /* Receive up to the buffer size (minus 1 to leave space for a null terminator) bytes from the sender */
        if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
            DieWithError("recv() failed or connection closed prematurely");
        totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
        echoBuffer[bytesRcvd] = '\0';  /* Terminate the string! */
        printf("%s", echoBuffer);      /* Print the echo buffer */
    }

    printf("\n");    /* Print a final linefeed */
}

// void scan(){
//     connection(10000); // need to put 9997
//     message("connection working"); // just for the test
//     receive();
// }

// void odom(){
//     connection(11000); // need to put 9998
//     message("scan data"); // just for the test
//     receive();
// }


void cmd_vel(int linear, int angular) {

    char command[100]; // Creating a character array to store the command
    // Formatting the command with linear and angular values
    snprintf(command, sizeof(command), "---Start---{\"linear\":%d , \"angular\":%d}___END___", linear, angular);

    message(command); // Sending the formatted command to the message function
}

void filterLaserScanData(const char *message, char *dataNames[], char *dataValues[], int *numEntries) {
    connection(9997);
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


// Fonction pour déplacer le robot sur une distance donnée
void moveRobot_linear(double distance) {

    connection(9999); // Establishing a connection with the robot

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

int main() {
    char message[] = "Each laser scan is a single scan line. The sensor_msgs/LaserScan message contains the following information: # Single scan from a planar laser range-finder Header header # stamp: The acquisition time of the first ray in the scan. # frame_id: The laser is assumed to spin around the positive Z axis # (counterclockwise, if Z is up) with the zero angle forward along the x axis float32 angle_min: -1.57 float32 angle_max: 1.57 float32 angle_increment: 0.0175 float32 time_increment: 0.001 float32 scan_time: 0.0333 float32 range_min: 0.0 float32 range_max: 100.0 float32[] ranges: [1.0, 2.5, 3.7, 5.2, 10.0] float32[] intensities: []";

    char *dataNames[MAX_ENTRIES];
    char *dataValues[MAX_ENTRIES];
    int numEntries = 0;

    for (int i = 0; i < MAX_ENTRIES; ++i) {
        dataNames[i] = NULL;
        dataValues[i] = NULL;
    }

    filterLaserScanData(message, dataNames, dataValues, &numEntries);

    for (int i = 0; i < numEntries; ++i) {
        printf("Nom : %s | Valeur : %s\n", dataNames[i], dataValues[i]);
        free(dataNames[i]);
        free(dataValues[i]);
    }

    // return 0;
    // connection(10000);
    // scan();
    // odom();
    // cmd_vel();
    moveRobot_linear(5);
    close(sock);
    exit(0);
}

